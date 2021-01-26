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
#include <nawa/Utils.h>
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

        config.insert({
                              {{"http",    "port"},            "8089"},
                              {{"http",    "reuseaddr"},       "on"},
                              {{"post",    "max_size"},        "1"},
                              {{"system",  "request_handler"}, "http"},
                              {{"logging", "level"},           "debug"},
                              {{"logging", "extended"},        "on"},
                      });

        isEnvironmentInitialized = true;
        return true;
    }
}

TEST_CASE("Basic request handling (HTTP)", "[basic][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());

    auto handlingFunction = [](Connection &connection) -> int {
        connection.response << "Hello World!";
        return 0;
    };
    unique_ptr<RequestHandler> requestHandler;
    REQUIRE_NOTHROW(requestHandler = RequestHandler::newRequestHandler(handlingFunction, config, 1));
    REQUIRE_NOTHROW(requestHandler->start());

    http::client client;
    http::client::request request("http://127.0.0.1:8089/");
    http::client::response response;
    REQUIRE_NOTHROW(response = client.get(request));
    REQUIRE(response.body() == "Hello World!");

    requestHandler->terminate();
    requestHandler->join();
}

TEST_CASE("Environment and request headers (HTTP)", "[headers][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());

    auto handlingFunction = [](Connection &connection) -> int {
        connection.response << connection.request.env.getRequestPath().size() << "\n"; // [0]
        connection.response << connection.request.env["REMOTE_ADDR"] << "\n"; // [1]
        connection.response << connection.request.env["REQUEST_URI"] << "\n"; // [2]
        connection.response << connection.request.env["REMOTE_PORT"] << "\n"; // [3]
        connection.response << connection.request.env["REQUEST_METHOD"] << "\n"; // [4]
        connection.response << connection.request.env["SERVER_ADDR"] << "\n"; // [5]
        connection.response << connection.request.env["SERVER_PORT"] << "\n"; // [6]
        connection.response << connection.request.env["SERVER_SOFTWARE"] << "\n"; // [7]
        connection.response << connection.request.env["BASE_URL"] << "\n"; // [8]
        connection.response << connection.request.env["FULL_URL_WITH_QS"] << "\n"; // [9]
        connection.response << connection.request.env["FULL_URL_WITHOUT_QS"] << "\n"; // [10]
        connection.response << connection.request.env["host"] << "\n"; // [11]
        connection.response << connection.request.env["content-type"] << "\n"; // [12]
        return 0;
    };
    unique_ptr<RequestHandler> requestHandler;
    REQUIRE_NOTHROW(requestHandler = RequestHandler::newRequestHandler(handlingFunction, config, 1));
    REQUIRE_NOTHROW(requestHandler->start());

    http::client client;

    SECTION("GET request") {
        http::client::request request("http://127.0.0.1:8089/tp0/tp1/test?qse0=v0&qse1=v1");
        http::client::response response;
        REQUIRE_NOTHROW(response = client.get(request));
        auto respLines = split_string(response.body(), '\n');

        REQUIRE(respLines.size() == 13);
        REQUIRE(respLines[0] == "3");
        REQUIRE(respLines[1] == "127.0.0.1");
        REQUIRE(respLines[2] == "/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE(respLines[4] == "GET");
        REQUIRE(respLines[5] == "127.0.0.1");
        REQUIRE(respLines[6] == "8089");
        REQUIRE(respLines[8] == "http://127.0.0.1:8089");
        REQUIRE(respLines[9] == "http://127.0.0.1:8089/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE(respLines[10] == "http://127.0.0.1:8089/tp0/tp1/test");
        REQUIRE(respLines[11] == "127.0.0.1:8089");
    }

    SECTION("POST request") {
        http::client::request request("http://127.0.0.1:8089/tp0/tp1/test?qse0=v0&qse1=v1");
        http::client::response response;
        REQUIRE_NOTHROW(response = client.post(request));
        auto respLines = split_string(response.body(), '\n');

        REQUIRE(respLines.size() == 13);
        REQUIRE(respLines[0] == "3");
        REQUIRE(respLines[1] == "127.0.0.1");
        REQUIRE(respLines[2] == "/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE(respLines[4] == "POST");
        REQUIRE(respLines[5] == "127.0.0.1");
        REQUIRE(respLines[6] == "8089");
        REQUIRE(respLines[8] == "http://127.0.0.1:8089");
        REQUIRE(respLines[9] == "http://127.0.0.1:8089/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE(respLines[10] == "http://127.0.0.1:8089/tp0/tp1/test");
        REQUIRE(respLines[11] == "127.0.0.1:8089");
    }

    requestHandler->terminate();
    requestHandler->join();
}

// TODO: POST, GET, files, env vars, ...