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
#include "qsf/RequestHandler.h"
#include "qsf/Request.h"
#include "qsf/Connection.h"
#include "qsf/Log.h"

namespace {
    Qsf::handleRequest_t* appHandleRequest;
    Qsf::Log LOG;
}

// initialize static private members, so that the linker does not complain
size_t Qsf::RequestHandler::postMax = 0;
uint Qsf::RequestHandler::rawPostAccess = 1;
Qsf::Config Qsf::RequestHandler::config;

bool Qsf::RequestHandler::response() {
    Qsf::Request request(*this);
    Qsf::Connection connection(request, config);

    // run application
    // TODO maybe do something with return value in future
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
