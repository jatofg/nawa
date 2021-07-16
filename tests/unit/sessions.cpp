/**
 * \file sessions.cpp
 * \brief Unit tests for the nawa::Session class.
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

#include <catch2/catch.hpp>
#include <nawa/connection/Connection.h>
#include <nawa/connection/ConnectionInitContainer.h>
#include <nawa/session/Session.h>

using namespace nawa;
using namespace std;

TEST_CASE("nawa::Session class", "[unit][session]") {
    ConnectionInitContainer connectionInit;
    connectionInit.requestInit.environment["REMOTE_ADDR"] = "1.2.3.4";
    Connection connection(connectionInit);
    Session session(connection);
    session.start();
    session.set("testKey", "testVal");
    CHECK(any_cast<string>(session["testKey"]) == "testVal");
    string sessionId = session.getID();

    connectionInit.requestInit.cookieVars.insert({"SESSION", sessionId});
    Connection connection2(connectionInit);
    Session session2(connection2);
    session2.start();
    REQUIRE(session2.getID() == sessionId);
    CHECK(any_cast<string>(session2["testKey"]) == "testVal");
}

// test cases: autostart on/off, client IP check, all session-specific config options,
// not-yet-started session, destroyed session, ...