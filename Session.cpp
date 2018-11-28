//
// Created by tobias on 23/11/18.
//

#include "Session.h"
#include "Utils.h"
#include <random>
#include <openssl/sha.h>

Qsf::Session::Session(Qsf::Connection &connection) : connection(connection) {
    // do not initialize session yet -> extra function
    // receive from connection.request or generate session key
    // check for or generate corresponding object?
    // multiple threads can access ONE SINGLE session at once! -> locking on the values!
    // lock while reading and writing
}

std::string Qsf::Session::generateID() {
    std::stringstream base;

    // Add 2 ints from random_device (should be in fact /dev/urandom), giving us (in general) 64 bits of entropy
    std::random_device rd;
    base << rd() << rd();

    // Add client IP
    base << connection.request.env["remoteAddress"];

    // Calculate SHA1
    // TODO create Qsf::Crypto namespace containing functions for sha1 etc.
    auto sha1BaseStr = base.str();
    auto sha1Base = (const unsigned char*) sha1BaseStr.c_str();
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    size_t sha1Size = sha1BaseStr.size();
    SHA1(sha1Base, sha1Size, sha1Hash);

    return hex_dump(std::string((char*)sha1Hash, SHA_DIGEST_LENGTH));
}

void Qsf::Session::start(Cookie properties) {
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
        // TODO check for validity
    }
    else {
        sessionCookieStr = generateID();
        // TODO check for duplicate
    }
    // set the response cookie and its properties according to the properties or
    // TODO special properties for session cookie -> extra private method for generating Cookie object?
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
    // TODO load or create SessionData object and add to data (locking!)
}
