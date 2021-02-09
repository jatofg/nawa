/**
 * \file Session.cpp
 * \brief Implementation of the Session class.
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

#include <nawa/Exception.h>
#include <nawa/connection/Connection.h>
#include <nawa/session/Session.h>
#include <nawa/util/Crypto.h>
#include <random>

using namespace nawa;
using namespace std;

namespace {
    mutex gLock; /**< Lock for data. */
    /**
    * Map containing (pointers to) the session data for all sessions. The key is the session ID string.
    */
    unordered_map<string, shared_ptr<SessionData>> sessionData;

    /**
     * Generate a random, 40 chars session ID.
     * @param remoteAddress Remote address, which will be part of the random input.
     * @return The session ID.
     */
    string generateID(const string &remoteAddress) {
        stringstream base;

        // Add 2 ints from random_device (should be in fact /dev/urandom), giving us (in general) 64 bits of entropy
        random_device rd;
        base << rd() << rd();

        // Add client IP
        base << remoteAddress;

        // Calculate and return hex-formatted SHA1
        return Crypto::sha1(base.str(), true);
    }

    /**
     * Garbage collection by removing every expired session from the data map.
     * Would be best if run async and in fixed intervals (or with 0.xx chance on certain session actions -> see php)
     */
    void collectGarbage() {
        lock_guard<mutex> lockGuard(gLock);
        // no increment in for statement as we want to remove elements
        for (auto it = sessionData.cbegin(); it != sessionData.cend();) {
            bool toDelete = false;
            {
                lock_guard<mutex> eGuard(it->second->eLock);
                toDelete = (it->second->expires < time(nullptr));
            }
            if (toDelete) {
                it = sessionData.erase(it);
            } else {
                ++it;
            }
        }
    }
}

Session::Session(Connection &connection) : connection(connection) {
    // session autostart cannot happen here yet, as connection.config is not yet available (dangling)
    // thus, it will be triggered by the Connection constructor
}

void Session::start(Cookie properties) {

    // if session already started, do not start it again
    if (established()) return;

    // get name of session cookie from config
    cookieName = connection.config[{"session", "cookie_name"}];
    if (cookieName.empty()) {
        cookieName = "SESSION";
    }

    // session duration
    unsigned long sessionKeepalive = 1800;
    if (properties.getMaxAge()) {
        sessionKeepalive = *properties.getMaxAge();
    } else {
        auto sessionKStr = connection.config[{"session", "keepalive"}];
        if (!sessionKStr.empty()) {
            try {
                sessionKeepalive = stoul(sessionKStr);
            }
            catch (invalid_argument &e) {
                sessionKeepalive = 1800;
            }
        }
    }

    // check whether client has submitted a session cookie
    auto sessionCookieStr = connection.request.cookie[cookieName];
    if (!sessionCookieStr.empty()) {
        // check for validity
        // global data map may be accessed concurrently by different threads
        lock_guard<mutex> lockGuard(gLock);
        if (sessionData.count(sessionCookieStr) == 1) {
            // read validate_ip setting from config (needed a few lines later)
            auto sessionValidateIP = connection.config[{"session", "validate_ip"}];
            // session already expired?
            if (sessionData.at(sessionCookieStr)->expires <= time(nullptr)) {
                sessionData.erase(sessionCookieStr);
            }
                // validate_ip enabled in NAWA config and IP mismatch?
            else if ((sessionValidateIP == "strict" || sessionValidateIP == "lax")
                     && sessionData.at(sessionCookieStr)->sourceIP != connection.request.env["remoteAddress"]) {
                if (sessionValidateIP == "strict") {
                    // in strict mode, session has to be invalidated
                    sessionData.erase(sessionCookieStr);
                }
            }
                // session is valid
            else {
                currentData = sessionData.at(sessionCookieStr);
                // reset expiry
                lock_guard<mutex> currentLock(currentData->eLock);
                currentData->expires = time(nullptr) + sessionKeepalive;
            }
        }
    }
    // if currentData not yet set (sessionCookieStr empty or invalid) -> initiate new session
    if (currentData.use_count() < 1) {
        // generate new session ID string (and check for duplicate - should not really occur)
        lock_guard<mutex> lockGuard(gLock);
        do {
            sessionCookieStr = generateID(connection.request.env["remoteAddress"]);
        } while (sessionData.count(sessionCookieStr) > 0);
        currentData = make_shared<SessionData>(connection.request.env["remoteAddr"]);
        currentData->expires = time(nullptr) + sessionKeepalive;
        sessionData[sessionCookieStr] = currentData;
    }

    // set the response cookie and its properties according to the Cookie parameter or the NAWA config
    string cookieExpiresStr;
    if (properties.getExpires() || connection.config[{"session", "cookie_expires"}] != "off") {
        properties.setExpires(time(nullptr) + sessionKeepalive);
        properties.setMaxAge(sessionKeepalive);
    } else {
        // we need to unset the maxAge value if it should not be used for the cookie
        properties.setMaxAge(nullopt);
    }

    if (!properties.getSecure() && connection.config[{"session", "cookie_secure"}] != "off") {
        properties.setSecure(true);
    }
    if (!properties.getHttpOnly() && connection.config[{"session", "cookie_httponly"}] != "off") {
        properties.setHttpOnly(true);
    }
    if (properties.getSameSite() == Cookie::SameSite::OFF) {
        auto sessionSameSite = connection.config[{"session", "cookie_samesite"}];
        if (sessionSameSite == "lax") {
            properties.setSameSite(Cookie::SameSite::LAX);
        } else if (sessionSameSite != "off") {
            properties.setSameSite(Cookie::SameSite::STRICT);
        }
    }

    // save the ID so we can invalidate the session
    currentID = sessionCookieStr;

    // set the content to the session ID and queue the cookie
    properties.setContent(sessionCookieStr);
    connection.setCookie(cookieName, properties);

    // run garbage collection in 1/x of invocations
    unsigned long divisor;
    try {
        auto divisorStr = connection.config[{"session", "gc_divisor"}];
        if (!divisorStr.empty()) {
            divisor = stoul(divisorStr);
        } else {
            divisor = 100;
        }
    }
    catch (invalid_argument &e) {
        divisor = 100;
    }
    random_device rd;
    if (rd() % divisor == 0) {
        collectGarbage();
    }
}

bool Session::established() const {
    return (currentData.use_count() > 0);
}

bool Session::isSet(const string &key) const {
    if (established()) {
        lock_guard<mutex> lockGuard(currentData->dLock);
        return (currentData->data.count(key) == 1);
    }
    return false;
}

any Session::operator[](const string &key) const {
    if (established()) {
        lock_guard<mutex> lockGuard(currentData->dLock);
        if (currentData->data.count(key) == 1) {
            return currentData->data.at(key);
        }
    }
    return any();
}

// doxygen bug requires std:: here
void Session::set(std::string key, const std::any &value) {
    if (!established()) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Session not established.");
    }
    lock_guard<mutex> lockGuard(currentData->dLock);
    currentData->data[move(key)] = value;
}

void Session::unset(const string &key) {
    if (!established()) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Session not established.");
    }
    lock_guard<mutex> lockGuard(currentData->dLock);
    currentData->data.erase(key);
}

[[maybe_unused]] void Session::destroy() {
    sessionData.clear();
}

void Session::invalidate() {

    // do nothing if no session has been established
    if (!established()) return;

    // reset currentData pointer, this will also make established() return false
    currentData.reset();

    // erase this session from the data map
    {
        lock_guard<mutex> lockGuard(gLock);
        sessionData.erase(currentID);
    }

    // unset the session cookie, so that a new session can be started
    connection.unsetCookie(cookieName);

}
