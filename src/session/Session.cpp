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
 * \file Session.cpp
 * \brief Implementation of the Session class.
 */

#include <mutex>
#include <nawa/Exception.h>
#include <nawa/connection/Connection.h>
#include <nawa/session/Session.h>
#include <nawa/util/crypto.h>
#include <random>

using namespace nawa;
using namespace std;

namespace {
    /**
     * SessionData objects contain all data of one session.
     */
    struct SessionData {
        mutex dLock;                     /**< Lock for data. */
        mutex eLock;                     /**< Lock for expires.  */
        unordered_map<string, any> data; /**< Map containing all values of this session. */
        time_t expires;                  /**< Time when this session expires. */
        const string sourceIP;           /**< IP address of the session initiator, for optional IP checking. */

        /**
         * Construct an empty SessionData object without a source IP.
         */
        SessionData() : expires(0) {}

        /**
         * Construct an empty SessionData object with a source IP.
         * @param sIP IP address of the session initiator.
         */
        explicit SessionData(string sIP) : expires(0), sourceIP(std::move(sIP)) {}
    };

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
    string generateID(string const& remoteAddress) {
        stringstream base;

        // Add 2 ints from random_device (should be in fact /dev/urandom), giving us (in general) 64 bits of entropy
        random_device rd;
        base << rd() << rd();

        // Add client IP
        base << remoteAddress;

        // Calculate and return hex-formatted SHA1
        return crypto::sha1(base.str(), true);
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
}// namespace

struct Session::Data {
    nawa::Connection& connection; /**< Reference to the Connection object in order to access objects. */
    /**
     * Pointer to the session data struct for the current session, if established.
     * Can be used to check whether a session is established by checking shared_ptr::use_count()
     * (used by established()).
     */
    std::shared_ptr<SessionData> currentData;
    std::string currentID;  /**< The current session ID. */
    std::string cookieName; /**< Name of the session cookie, as determined by start(). */

    explicit Data(Connection& connection) : connection(connection) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Session)

Session::Session(Connection& connection) {
    data = make_unique<Data>(connection);

    // session autostart cannot happen here yet, as connection.config is not yet available (dangling)
    // thus, it will be triggered by the Connection constructor
}

std::string Session::start(std::string sessionId, std::optional<unsigned long> keepalive) {
    // if session already started, do not start it again
    if (established()) {
        return data->currentID;
    }

    // session duration
    unsigned long sessionKeepalive = 1800;
    if (keepalive) {
        sessionKeepalive = *keepalive;
    } else {
        auto sessionKStr = data->connection.config()[{"session", "keepalive"}];
        if (!sessionKStr.empty()) {
            try {
                sessionKeepalive = stoul(sessionKStr);
            } catch (invalid_argument& e) {
                sessionKeepalive = 1800;
            }
        }
    }

    if (!sessionId.empty()) {
        // check for validity
        // global data map may be accessed concurrently by different threads
        lock_guard<mutex> lockGuard(gLock);
        if (sessionData.count(sessionId) == 1) {
            // read validate_ip setting from config (needed a few lines later)
            auto sessionValidateIP = data->connection.config()[{"session", "validate_ip"}];
            // session already expired?
            if (sessionData.at(sessionId)->expires <= time(nullptr)) {
                sessionData.erase(sessionId);
            }
            // validate_ip enabled in NAWA config and IP mismatch?
            else if ((sessionValidateIP == "strict" || sessionValidateIP == "lax") &&
                     sessionData.at(sessionId)->sourceIP != data->connection.request().env()["REMOTE_ADDR"]) {
                if (sessionValidateIP == "strict") {
                    // in strict mode, session has to be invalidated
                    sessionData.erase(sessionId);
                }
            }
            // session is valid
            else {
                data->currentData = sessionData.at(sessionId);
                // reset expiry
                lock_guard<mutex> currentLock(data->currentData->eLock);
                data->currentData->expires = time(nullptr) + sessionKeepalive;
            }
        }
    }
    // if currentData not yet set (sessionCookieStr empty or invalid) -> initiate new session
    if (data->currentData.use_count() < 1) {
        // generate new session ID string (and check for duplicate - should not really occur)
        lock_guard<mutex> lockGuard(gLock);
        do {
            sessionId = generateID(data->connection.request().env()["REMOTE_ADDR"]);
        } while (sessionData.count(sessionId) > 0);
        data->currentData = make_shared<SessionData>(data->connection.request().env()["REMOTE_ADDR"]);
        data->currentData->expires = time(nullptr) + sessionKeepalive;
        sessionData[sessionId] = data->currentData;
    }

    // save the ID so we can invalidate the session
    data->currentID = sessionId;

    // run garbage collection in 1/x of invocations
    unsigned long divisor;
    try {
        auto divisorStr = data->connection.config()[{"session", "gc_divisor"}];
        if (!divisorStr.empty()) {
            divisor = stoul(divisorStr);
        } else {
            divisor = 100;
        }
    } catch (invalid_argument const& e) {
        divisor = 100;
    }
    random_device rd;
    if (rd() % divisor == 0) {
        collectGarbage();
    }

    return sessionId;
}

void Session::start(Cookie properties) {
    // if session already started, do not start it again
    if (established())
        return;

    // get name of session cookie from config
    data->cookieName = data->connection.config()[{"session", "cookie_name"}];
    if (data->cookieName.empty()) {
        data->cookieName = "SESSION";
    }

    // session duration
    unsigned long sessionKeepalive = 1800;
    if (properties.maxAge()) {
        sessionKeepalive = *properties.maxAge();
    } else {
        auto sessionKStr = data->connection.config()[{"session", "keepalive"}];
        if (!sessionKStr.empty()) {
            try {
                sessionKeepalive = stoul(sessionKStr);
            } catch (invalid_argument& e) {
                sessionKeepalive = 1800;
            }
        }
    }

    // the session ID may be given in a session cookie, if not, the string will be empty
    // Session::start will use the session, if present, and return a valid session ID
    string sessionId = start(data->connection.request().cookie()[data->cookieName], sessionKeepalive);

    // set the response cookie and its properties according to the Cookie parameter or the NAWA config
    string cookieExpiresStr;
    if (properties.expires() || data->connection.config()[{"session", "cookie_expires"}] != "off") {
        properties.expires(time(nullptr) + sessionKeepalive)
                .maxAge(sessionKeepalive);
    } else {
        // we need to unset the maxAge value if it should not be used for the cookie
        properties.maxAge(nullopt);
    }

    if (!properties.secure() && data->connection.config()[{"session", "cookie_secure"}] != "off") {
        properties.secure(true);
    }
    if (!properties.httpOnly() && data->connection.config()[{"session", "cookie_httponly"}] != "off") {
        properties.httpOnly(true);
    }
    if (properties.sameSite() == Cookie::SameSite::OFF) {
        auto sessionSameSite = data->connection.config()[{"session", "cookie_samesite"}];
        if (sessionSameSite == "lax") {
            properties.sameSite(Cookie::SameSite::LAX);
        } else if (sessionSameSite != "off") {
            properties.sameSite(Cookie::SameSite::STRICT);
        }
    }

    // save the ID so we can invalidate the session
    data->currentID = sessionId;

    // set the content to the session ID and queue the cookie
    properties.content(sessionId);
    data->connection.setCookie(data->cookieName, properties);
}

bool Session::established() const {
    return (data->currentData.use_count() > 0);
}

bool Session::isSet(std::string const& key) const {
    if (established()) {
        lock_guard<mutex> lockGuard(data->currentData->dLock);
        return (data->currentData->data.count(key) == 1);
    }
    return false;
}

std::any Session::operator[](std::string const& key) const {
    if (established()) {
        lock_guard<mutex> lockGuard(data->currentData->dLock);
        if (data->currentData->data.count(key) == 1) {
            return data->currentData->data.at(key);
        }
    }
    return {};
}

// doxygen bug requires std:: here
void Session::set(std::string key, std::any const& value) {
    if (!established()) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Session not established.");
    }
    lock_guard<mutex> lockGuard(data->currentData->dLock);
    data->currentData->data[std::move(key)] = value;
}

void Session::unset(std::string const& key) {
    if (!established()) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Session not established.");
    }
    lock_guard<mutex> lockGuard(data->currentData->dLock);
    data->currentData->data.erase(key);
}

void Session::invalidate() {

    // do nothing if no session has been established
    if (!established())
        return;

    // reset currentData pointer, this will also make established() return false
    data->currentData.reset();

    // erase this session from the data map
    {
        lock_guard<mutex> lockGuard(gLock);
        sessionData.erase(data->currentID);
    }

    // unset the session cookie, so that a new session can be started
    data->connection.unsetCookie(data->cookieName);
}

std::string Session::getID() const {
    return established() ? data->currentID : string();
}

void Session::destroy() {
    sessionData.clear();
}
