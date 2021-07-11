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
#include <nawa/Exception.h>
#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/connection/Connection.h>
#include <nawa/util/utils.h>

using namespace nawa;
using namespace std;

namespace http = boost::network::http;

Config loadConfig();

namespace {
    bool isEnvironmentInitialized = false;
    Config config;
    string port;
    string baseUrl;
    unique_ptr<RequestHandler> httpRequestHandler;

    /**
     * Initialize test environment if not yet done.
     * @return True on success, false on failure.
     */
    bool initializeEnvironmentIfNotYetDone() {
        if (isEnvironmentInitialized) {
            return true;
        }

        config = loadConfig();
        config.insert({
                {{"http", "reuseaddr"}, "on"},
                {{"post", "max_size"}, "1"},
                {{"system", "request_handler"}, "http"},
                {{"logging", "level"}, "debug"},
                {{"logging", "extended"}, "on"},
        });

        port = config[{"http", "port"}].empty() ? "8080" : config[{"http", "port"}];
        baseUrl = "http://127.0.0.1:" + port;

        REQUIRE_NOTHROW(
                httpRequestHandler = RequestHandler::newRequestHandler([](Connection&) { return 0; }, config, 1));

        isEnvironmentInitialized = true;
        return true;
    }
}// namespace

TEST_CASE("Basic request handling (HTTP)", "[basic][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());
    // GENERATE can be used to run test cases with both HTTP and FCGI request handler
    auto& requestHandler = httpRequestHandler;

    auto handlingFunction = [](Connection& connection) -> int {
        connection.responseStream() << "Hello World!";
        return 0;
    };
    REQUIRE_NOTHROW(
            requestHandler->reconfigure(make_shared<HandleRequestFunctionWrapper>(handlingFunction), nullopt, config));
    REQUIRE_NOTHROW(requestHandler->start());

    http::client client;
    http::client::request request(baseUrl);
    http::client::response response;
    REQUIRE_NOTHROW(response = client.get(request));
    CHECK(response.body() == "Hello World!");
}

TEST_CASE("Environment and headers (HTTP)", "[headers][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());
    auto& requestHandler = httpRequestHandler;

    auto handlingFunction = [](Connection& connection) -> int {
        auto& resp = connection.responseStream();
        auto& env = connection.request().env();

        resp << env.getRequestPath().size() << "\n";// [0]
        resp << env["REMOTE_ADDR"] << "\n";         // [1]
        resp << env["REQUEST_URI"] << "\n";         // [2]
        resp << env["REMOTE_PORT"] << "\n";         // [3]
        resp << env["REQUEST_METHOD"] << "\n";      // [4]
        resp << env["SERVER_ADDR"] << "\n";         // [5]
        resp << env["SERVER_PORT"] << "\n";         // [6]
        resp << env["SERVER_SOFTWARE"] << "\n";     // [7]
        resp << env["BASE_URL"] << "\n";            // [8]
        resp << env["FULL_URL_WITH_QS"] << "\n";    // [9]
        resp << env["FULL_URL_WITHOUT_QS"] << "\n"; // [10]
        resp << env["host"] << "\n";                // [11]
        resp << env["content-type"] << "\n";        // [12]

        // response headers
        connection.setHeader("x-test-header", "test");
        connection.setHeader("x-second-test", "test");
        connection.unsetHeader("x-second-test");
        return 0;
    };
    REQUIRE_NOTHROW(
            requestHandler->reconfigure(make_shared<HandleRequestFunctionWrapper>(handlingFunction), nullopt, config));
    REQUIRE_NOTHROW(requestHandler->start());

    http::client client;
    http::client::response response;

    auto checkResponse = [&](string const& method) {
        auto respLines = utils::splitString(response.body(), '\n');
        REQUIRE(respLines.size() == 13);
        CHECK(respLines[0] == "3");
        CHECK(respLines[1] == "127.0.0.1");
        CHECK(respLines[2] == "/tp0/tp1/test?qse0=v0&qse1=v1");
        CHECK(respLines[4] == method);
        CHECK(respLines[5] == "127.0.0.1");
        CHECK(respLines[6] == port);
        CHECK(respLines[8] == baseUrl);
        CHECK(respLines[9] == baseUrl + "/tp0/tp1/test?qse0=v0&qse1=v1");
        CHECK(respLines[10] == baseUrl + "/tp0/tp1/test");
        CHECK(respLines[11] == "127.0.0.1:" + port);

        REQUIRE(response.headers().count("x-test-header") == 1);
        CHECK(response.headers().equal_range("x-test-header").first->second == "test");
        CHECK(response.headers().count("x-second-test") == 0);
    };

    SECTION("GET request") {
        http::client::request request(baseUrl + "/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE_NOTHROW(response = client.get(request));
        checkResponse("GET");
    }

    SECTION("POST request") {
        http::client::request request(baseUrl + "/tp0/tp1/test?qse0=v0&qse1=v1");
        REQUIRE_NOTHROW(response = client.post(request));
        checkResponse("POST");
    }
}

TEST_CASE("Sessions (HTTP)", "[sessions][http]") {
    REQUIRE(initializeEnvironmentIfNotYetDone());
    auto& requestHandler = httpRequestHandler;

    auto handlingFunction = [](Connection& connection) -> int {
        auto& resp = connection.responseStream();
        auto& env = connection.request().env();
        auto& session = connection.session();

        session.start();
        if (!session.isSet("testKey")) {
            resp << "not set";
            session.set("testKey", "testVal");
        } else {
            try {
                resp << any_cast<string>(session["testKey"]);
            } catch (bad_any_cast const&) {
                resp << "bad cast";
            }
        }

        return 0;
    };

    REQUIRE_NOTHROW(
            requestHandler->reconfigure(make_shared<HandleRequestFunctionWrapper>(handlingFunction), nullopt, config));
    REQUIRE_NOTHROW(requestHandler->start());

    http::client client;
    http::client::response response;

    http::client::request request(baseUrl);
    REQUIRE_NOTHROW(response = client.get(request));
    REQUIRE(response.body() == "not set");
    auto cookieIterator = response.headers().find("Set-Cookie");
    REQUIRE(cookieIterator != response.headers().end());
    auto parsedCookie = utils::parseCookies(cookieIterator->second);
    auto parsedCookieIt = parsedCookie.find("SESSION");
    REQUIRE(parsedCookieIt != parsedCookie.end());
    string sessionId = parsedCookieIt->second;

    request << boost::network::header("Cookie", "SESSION=" + sessionId);
    REQUIRE_NOTHROW(response = client.get(request));
    REQUIRE(response.body() == "testVal");
}
