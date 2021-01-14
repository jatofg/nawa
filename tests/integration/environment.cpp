/**
 * \file environment.cpp
 * \brief Unit tests for the request environment using the HTTP request handler.
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

#include <boost/network/protocol/http/client.hpp>
#include <catch2/catch.hpp>
#include <nawa/Connection.h>
#include <nawa/Exception.h>
#include <nawa/RequestHandlers/RequestHandler.h>

using namespace nawa;
using namespace std;

namespace http = boost::network::http;

namespace {
    bool isEnvironmentInitialized = false;
    Config config;

    /**
     * Initialize test environment if not yet done.
     * @return True on success, false on failure.
     */
    bool initializeEnvironmentIfNotYetDone() {
        if (isEnvironmentInitialized) {
            return true;
        }

        config.set("http", "port", "8089");
        config.set("http", "reuseaddr", "on");
        config.set("post", "max_size", "1");
        config.set("system", "request_handler", "http");
        config.set("logging", "level", "debug");
        config.set("logging", "extended", "on");

        isEnvironmentInitialized = true;
        return true;
    }
}

TEST_CASE("Basic request handling (HTTP)", "[basic][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());

    auto handlingFunction = [](Connection& connection) -> int {
        connection.response << "Hello World!";
        return 0;
    };
    unique_ptr<RequestHandler> requestHandler;
    REQUIRE_NOTHROW(requestHandler = RequestHandler::newRequestHandler(handlingFunction, config, 1));
    REQUIRE_NOTHROW(requestHandler->start());

    // wait until started
    sleep(1);

    http::client client;
    http::client::request request("http://127.0.0.1:8089/");
    http::client::response response;
    REQUIRE_NOTHROW(response = client.get(request));
    REQUIRE(response.body() == "Hello World!");

    requestHandler->terminate();
    requestHandler->join();
}

// TODO: headers, POST, GET, files, env vars, ...