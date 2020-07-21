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

using namespace nawa;
using namespace std;

class nawa::FastcgiRequestHandler::FastcgippRequestAdapter : public Fastcgipp::Request<char> {
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
    FastcgippRequestAdapter();

    bool response() override;

    bool inProcessor() override;
};

struct nawa::FastcgiRequestHandler::FastcgippManagerAdapter {
    std::unique_ptr<Fastcgipp::Manager<nawa::FastcgiRequestHandler::FastcgippRequestAdapter>> manager;
};

namespace {

}

nawa::FastcgiRequestHandler::FastcgiRequestHandler(nawa::HandleRequestFunction handleRequestFunction_,
                                                   nawa::Config config_, int concurrency) {
    setAppRequestHandler(move(handleRequestFunction_));
    setConfig(move(config_));
    fastcgippManager = std::make_unique<FastcgippManagerAdapter>();
    fastcgippManager->manager = std::make_unique<Fastcgipp::Manager<FastcgippRequestAdapter>>(concurrency);
}
