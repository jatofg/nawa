//
// Created by tobias on 23/11/18.
//

#include "Session.h"
#include "Connection.h"
#include "Crypto.h"
#include "UserException.h"
#include <random>

std::mutex Qsf::Session::gLock;
std::unordered_map<std::string, std::shared_ptr<Qsf::SessionData>> Qsf::Session::data;

Qsf::Session::Session(Qsf::Connection &connection) : connection(connection) {
    // check if autostart is enabled in config and if yes, directly call ::start
    // TODO session autostart - the following will cause a SIGFPE
//    if(connection.config[{"session", "autostart"}] == "on") {
//        start();
//    }
}

std::string Qsf::Session::generateID() {
    std::stringstream base;

    // Add 2 ints from random_device (should be in fact /dev/urandom), giving us (in general) 64 bits of entropy
    std::random_device rd;
    base << rd() << rd();

    // Add client IP
    base << connection.request.env["remoteAddress"];

    // Calculate and return hex-formatted SHA1
    return Crypto::sha1(base.str(), true);
}

void Qsf::Session::start(Cookie properties) {

    // TODO check everything really carefully for possible race conditions

    // get name of session cookie from config
    auto sessionCookieName = connection.config[{"session", "cookie_name"}];
    if(sessionCookieName.empty()) {
        // TODO change?
        sessionCookieName = "SESSION";
    }

    // session duration
    unsigned long sessionKeepalive = 1800;
    if(properties.maxAge > 0) {
        sessionKeepalive = properties.maxAge;
    }
    else {
        auto sessionKStr = connection.config[{"session", "keepalive"}];
        if(!sessionKStr.empty()) {
            try {
                sessionKeepalive = std::stoul(sessionKStr);
            }
            catch(std::invalid_argument& e) {
                sessionKeepalive = 1800;
            }
        }
    }

    // check whether client has submitted a session cookie
    auto sessionCookieStr = connection.request.cookie[sessionCookieName];
    if(!sessionCookieStr.empty()) {
        // check for validity
        // global data map may be accessed concurrently by different threads
        std::lock_guard<std::mutex> lockGuard(gLock);
        if(data.count(sessionCookieStr) == 1) {
            // read validate_ip setting from config (needed a few lines later)
            auto sessionValidateIP = connection.config[{"session", "validate_ip"}];
            // session already expired?
            if(data.at(sessionCookieStr)->expires <= time(nullptr)) {
                data.erase(sessionCookieStr);
            }
            // validate_ip enabled in QSF config and IP mismatch?
            else if((sessionValidateIP == "strict" || sessionValidateIP == "lax")
                    && data.at(sessionCookieStr)->sourceIP != connection.request.env["remoteAddress"]) {
                if(sessionValidateIP == "strict") {
                    // in strict mode, session has to be invalidated
                    data.erase(sessionCookieStr);
                }
            }
            // session is valid
            else {
                currentData = data.at(sessionCookieStr);
                // reset expiry
                std::lock_guard<std::mutex> currentLock(currentData->eLock);
                currentData->expires = time(nullptr) + sessionKeepalive;
            }
        }
    }
    // if currentData not yet set (sessionCookieStr empty or invalid) -> initiate new session
    if(currentData.use_count() < 1) {
        // generate new session ID string (and check for duplicate - should not really occur)
        std::lock_guard<std::mutex> lockGuard(gLock);
        do {
            sessionCookieStr = generateID();
        } while(data.count(sessionCookieStr) > 0);
        currentData = std::make_shared<Qsf::SessionData>(connection.request.env["remoteAddr"]);
        currentData->expires = time(nullptr) + sessionKeepalive;
        data[sessionCookieStr] = currentData;
    }

    // set the response cookie and its properties according to the Cookie parameter or the QSF config
    std::string cookieExpiresStr;
    if(properties.expires > 0 || connection.config[{"session", "cookie_expires"}] != "off") {
        properties.expires = time(nullptr) + sessionKeepalive;
        properties.maxAge = sessionKeepalive;
    }
    else {
        // we need to reset the maxAge value to 0 if it should not be used for the cookie
        properties.maxAge = 0;
    }

    if(!properties.secure && connection.config[{"session", "cookie_secure"}] != "off") {
        properties.secure = true;
    }
    if(!properties.httpOnly && connection.config[{"session", "cookie_httponly"}] != "off") {
        properties.httpOnly = true;
    }
    if(properties.sameSite == 0) {
        auto sessionSameSite = connection.config[{"session", "cookie_samesite"}];
        if(sessionSameSite == "lax") {
            properties.sameSite = 1;
        }
        else if(sessionSameSite != "off") {
            properties.sameSite = 2;
        }
    }

    // set the content to the session ID and queue the cookie
    properties.content = sessionCookieStr;
    connection.setCookie(sessionCookieName, properties);

    // run garbage collection in 1/x of invocations
    unsigned long divisor;
    try {
        auto divisorStr = connection.config[{"session", "gc_divisor"}];
        if(!divisorStr.empty()) {
            divisor = std::stoul(divisorStr);
        }
        else {
            divisor = 100;
        }
    }
    catch(std::invalid_argument& e) {
        divisor = 100;
    }
    std::random_device rd;
    if(rd() % divisor == 0) {
        // TODO make this async somehow
        collectGarbage();
    }
}

bool Qsf::Session::established() const {
    return (currentData.use_count() > 0);
}

bool Qsf::Session::isSet(std::string key) const {
    if(established()) {
        std::lock_guard<std::mutex> lockGuard(currentData->dLock);
        return (currentData->data.count(key) == 1);
    }
    return false;
}

Qsf::Types::Universal Qsf::Session::operator[](std::string key) const {
    if(established()) {
        std::lock_guard<std::mutex> lockGuard(currentData->dLock);
        if(currentData->data.count(key) == 1) {
            return currentData->data.at(key);
        }
    }
    return Types::Universal();
}

void Qsf::Session::set(std::string key, const Qsf::Types::Universal& value) {
    if(!established()) {
        throw UserException("Qsf::Session::set", 1, "Session not established.");
    }
    std::lock_guard<std::mutex> lockGuard(currentData->dLock);
    currentData->data[std::move(key)] = value;
}

void Qsf::Session::collectGarbage() {
    std::lock_guard<std::mutex> lockGuard(gLock);
    // no increment in for statement as we want to remove elements
    for(auto it = data.cbegin(); it != data.cend();) {
        bool toDelete = false;
        {
            std::lock_guard<std::mutex> eGuard(it->second->eLock);
            toDelete = (it->second->expires < time(nullptr));
        }
        if(toDelete) {
            it = data.erase(it);
        }
        else {
            ++it;
        }
    }
}
