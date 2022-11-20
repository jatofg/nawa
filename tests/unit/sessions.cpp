/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file sessions.cpp
 * \brief Unit tests for the nawa::Session class.
 */

#include <catch2/catch.hpp>
#include <nawa/Exception.h>
#include <nawa/connection/Connection.h>
#include <nawa/connection/ConnectionInitContainer.h>
#include <nawa/session/Session.h>

using namespace nawa;
using namespace std;

TEST_CASE("nawa::Session class", "[unit][session]") {
    ConnectionInitContainer connectionInit;
    connectionInit.requestInit.environment["REMOTE_ADDR"] = "1.2.3.4";

    SECTION("Basic session handling") {
        string sessionId;
        {
            Connection connection(connectionInit);
            auto& session = connection.session();
            session.start();
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
            sessionId = session.getID();
        }
        {
            ConnectionInitContainer connectionInit1 = connectionInit;
            connectionInit1.requestInit.cookieVars.insert({"SESSION", sessionId});
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            REQUIRE(session.getID() == sessionId);
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
    }

    SECTION("Basic session handling without cookies") {
        string sessionId;
        {
            Connection connection(connectionInit);
            auto& session = connection.session();
            sessionId = session.start("");
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
        {
            Connection connection(connectionInit);
            auto& session = connection.session();
            string newSessionId = session.start(sessionId);
            REQUIRE(newSessionId == sessionId);
            REQUIRE(session.getID() == sessionId);
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
    }

    SECTION("Session autostart") {
        ConnectionInitContainer connectionInit1 = connectionInit;
        connectionInit1.config.set({"session", "autostart"}, "on");
        string sessionId;
        {
            Connection connection(connectionInit1);
            auto& session = connection.session();
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
            sessionId = session.getID();
        }
        {
            connectionInit1.requestInit.cookieVars.insert({"SESSION", sessionId});
            Connection connection(connectionInit1);
            auto& session = connection.session();
            REQUIRE(session.getID() == sessionId);
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
    }

    SECTION("Attempt to access and modify inactive session") {
        Connection connection(connectionInit);
        auto& session = connection.session();
        CHECK_THROWS_AS(session.set("testKey", "testVal"), Exception);
        CHECK_THROWS_AS(session.unset("testKey"), Exception);
        CHECK_NOTHROW(session["testKey"]);
    }

    SECTION("Invalidation of sessions") {
        Connection connection(connectionInit);
        auto& session = connection.session();
        session.start();
        CHECK_NOTHROW(session.set("testKey", "testVal"));
        session.invalidate();
        CHECK_THROWS_AS(session.set("testKey", "testVal"), Exception);
    }

    SECTION("Client IP check: same IP") {
        ConnectionInitContainer connectionInit1 = connectionInit;
        string clientIPCheckMode = GENERATE("lax", "strict");
        connectionInit1.config.set({"session", "validate_ip"}, clientIPCheckMode);
        string sessionId;
        {
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
            sessionId = session.getID();
        }
        {
            connectionInit1.requestInit.cookieVars.insert({"SESSION", sessionId});
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            REQUIRE(session.getID() == sessionId);
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
    }

    SECTION("Client IP check: different IP (lax)") {
        ConnectionInitContainer connectionInit1 = connectionInit;
        connectionInit1.config.set({"session", "validate_ip"}, "lax");
        string sessionId;
        {
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
            sessionId = session.getID();
        }
        {
            ConnectionInitContainer connectionInit2 = connectionInit1;
            connectionInit2.requestInit.cookieVars.insert({"SESSION", sessionId});
            connectionInit2.requestInit.environment["REMOTE_ADDR"] = "1.2.3.5";
            Connection connection(connectionInit2);
            auto& session = connection.session();
            session.start();
            CHECK_FALSE(session.getID() == sessionId);
            CHECK_FALSE(session.isSet("testKey"));
        }
        {
            connectionInit1.requestInit.cookieVars.insert({"SESSION", sessionId});
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            REQUIRE(session.getID() == sessionId);
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
        }
    }

    SECTION("Client IP check: different IP (strict)") {
        ConnectionInitContainer connectionInit1 = connectionInit;
        connectionInit1.config.set({"session", "validate_ip"}, "strict");
        string sessionId;
        {
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            CHECK_NOTHROW(session.set("testKey", "testVal"));
            CHECK(any_cast<string>(session["testKey"]) == "testVal");
            sessionId = session.getID();
        }
        {
            ConnectionInitContainer connectionInit2 = connectionInit1;
            connectionInit2.requestInit.cookieVars.insert({"SESSION", sessionId});
            connectionInit2.requestInit.environment["REMOTE_ADDR"] = "1.2.3.5";
            Connection connection(connectionInit2);
            auto& session = connection.session();
            session.start();
            CHECK_FALSE(session.getID() == sessionId);
            CHECK_FALSE(session.isSet("testKey"));
        }
        {
            connectionInit1.requestInit.cookieVars.insert({"SESSION", sessionId});
            Connection connection(connectionInit1);
            auto& session = connection.session();
            session.start();
            CHECK_FALSE(session.getID() == sessionId);
            CHECK_FALSE(session.isSet("testKey"));
        }
    }
}
