/**
 * \file RequestHandler.cpp
 * \brief Implementation of the RequestHandler class.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/RequestHandler/impl/FastcgiRequestHandler.h>
#include <nawa/RequestHandler/impl/HttpRequestHandler.h>
#include <nawa/connection/Connection.h>
#include <nawa/session/Session.h>
#include <nawa/util/encoding.h>
#include <shared_mutex>

using namespace nawa;
using namespace std;

struct RequestHandler::Data {
    shared_mutex configurationMutex;
    shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction;
    shared_ptr<AccessFilterList> accessFilters;
    shared_ptr<Config> config;
};

NAWA_DEFAULT_CONSTRUCTOR_IMPL(RequestHandler)

void RequestHandler::setAppRequestHandler(shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction) noexcept {
    unique_lock l(data->configurationMutex);
    data->handleRequestFunction = move(handleRequestFunction);
}

void RequestHandler::setAccessFilters(AccessFilterList accessFilters) noexcept {
    unique_lock l(data->configurationMutex);
    data->accessFilters = make_shared<AccessFilterList>(move(accessFilters));
}

void RequestHandler::setConfig(Config config) noexcept {
    unique_lock l(data->configurationMutex);
    data->config = make_shared<Config>(move(config));
}

shared_ptr<Config const> RequestHandler::getConfig() const noexcept {
    return data->config;
}

void RequestHandler::reconfigure(optional<shared_ptr<HandleRequestFunctionWrapper>> handleRequestFunction,
                                 optional<AccessFilterList> accessFilters,
                                 optional<Config> config) noexcept {
    unique_lock l(data->configurationMutex);
    if (handleRequestFunction) {
        data->handleRequestFunction = *handleRequestFunction;
    }
    if (accessFilters) {
        data->accessFilters = make_shared<AccessFilterList>(move(*accessFilters));
    }
    if (config) {
        data->config = make_shared<Config>(move(*config));
    }
}

void nawa::RequestHandler::reconfigure(HandleRequestFunction handleRequestFunction, optional<AccessFilterList> accessFilters,
                                       optional<Config> config) noexcept {
    reconfigure(make_shared<HandleRequestFunctionWrapper>(move(handleRequestFunction)), move(accessFilters),
                move(config));
}

void RequestHandler::handleRequest(Connection& connection) {
    shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction;
    shared_ptr<AccessFilterList> accessFilters;
    shared_ptr<Config> config;
    {
        shared_lock l(data->configurationMutex);
        handleRequestFunction = data->handleRequestFunction;
        accessFilters = data->accessFilters;
    }
    // test filters and run app if no filter was triggered
    if (!accessFilters || !connection.applyFilters(*accessFilters)) {
        (*handleRequestFunction)(connection);
    }
}

unique_ptr<RequestHandler>
RequestHandler::newRequestHandler(shared_ptr<HandleRequestFunctionWrapper> const& handleRequestFunction,
                                  Config config, int concurrency) {
    if (config[{"system", "request_handler"}] == "http") {
        return make_unique<HttpRequestHandler>(handleRequestFunction, move(config), concurrency);
    }
    return make_unique<FastcgiRequestHandler>(handleRequestFunction, move(config), concurrency);
}

unique_ptr<RequestHandler>
RequestHandler::newRequestHandler(HandleRequestFunction handleRequestFunction, Config config,
                                  int concurrency) {
    return newRequestHandler(make_shared<HandleRequestFunctionWrapper>(move(handleRequestFunction)), move(config),
                             concurrency);
}

RequestHandler::~RequestHandler() = default;
