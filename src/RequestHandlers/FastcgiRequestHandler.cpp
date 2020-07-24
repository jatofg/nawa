/**
 * \file FastcgiRequestHandler.cpp
 * \brief Implementation of the FastcgiRequestHandler class.
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

#include <nawa/RequestHandlers/RequestHandler.h>
#include <nawa/RequestHandlers/FastcgiRequestHandler.h>
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <nawa/Log.h>

using namespace nawa;
using namespace std;

namespace {
    Log LOG;
}

struct FastcgippArgumentContainer {
    RequestHandler* requestHandler;
    size_t postMax;
};

class FastcgippRequestAdapter : public Fastcgipp::Request<char> {
public:
    // TODO:
    //  - in the constructor, set postMax (has to be passed via externalObject)
    //  - also pass the request handler via the externalObject (RequestHandler::handleRequest)
    //      - in Response, it is necessary to use this function to handle the request
    //  - for flushing, use a callback saved in the Connection
    //  - is the FastcgiRequestHandler object still required then?

    /**
     * Construct the RequestHandler object by passing the postMax (as set by setConfig(...)) to the fastcgi library.
     */
    FastcgippRequestAdapter() : Fastcgipp::Request<char>(any_cast<FastcgippArgumentContainer>(m_externalObject).postMax) {}

    bool response() override;

    bool inProcessor() override;
};

bool FastcgippRequestAdapter::response() {

    // create Request and Connection (have to be modified before)
    // callback for flushing has to be in Connection as well, needs to be set here
    //      auto raw = connection.getRaw();
    //      dump(raw.c_str(), raw.size());
    // call m_externalObject.requestHandler->handleRequest
    //      maybe set the FastcgippArgumentContainer object as member
    // flush response
    // return true

    return false;
}

bool FastcgippRequestAdapter::inProcessor() {
//    postContentType = environment().contentType;
//    if(rawPostAccess == RawPostAccess::NEVER) {
//        return false;
//    }
//    else if (rawPostAccess == RawPostAccess::NONSTANDARD &&
//             (postContentType == "multipart/form-data" || postContentType == "application/x-www-form-urlencoded")) {
//        return false;
//    }
//    auto postBuffer = environment().postBuffer();
//    rawPost = std::string(postBuffer.data(), postBuffer.size());
    return false;
}

struct nawa::FastcgiRequestHandler::FastcgippManagerAdapter {
    std::unique_ptr<Fastcgipp::Manager<FastcgippRequestAdapter>> manager;
};

nawa::FastcgiRequestHandler::FastcgiRequestHandler(nawa::HandleRequestFunction handleRequestFunction_,
                                                   nawa::Config config_, int concurrency) {
    setAppRequestHandler(move(handleRequestFunction_));
    setConfig(move(config_));

    FastcgippArgumentContainer arg {
        .requestHandler=this,
        .postMax=0
    };

    try {
        arg.postMax = config.isSet({"post", "max_size"})
                  ? static_cast<size_t>(std::stoul(config[{"post", "max_size"}])) * 1024 : 0;
    }
    catch(std::invalid_argument& e) {
        LOG("WARNING: Invalid value given for post/max_size given in the config file.");
    }

    fastcgippManager = std::make_unique<FastcgippManagerAdapter>();
    fastcgippManager->manager = std::make_unique<Fastcgipp::Manager<FastcgippRequestAdapter>>(concurrency, arg);
}

void FastcgiRequestHandler::start() {
    if(fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->start();
    }
}

void FastcgiRequestHandler::stop() {
    if(fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->stop();
    }
}

void FastcgiRequestHandler::terminate() {
    if(fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->terminate();
    }
}

void FastcgiRequestHandler::join() {
    if(fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->join();
    }
}
