/**
 * \file qsftest.cpp
 * \brief QSF app with a set of basic tests.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "app.h"

#include <iostream>
#include <random>
#include <soru/Engines/Argon2HashingEngine.h>
#include <soru/Encoding.h>
#include <soru/Session.h>
#include <soru/Universal.h>
#include <soru/Crypto.h>
#include <soru/Utils.h>

using namespace soru;

std::string genRandomUnicode(size_t len, unsigned int rseed) {
    const char cl[][50] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
                            "s", "t", "u", "v", "w", "x", "y", "z", " ", "\t", "<", ">", "\"", "=", "ä", "ö", "ü", "ß",
                            "é", "ó", "ú", "𝔸", "@", "#", "$", "€", "?", "!", "/", "\\", "-", "~"};
    std::stringstream ret;
    std::default_random_engine dre(rseed);
    std::uniform_int_distribution<size_t> distribution(0, 49);
    for(size_t i = len; i > 0; --i) {
        ret << cl[distribution(dre)];
    }
    return ret.str();
}

int init(soru::AppInit& appInit) {

    // GROUP 1: soru::Encoding

    std::string decoded;
    for(unsigned int rseed = 0; rseed < 10; ++rseed) {

        std::cout << "TESTING NOW WITH SEED " << rseed << std::endl;
        decoded = genRandomUnicode(100, rseed);

        // TEST 1.1: HTML encoding
        std::string htmlDecoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";
        std::string htmlEncoded = Encoding::htmlEncode(htmlDecoded, true);
        std::string htmlEncoded2 = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
        assert(htmlEncoded.length() > htmlDecoded.length());
        assert(Encoding::htmlDecode(htmlEncoded) == htmlDecoded);
        assert(htmlDecoded == Encoding::htmlDecode(htmlEncoded2));
        std::string htmlEncodedRand = Encoding::htmlEncode(decoded, true);
        std::string htmlEncodedRand2 = Encoding::htmlEncode(decoded, false);
        //std::cout << decoded << std::endl << htmlEncodedRand << std::endl << Encoding::htmlDecode(htmlEncodedRand) << std::endl;
        assert(Encoding::htmlDecode(htmlEncodedRand) == decoded);
        assert(Encoding::htmlDecode(htmlEncodedRand2) == decoded);
        std::cout << "TEST 1.1 passed" << std::endl;

        // TEST 1.2: URL encoding
        std::string urlDecoded = "bla bla bla!??xyzäßédsfsdf ";
        auto urlEncoded = Encoding::urlEncode(urlDecoded);
        auto urlEncodedRand = Encoding::urlEncode(decoded);
        assert(Encoding::urlDecode(urlEncoded) == urlDecoded);
        assert(Encoding::urlDecode(urlEncodedRand) == decoded);
        std::cout << "TEST 1.2 passed" << std::endl;

        // TEST 1.3: BASE64
        auto base64Encoded = Encoding::base64Encode(decoded, 80, "\r\n");
        //std::cout << decoded << std::endl << base64Encoded << std::endl << Encoding::base64Decode(base64Encoded) << std::endl;
        assert(Encoding::isBase64(base64Encoded, true));
        assert(Encoding::base64Decode(base64Encoded) == decoded);
        std::cout << "TEST 1.3 passed" << std::endl;

        // TEST 1.4: quoted-printable
        auto qpEncoded = Encoding::quotedPrintableEncode(decoded);
        //std::cout << decoded << std::endl << qpEncoded << std::endl << Encoding::quotedPrintableDecode(qpEncoded) << std::endl;
        assert(Encoding::quotedPrintableDecode(qpEncoded) == decoded);
        std::cout << "TEST 1.4 passed" << std::endl;

        // GROUP 2: soru::Crypto

        // TEST 2.1: password hashing using bcrypt
        auto hashedPw = Crypto::passwordHash(decoded, Engines::BcryptHashingEngine(8));
        auto startTime = std::chrono::steady_clock::now();
        assert(Crypto::passwordVerify(decoded, hashedPw));
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime);
        //std::cout << decoded << std::endl << hashedPw << std::endl;
        std::cout << "TEST 2.1 passed, took " << elapsed.count() << " µs" << std::endl;

        // TEST 2.2: password hashing using argon2
        hashedPw = Crypto::passwordHash(decoded, Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2ID, 2, 1 << 16, 2, "", 40));
        auto hashedPw_i = Crypto::passwordHash(decoded, Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2I));
        auto hashedPw_d = Crypto::passwordHash(decoded, Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2D));
        startTime = std::chrono::steady_clock::now();
        std::cout << decoded << std::endl << hashedPw << std::endl;
        std::cout << hashedPw_i << std::endl << hashedPw_d << std::endl;
        //auto he = Engines::Argon2HashingEngine();
        assert(Crypto::passwordVerify(decoded, hashedPw));
        assert(Crypto::passwordVerify(decoded, hashedPw_i));
        assert(Crypto::passwordVerify(decoded, hashedPw_d));
        elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime);
        std::cout << "TEST 2.2 passed, took " << elapsed.count() << " µs" << std::endl;
    }

    // GROUP 3 (utils)

    // TEST 3.1: Time conversions
    time_t currentTime = time(nullptr);
    assert(read_http_time(make_http_time(currentTime)) == currentTime);
    assert(read_smtp_time(make_smtp_time(currentTime)) == currentTime);
    std::cout << "TEST 3.1 passed" << std::endl;

    // GROUP 4: soru::Universal

    // TEST 4.1: Universal test
    Universal u1(decoded);
    Universal u2;
    u2 = 5;
    assert(u2.get<int>() == 5);
    u2 = u1;
    u1.unset();
    Universal u3 = u2;
    u2 = std::string("test2xyz");
    assert(u3.get<std::string>() == decoded);
    try {
        auto test = u2.get<int>();
        std::cerr << "TEST 4.1 failed: could get an int from a string Universal" << std::endl;
        exit(1);
    }
    catch(const UserException&) {}
    std::cout << "TEST 4.1 passed" << std::endl;

    exit(0);

    // enable access filtering
    appInit.accessFilters.filtersEnabled = true;

    // apply a forward filter for images
    ForwardFilter forwardFilter;
    forwardFilter.pathFilter = {"test", "images"};
    forwardFilter.extensionFilter = "png";
    forwardFilter.basePath = "/home/tobias/Pictures";
    appInit.accessFilters.forwardFilters.push_back(forwardFilter);

    // apply a block filter for everything that is not in /test or /test/images (and some more restrictions)
    BlockFilter blockFilter;
    blockFilter.invert = true;
    blockFilter.regexFilterEnabled = true;
    blockFilter.regexFilter.assign(R"(/test(/images)?(/[A-Za-z0-9_\-]*\.?[A-Za-z]{2,4})?)");
    blockFilter.status = 404;
    appInit.accessFilters.blockFilters.push_back(blockFilter);

    // authenticate access to the images directory
    AuthFilter authFilter;
    authFilter.pathFilter = {"test", "images"};
    authFilter.authName = "Not for everyone!";
    authFilter.authFunction = [](std::string user, std::string password) -> bool {
        return (user == "test" && password == "supersecure");
    };
    appInit.accessFilters.authFilters.push_back(authFilter);

    return 0;
}

int handleRequest(Connection &connection) {

    // start a session
    connection.session.start();

    connection.setCookie("TEST", Cookie("test"));
    Cookie policy;
    policy.httpOnly = true;
    connection.setCookiePolicy(policy);

    std::string encoded = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
    std::string decoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";

    connection.response << "<!DOCTYPE html>\n"
                  "<html><head><title>Test</title></head><body>"
                  "<p>Hello World! HTML string: " << Encoding::htmlEncode(decoded, true) << "</p>"
                  "<p>Client IP: " << Encoding::htmlEncode(connection.request.env["remoteAddress"]) << "</p>"
                  "<p>Request URI: (" << connection.request.env.getRequestPath().size() << " elements): "
                  << connection.request.env["requestUri"] << "</p>";

    // alternative: connection.session["test"].isSet()
    if(connection.session.isSet("test")) {
        connection.response << "<p>Session available! Value: " << connection.session["test"].get<std::string>() << "</p>";
        connection.session.invalidate();
        connection.session.start();
        connection.session.set("test", std::string("and even more blah"));
        
    }
    else {
        connection.session.set("test", "blah blah blah");
        connection.response << "<p>There was no session yet, but now there should be one!" << "</p>";
    }

    std::string encodedDecoded = Encoding::htmlDecode(encoded);
    if(decoded == encodedDecoded) {
        connection.response << "<p>yay!</p>";
    }
    else {
        connection.response << "<p>" << Encoding::htmlEncode(encodedDecoded) <<"</p>";
    }

    connection.flushResponse();

    connection.response << "<p>Hello World 2!</p>"
                  "</body></html>";

    return 0;
}
