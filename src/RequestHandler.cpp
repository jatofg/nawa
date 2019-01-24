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

        if(!flt.regexFilterEnabled) {
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
        // if the filter does not apply, go to the next
        if(!filterMatches(requestPath, flt)) {
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

    // TODO check auth filters (or remove them completely)

    // check forward filters
    for(auto const &flt: appInit.accessFilters.forwardFilters) {
        if(!filterMatches(requestPath, flt)) {
            continue;
        }
        // TODO use sendFile from Connection
        //  - do not forget checking if-modified-since (not modified response)

        // check if-modified-since header
        // if i-m-s header available, check the last modification time of the file and compare (phew...)
        // if not modified, send not-modified response (with corresponding http status code)
        // otherwise, use connection.sendFile to send the file
        // catch UserException from sendFile, for 404 case, and send 404 with flt.response or generate_error_page(404)

        return true;
    }

    // if no filters were triggered (and therefore returned true), return false so that the request can be handled by
    // the app
    return false;

}
