/**
 * \file RequestHandler.cpp
 * \brief Implementation of the RequestHandler class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include <qsf/RequestHandler.h>
#include <qsf/Utils.h>
#include <qsf/Encoding.h>

#include "qsf/RequestHandler.h"
#include "qsf/Request.h"
#include "qsf/Connection.h"
#include "qsf/Log.h"

namespace {
    Qsf::handleRequest_t* appHandleRequest;
    Qsf::Log LOG;
    size_t postMax = 0; /* Maximum post size, in bytes, read from the config by setConfig(...). */
    // Integer value referring to the raw post access level, as described by the QSF_RAWPOST_* macros.
    unsigned int rawPostAccess = 1;
    // The config read from the config file will be stored here statically. The Config object will be copied
    // upon each request into a non-static member of Connection, so it can be modified at runtime.
    Qsf::Config config; /* The config as loaded by main. */
    Qsf::AppInit appInit; /* The initialization struct as returned by the app init() function. */

    /**
     * Check the conditions of an AccessFilter.
     * @param requestPath The request path of the current request.
     * @param flt The access filter that will be checked.
     * @return True if the filter matches, false otherwise.
     */
    bool filterMatches(const std::vector<std::string> &requestPath, const Qsf::AccessFilter &flt) {
        if(!flt.pathFilter.empty()) {
            // path condition is set but does not match -> the whole filter does not match
            // all elements of the filter path must be in the request path
            if(requestPath.size() < flt.pathFilter.size())
                return false;
            for(unsigned long i = 0; i < flt.pathFilter.size(); ++i) {
                if(flt.pathFilter.at(i) != requestPath.at(i))
                    return false;
            }
            // path condition matches -> continue to the next filter
        }

        if(!flt.extensionFilter.empty()) {
            auto const &filename = requestPath.back();
            if(Qsf::get_file_extension(filename) != flt.extensionFilter)
                return false;
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

}

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    Qsf::Connection connection(request, config);

    // test filters and run app if no filter was triggered
    // TODO maybe do something with return value in future
    if(!applyFilters(connection)) {
        appHandleRequest(connection);
    }

    // flush response
    connection.flushResponse();

    return true;
}

void Qsf::RequestHandler::flush(Qsf::Connection& connection) {
    auto raw = connection.getRaw();
    dump(raw.c_str(), raw.size());
}

void Qsf::RequestHandler::setConfig(const Qsf::Config& cfg, void* appOpen) {
    config = cfg;
    try {
        postMax = config.isSet({"post", "max_size"})
                      ? static_cast<size_t>(std::stoul(config[{"post", "max_size"}])) * 1024 : 0;
    }
    catch(std::invalid_argument& e) {
        LOG("WARNING: Invalid value given for post/max_size given in the config file.");
        postMax = 0;
    }
    // raw_access is translated to an integer according to the macros defined in RequestHandler.h
    std::string rawPostStr = config[{"post", "raw_access"}];
    rawPostAccess = (rawPostStr == "never")
                   ? QSF_RAWPOST_NEVER : ((rawPostStr == "always") ? QSF_RAWPOST_ALWAYS : QSF_RAWPOST_NONSTANDARD);

    // load appHandleRequest function
    appHandleRequest = (Qsf::handleRequest_t*) dlsym(appOpen, "handleRequest");
    auto dlsymErr = dlerror();
    if(dlsymErr) {
        LOG(std::string("Fatal Error: Could not load handleRequest function from application: ") + dlsymErr);
        exit(1);
    }
}

Qsf::RequestHandler::RequestHandler() : Fastcgipp::Request<char>(postMax) {}

bool Qsf::RequestHandler::inProcessor() {
    postContentType = environment().contentType;
    if(rawPostAccess == QSF_RAWPOST_NEVER) {
        return false;
    }
    else if (rawPostAccess == QSF_RAWPOST_NONSTANDARD &&
            (postContentType == "multipart/form-data" || postContentType == "application/x-www-form-urlencoded")) {
        return false;
    }
    auto postBuffer = environment().postBuffer();
    rawPost = std::string(postBuffer.data(), postBuffer.size());
    return false;
}

void Qsf::RequestHandler::setAppInit(const Qsf::AppInit &_appInit) {
    appInit = _appInit;
}

bool Qsf::RequestHandler::applyFilters(Qsf::Connection &connection) {

    // if filters are disabled, do not even check
    if(!appInit.accessFilters.filtersEnabled) return false;

    auto requestPath = connection.request.env.getRequestPath();

    // check block filters
    for(auto const &flt: appInit.accessFilters.blockFilters) {
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
            connection.setBody(Qsf::generate_error_page(flt.status));
        }
        // the request has been blocked, so no more filters have to be applied
        // returning true means: the request has been filtered
        return true;
    }

    // the ID is used to identify the exact filter for session cookie creation
    int authFilterID = -1;
    for(auto const &flt: appInit.accessFilters.authFilters) {
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
            sessionCookieName = "_qsf_authfilter" + std::to_string(authFilterID);
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
                auto authResponse = Qsf::split_string(connection.request.env["authorization"], ' ', true);
                // here, we should have a vector with size 2 and [0]=="Basic", otherwise sth is wrong
                if(authResponse.size() == 2 || authResponse.at(0) == "Basic") {
                    auto credentials = Qsf::split_string(Qsf::Encoding::base64Decode(authResponse.at(1)), ':', true);
                    // credentials must also have 2 elements, a username and a password,
                    // and the auth function must be callable
                    if(credentials.size() == 2 && flt.authFunction) {
                        // now we can actually check the credentials with our function (if it is set)
                        if(flt.authFunction(credentials.at(0), credentials.at(1))) {
                            isAuthenticated = true;
                            // now, if sessions are used, set the session variable to the username
                            if(flt.useSessions) {
                                connection.session.set(sessionCookieName, Types::Universal(credentials.at(0)));
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
                connection.setBody(Qsf::generate_error_page(403));
            }

            // request blocked
            return true;
        }

        // if the user is authenticated, we can continue to process forward filters
        break;

    }

    // check forward filters
    for(auto const &flt: appInit.accessFilters.forwardFilters) {
        bool matches = filterMatches(requestPath, flt);
        if((!matches && !flt.invert) || (matches && flt.invert)) {
            continue;
        }

        std::stringstream filePath;
        filePath << flt.basePath;
        if(flt.basePathExtension == Qsf::ForwardFilter::BY_PATH) {
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
        catch(Qsf::UserException&) {
            // file does not exist, send 404
            connection.setStatus(404);
            if(!flt.response.empty()) {
                connection.setBody(flt.response);
            }
            else {
                connection.setBody(Qsf::generate_error_page(404));
            }
        }

        // return true as the request has been filtered
        return true;
    }

    // if no filters were triggered (and therefore returned true), return false so that the request can be handled by
    // the app
    return false;

}
