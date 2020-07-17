/**
 * \file RequestHandler.cpp
 * \brief Implementation of the RequestHandler class.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <thread>
#include <dlfcn.h>
#include <nawa/RequestHandler.h>
#include <nawa/Utils.h>
#include <nawa/Encoding.h>
#include <nawa/Request.h>
#include <nawa/Connection.h>
#include <nawa/Log.h>
#include <nawa/SysException.h>
#include <nawa/UserException.h>

namespace {
    nawa::handleRequest_t* appHandleRequest;
    nawa::Log LOG;
    size_t postMax = 0; /**< Maximum post size, in bytes, read from the config by setAppRequestHandler(...). */

    /**
     * Stores the raw post access level, as read from the config file.
     */
    enum class RawPostAccess {
        NEVER,
        NONSTANDARD,
        ALWAYS
    } rawPostAccess;

    // The config read from the config file (and possibly modified by the app's init() function) will be stored in
    // the AppInit. The Config object will be copied upon each request into a non-static member of Connection,
    // so it can be modified at runtime.
    // unique_ptr necessary so we can explicitly destruct it and avoid a segmentation fault on termination
    std::unique_ptr<nawa::AppInit> appInitPtr; /**< The initialization struct as returned by the app init() function. */

    /**
     * Check the conditions of an AccessFilter.
     * @param requestPath The request path of the current request.
     * @param flt The access filter that will be checked.
     * @return True if the filter matches, false otherwise.
     */
    bool filterMatches(const std::vector<std::string> &requestPath, const nawa::AccessFilter &flt) {
        if(!flt.pathFilter.empty()) {
            // one of the paths in the path filter must match for the path filter to match
            bool pathFilterMatches = false;
            for(auto const &filter: flt.pathFilter) {
                // path condition is set but does not match -> the whole filter does not match
                // all elements of the filter path must be in the request path
                if(requestPath.size() < filter.size()) {
                    continue;
                }
                pathFilterMatches = true;
                for(size_t i = 0; i < filter.size(); ++i) {
                    if(filter.at(i) != requestPath.at(i)) {
                        pathFilterMatches = false;
                        break;
                    }
                }
                if(pathFilterMatches) {
                    break;
                }
            }
            if((!pathFilterMatches && !flt.invertPathFilter) || (pathFilterMatches && flt.invertPathFilter)) {
                return false;
            }
            // path condition matches -> continue to the next filter condition
        }

        if(!flt.extensionFilter.empty()) {
            auto fileExtension = nawa::get_file_extension(requestPath.back());
            bool extensionFilterMatches = false;
            for(auto const &e: flt.extensionFilter) {
                if(fileExtension == e) {
                    extensionFilterMatches = true;
                    break;
                }
            }
            if((!extensionFilterMatches && !flt.invertExtensionFilter) || (extensionFilterMatches && flt.invertExtensionFilter)) {
                return false;
            }
            // extension condition matches -> continue to the next filter condition
        }

        if(flt.regexFilterEnabled) {
            // merge request path to string
            std::stringstream pathStr;
            for(auto const &e: requestPath) {
                pathStr << '/' << e;
            }
            if(!std::regex_match(pathStr.str(), flt.regexFilter))
                return false;
        }

        // all conditions match or no condition has been set -> the filter matches
        return true;
    }

    /**
     * Apply the filters set by the app (through AppInit), if filtering is enabled.
     * @param connection Reference to the connection object to read the request from and write the response to,
     * if the request has to be filtered.
     * @return True if the request has been filtered and a response has already been set by this function
     * (and the app should not be invoked on this request). False if the app should handle this request.
     */
    bool applyFilters(nawa::Connection &connection) {
        // check AppInitPtr for safety
        if(!appInitPtr) {
            throw nawa::SysException("RequestHandler.cpp", __LINE__, "AppInit pointer empty");
        }

        // if filters are disabled, do not even check
        if(!appInitPtr->accessFilters.filtersEnabled) return false;

        auto requestPath = connection.request.env.getRequestPath();

        // check block filters
        for(auto const &flt: appInitPtr->accessFilters.blockFilters) {
            // if the filter does not apply (or does in case of an inverted filter), go to the next
            bool matches = filterMatches(requestPath, flt);
            if((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            // filter matches -> apply block
            connection.setStatus(flt.status);
            if(!flt.response.empty()) {
                connection.setBody(flt.response);
            }
            else {
                connection.setBody(nawa::generate_error_page(flt.status));
            }
            // the request has been blocked, so no more filters have to be applied
            // returning true means: the request has been filtered
            return true;
        }

        // the ID is used to identify the exact filter for session cookie creation
        int authFilterID = -1;
        for(auto const &flt: appInitPtr->accessFilters.authFilters) {
            ++authFilterID;

            bool matches = filterMatches(requestPath, flt);
            if((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            bool isAuthenticated = false;
            std::string sessionCookieName;

            // check session variable for this filter, if session usage is on
            if(flt.useSessions) {
                connection.session.start();
                sessionCookieName = "_nawa_authfilter" + std::to_string(authFilterID);
                if(connection.session.isSet(sessionCookieName)) {
                    isAuthenticated = true;
                }
            }

            // if this did not work, request authentication or invoke auth function if credentials have already been sent
            if(!isAuthenticated) {
                // case 1: no authorization header sent by client -> send a 401 without body
                if(connection.request.env["authorization"].empty()) {
                    connection.setStatus(401);
                    std::stringstream hval;
                    hval << "Basic";
                    if(!flt.authName.empty()) {
                        hval << " realm=\"" << flt.authName << '"';
                    }
                    connection.setHeader("www-authenticate", hval.str());

                    // that's it, the response must be sent to the client directly so it can authenticate
                    return true;
                }
                    // case 2: credentials already sent
                else {
                    // split the authorization string, only the last part should contain base64
                    auto authResponse = nawa::split_string(connection.request.env["authorization"], ' ', true);
                    // here, we should have a vector with size 2 and [0]=="Basic", otherwise sth is wrong
                    if(authResponse.size() == 2 || authResponse.at(0) == "Basic") {
                        auto credentials = nawa::split_string(nawa::Encoding::base64Decode(authResponse.at(1)), ':', true);
                        // credentials must also have 2 elements, a username and a password,
                        // and the auth function must be callable
                        if(credentials.size() == 2 && flt.authFunction) {
                            // now we can actually check the credentials with our function (if it is set)
                            if(flt.authFunction(credentials.at(0), credentials.at(1))) {
                                isAuthenticated = true;
                                // now, if sessions are used, set the session variable to the username
                                if(flt.useSessions) {
                                    connection.session.set(sessionCookieName, std::any(credentials.at(0)));
                                }
                            }
                        }
                    }
                }
            }

            // now, if the user is still not authenticated, send a 403 Forbidden
            if(!isAuthenticated) {
                connection.setStatus(403);
                if(!flt.response.empty()) {
                    connection.setBody(flt.response);
                }
                else {
                    connection.setBody(nawa::generate_error_page(403));
                }

                // request blocked
                return true;
            }

            // if the user is authenticated, we can continue to process forward filters
            break;

        }

        // check forward filters
        for(auto const &flt: appInitPtr->accessFilters.forwardFilters) {
            bool matches = filterMatches(requestPath, flt);
            if((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            std::stringstream filePath;
            filePath << flt.basePath;
            if(flt.basePathExtension == nawa::ForwardFilter::BY_PATH) {
                for(auto const &e: requestPath) {
                    filePath << '/' << e;
                }
            }
            else {
                filePath << '/' << requestPath.back();
            }

            // send file if it exists, catch the "file does not exist" UserException and send 404 document if not
            auto filePathStr = filePath.str();
            try {
                connection.sendFile(filePathStr, "", false, "", true);
            }
            catch(nawa::UserException&) {
                // file does not exist, send 404
                connection.setStatus(404);
                if(!flt.response.empty()) {
                    connection.setBody(flt.response);
                }
                else {
                    connection.setBody(nawa::generate_error_page(404));
                }
            }

            // return true as the request has been filtered
            return true;
        }

        // if no filters were triggered (and therefore returned true), return false so that the request can be handled by
        // the app
        return false;

    }

}

bool nawa::RequestHandler::response() {
    // check AppInitPtr for safety
    if(!appInitPtr) {
        throw nawa::SysException("RequestHandler.cpp", __LINE__, "AppInit pointer empty");
    }

    nawa::Request request(*this);
    nawa::Connection connection(request, appInitPtr->config);

    // test filters and run app if no filter was triggered
    // TODO maybe do something with return value in future
    if(!applyFilters(connection)) {
        appHandleRequest(connection);
    }

    // flush response
    connection.flushResponse();

    return true;
}

void nawa::RequestHandler::flush(nawa::Connection& connection) {
    auto raw = connection.getRaw();
    dump(raw.c_str(), raw.size());
}

void nawa::RequestHandler::setAppRequestHandler(const nawa::Config &cfg, void *appOpen) {
    try {
        postMax = cfg.isSet({"post", "max_size"})
                      ? static_cast<size_t>(std::stoul(cfg[{"post", "max_size"}])) * 1024 : 0;
    }
    catch(std::invalid_argument& e) {
        LOG("WARNING: Invalid value given for post/max_size given in the config file.");
        postMax = 0;
    }
    // raw_access is translated to an integer according to the macros defined in RequestHandler.h
    std::string rawPostStr = cfg[{"post", "raw_access"}];
    rawPostAccess = (rawPostStr == "never")
                   ? RawPostAccess::NEVER : ((rawPostStr == "always") ? RawPostAccess::ALWAYS : RawPostAccess::NONSTANDARD);

    // load appHandleRequest function
    appHandleRequest = (nawa::handleRequest_t*) dlsym(appOpen, "handleRequest");
    auto dlsymErr = dlerror();
    if(dlsymErr) {
        LOG(std::string("Fatal Error: Could not load handleRequest function from application: ") + dlsymErr);
        exit(1);
    }
}

nawa::RequestHandler::RequestHandler() : Fastcgipp::Request<char>(postMax) {}

bool nawa::RequestHandler::inProcessor() {
    postContentType = environment().contentType;
    if(rawPostAccess == RawPostAccess::NEVER) {
        return false;
    }
    else if (rawPostAccess == RawPostAccess::NONSTANDARD &&
            (postContentType == "multipart/form-data" || postContentType == "application/x-www-form-urlencoded")) {
        return false;
    }
    auto postBuffer = environment().postBuffer();
    rawPost = std::string(postBuffer.data(), postBuffer.size());
    return false;
}

void nawa::RequestHandler::setConfig(const nawa::AppInit &_appInit) {
    appInitPtr = std::make_unique<nawa::AppInit>(_appInit);
}

void nawa::RequestHandler::destroyEverything() {
    appInitPtr.reset(nullptr);
    nawa::Session::destroy();
}
