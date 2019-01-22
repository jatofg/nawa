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

    // check extension in request path and compare it
    bool extensionMatches(const std::vector<std::string> &path, const std::string& extension) {
        auto const &filename = path.back();
        return filename.substr(filename.find_last_of('.') + 1) == extension;
    }

    // check if the given request path is a member of the second one (for path filtering)
    bool pathMatches(const std::vector<std::string> &path1, const std::vector<std::string> &path2) {
        // the second path is the one given by the filter: all elements of the second path must be in the first
        if(path2.size() < path1.size())
            return false;
        for(unsigned long i = 0; i < path2.size(); ++i) {
            if(path2.at(i) != path1.at(i))
                return false;
        }
        return true;
    }

    // check whether the req. path (as a string "/dir1/dir2/file.ext") is matched by the regex
    bool regexMatches(const std::vector<std::string> &path, const std::regex &regex) {
        // merge path to string
        std::stringstream pathStr;
        for(auto const &e: path) {
            pathStr << '/' << e;
        }
        return std::regex_match(pathStr.str(), regex);
    }

}

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    Qsf::Connection connection(request, config);

    // run application
    // TODO maybe do something with return value in future
    // TODO apply filters before
    appHandleRequest(connection);

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

    auto requestPath = connection.request.env.getPathInfo();

    // check block filters
    for(auto const &flt: appInit.accessFilters.blockFilters) {
        switch(flt.filterType) {
            case Qsf::AccessFilter::EXTENSION:
                if(extensionMatches(requestPath, flt.extensionFilter)) {
                    connection.setStatus(flt.status);
                    if(!flt.response.empty()) {
                        connection.setBody(flt.response);
                    }
                    else {
                        connection.setBody(Qsf::generate_error_page(flt.status));
                    }
                    return true;
                }
                break;
            case Qsf::AccessFilter::PATH:
                if(pathMatches(requestPath, flt.pathFilter)) {
                    connection.setStatus(flt.status);
                    if(!flt.response.empty()) {
                        connection.setBody(flt.response);
                    }
                    else {
                        connection.setBody(Qsf::generate_error_page(flt.status));
                    }
                    return true;
                }
                break;
            case Qsf::AccessFilter::REGEX:
                if(regexMatches(requestPath, flt.regexFilter)) {
                    connection.setStatus(flt.status);
                    if(!flt.response.empty()) {
                        connection.setBody(flt.response);
                    }
                    else {
                        connection.setBody(Qsf::generate_error_page(flt.status));
                    }
                    return true;
                }
                break;
        }

        // check auth filters
        for(auto const &flt: appInit.accessFilters.authFilters) {
            switch(flt.filterType) {
                case Qsf::AccessFilter::EXTENSION:
                    if(extensionMatches(requestPath, flt.extensionFilter)) {
                        // request auth, then return true if auth succeeds, otherwise send error document
                    }
                    break;
                case Qsf::AccessFilter::PATH:
                    if(pathMatches(requestPath, flt.pathFilter)) {
                        // ...
                    }
                    break;
                case Qsf::AccessFilter::REGEX:
                    if(regexMatches(requestPath, flt.regexFilter)) {
                        // ...
                    }
                    break;
            }
        }

        // check forward filters
        for(auto const &flt: appInit.accessFilters.forwardFilters) {
            switch(flt.filterType) {
                case Qsf::AccessFilter::EXTENSION:
                    if(extensionMatches(requestPath, flt.extensionFilter)) {
                        // look up path (with correct basePathExtension), return document with correct content-type
                        // if found, otherwise return 404 error page (flt.response or generate_error_page(404))
                        // check if cached in the browser (ifModifiedSince) and set a modified header!
                    }
                    break;
                case Qsf::AccessFilter::PATH:
                    if(pathMatches(requestPath, flt.pathFilter)) {
                        // ...
                    }
                    break;
                case Qsf::AccessFilter::REGEX:
                    if(regexMatches(requestPath, flt.regexFilter)) {
                        // ...
                    }
                    break;
            }
        }
    }

}
