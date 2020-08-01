/**
 * \file RequestHandler.cpp
 * \brief Implementation of the RequestHandler class.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#include <nawa/Connection.h>
#include <nawa/Utils.h>
#include <nawa/Session.h>
#include <nawa/Encoding.h>
#include <nawa/Exception.h>
#include <nawa/RequestHandlers/RequestHandler.h>
#include <nawa/RequestHandlers/FastcgiRequestHandler.h>

using namespace nawa;
using namespace std;

namespace {
    /**
     * Check the conditions of an AccessFilter.
     * @param requestPath The request path of the current request.
     * @param flt The access filter that will be checked.
     * @return True if the filter matches, false otherwise.
     */
    bool filterMatches(const vector<string> &requestPath, const AccessFilter &flt) {
        if (!flt.pathFilter.empty()) {
            // one of the paths in the path filter must match for the path filter to match
            bool pathFilterMatches = false;
            for (auto const &filter: flt.pathFilter) {
                // path condition is set but does not match -> the whole filter does not match
                // all elements of the filter path must be in the request path
                if (requestPath.size() < filter.size()) {
                    continue;
                }
                pathFilterMatches = true;
                for (size_t i = 0; i < filter.size(); ++i) {
                    if (filter.at(i) != requestPath.at(i)) {
                        pathFilterMatches = false;
                        break;
                    }
                }
                if (pathFilterMatches) {
                    break;
                }
            }
            if ((!pathFilterMatches && !flt.invertPathFilter) || (pathFilterMatches && flt.invertPathFilter)) {
                return false;
            }
            // path condition matches -> continue to the next filter condition
        }

        if (!flt.extensionFilter.empty()) {
            auto fileExtension = get_file_extension(requestPath.back());
            bool extensionFilterMatches = false;
            for (auto const &e: flt.extensionFilter) {
                if (fileExtension == e) {
                    extensionFilterMatches = true;
                    break;
                }
            }
            if ((!extensionFilterMatches && !flt.invertExtensionFilter) ||
                (extensionFilterMatches && flt.invertExtensionFilter)) {
                return false;
            }
            // extension condition matches -> continue to the next filter condition
        }

        if (flt.regexFilterEnabled) {
            // merge request path to string
            stringstream pathStr;
            for (auto const &e: requestPath) {
                pathStr << '/' << e;
            }
            if (!regex_match(pathStr.str(), flt.regexFilter))
                return false;
        }

        // all conditions match or no condition has been set -> the filter matches
        return true;
    }

    /**
     * Apply the filters set by the app (through AppInit), if filtering is enabled.
     * @param connection Reference to the connection object to read the request from and write the response to,
     * if the request has to be filtered.
     * @param accessFilters The filters to apply.
     * @return True if the request has been filtered and a response has already been set by this function
     * (and the app should not be invoked on this request). False if the app should handle this request.
     */
    bool applyFilters(Connection &connection, const AccessFilterList &accessFilters) {
        // if filters are disabled, do not even check
        if (!accessFilters.filtersEnabled) return false;

        auto requestPath = connection.request.env.getRequestPath();

        // check block filters
        for (auto const &flt: accessFilters.blockFilters) {
            // if the filter does not apply (or does in case of an inverted filter), go to the next
            bool matches = filterMatches(requestPath, flt);
            if ((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            // filter matches -> apply block
            connection.setStatus(flt.status);
            if (!flt.response.empty()) {
                connection.setBody(flt.response);
            } else {
                connection.setBody(generate_error_page(flt.status));
            }
            // the request has been blocked, so no more filters have to be applied
            // returning true means: the request has been filtered
            return true;
        }

        // the ID is used to identify the exact filter for session cookie creation
        int authFilterID = -1;
        for (auto const &flt: accessFilters.authFilters) {
            ++authFilterID;

            bool matches = filterMatches(requestPath, flt);
            if ((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            bool isAuthenticated = false;
            string sessionCookieName;

            // check session variable for this filter, if session usage is on
            if (flt.useSessions) {
                connection.session.start();
                sessionCookieName = "_nawa_authfilter" + to_string(authFilterID);
                if (connection.session.isSet(sessionCookieName)) {
                    isAuthenticated = true;
                }
            }

            // if this did not work, request authentication or invoke auth function if credentials have already been sent
            if (!isAuthenticated) {
                // case 1: no authorization header sent by client -> send a 401 without body
                if (connection.request.env["authorization"].empty()) {
                    connection.setStatus(401);
                    stringstream hval;
                    hval << "Basic";
                    if (!flt.authName.empty()) {
                        hval << " realm=\"" << flt.authName << '"';
                    }
                    connection.setHeader("www-authenticate", hval.str());

                    // that's it, the response must be sent to the client directly so it can authenticate
                    return true;
                }
                    // case 2: credentials already sent
                else {
                    // split the authorization string, only the last part should contain base64
                    auto authResponse = split_string(connection.request.env["authorization"], ' ', true);
                    // here, we should have a vector with size 2 and [0]=="Basic", otherwise sth is wrong
                    if (authResponse.size() == 2 || authResponse.at(0) == "Basic") {
                        auto credentials = split_string(Encoding::base64Decode(authResponse.at(1)), ':', true);
                        // credentials must also have 2 elements, a username and a password,
                        // and the auth function must be callable
                        if (credentials.size() == 2 && flt.authFunction) {
                            // now we can actually check the credentials with our function (if it is set)
                            if (flt.authFunction(credentials.at(0), credentials.at(1))) {
                                isAuthenticated = true;
                                // now, if sessions are used, set the session variable to the username
                                if (flt.useSessions) {
                                    connection.session.set(sessionCookieName, any(credentials.at(0)));
                                }
                            }
                        }
                    }
                }
            }

            // now, if the user is still not authenticated, send a 403 Forbidden
            if (!isAuthenticated) {
                connection.setStatus(403);
                if (!flt.response.empty()) {
                    connection.setBody(flt.response);
                } else {
                    connection.setBody(generate_error_page(403));
                }

                // request blocked
                return true;
            }

            // if the user is authenticated, we can continue to process forward filters
            break;

        }

        // check forward filters
        for (auto const &flt: accessFilters.forwardFilters) {
            bool matches = filterMatches(requestPath, flt);
            if ((!matches && !flt.invert) || (matches && flt.invert)) {
                continue;
            }

            stringstream filePath;
            filePath << flt.basePath;
            if (flt.basePathExtension == ForwardFilter::BY_PATH) {
                for (auto const &e: requestPath) {
                    filePath << '/' << e;
                }
            } else {
                filePath << '/' << requestPath.back();
            }

            // send file if it exists, catch the "file does not exist" nawa::Exception and send 404 document if not
            auto filePathStr = filePath.str();
            try {
                connection.sendFile(filePathStr, "", false, "", true);
            }
            catch (Exception &) {
                // file does not exist, send 404
                connection.setStatus(404);
                if (!flt.response.empty()) {
                    connection.setBody(flt.response);
                } else {
                    connection.setBody(generate_error_page(404));
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

void RequestHandler::setAppRequestHandler(HandleRequestFunction fn) noexcept {
    handleRequestFunction = move(fn);
}

void RequestHandler::setAppInit(AppInit appInit_) noexcept {
    appInit = move(appInit_);
}

void RequestHandler::setConfig(Config config_) noexcept {
    config = move(config_);
}

void RequestHandler::handleRequest(Connection &connection) {
    // test filters and run app if no filter was triggered
    // TODO maybe do something with return value in future
    if (!applyFilters(connection, appInit.accessFilters)) {
        handleRequestFunction(connection);
    }
}

std::unique_ptr<RequestHandler>
RequestHandler::newRequestHandler(HandleRequestFunction handleRequestFunction, Config config, int concurrency) {
    return make_unique<FastcgiRequestHandler>(move(handleRequestFunction), move(config), concurrency);
}