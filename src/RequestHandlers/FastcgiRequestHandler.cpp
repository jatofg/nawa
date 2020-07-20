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
    //  - in the constructor, set postMax (has to be set in a static function before)
    //  - also use a static function to point to the request handler (RequestHandler::handleRequest)
    //  - in Response, it is necessary to use this function to handle the request
    //  - in the constructor, also make sure to pass the object (this) to FascgiRequestHandler
    //      - to achieve this, it should be necessary to pass a ref/ptr to the FcgiRH object to the Adapter
    //          using a static function
    //  - as static functions are used, it is necessary to analyze the impact on obect orientation
    //      - are the static objects bound to the FcgiRH object as this class is a FcgiRH non-static member?
    //      - sounds unlikely :(

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
