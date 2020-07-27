/**
 * \file assertions.cpp
 * \brief Unit tests for some basic components. No request handling, command line output only.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#include <nawa/Application.h>
#include <iostream>
#include <random>
#include <nawa/Engines/Argon2HashingEngine.h>
#include <nawa/Encoding.h>
#include <nawa/Crypto.h>
#include <nawa/Utils.h>

using namespace nawa;
using namespace std;

/**
 * Generate a random string of size len with a random combination of selected unicode characters.
 * @param len Length of the resulting string.
 * @param rseed Seed for the random engine.
 * @return The random string.
 */
std::string genRandomUnicode(size_t len, unsigned int rseed) {
    const char cl[][50] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
                           "s", "t", "u", "v", "w", "x", "y", "z", " ", "\t", "<", ">", "\"", "=", "ä", "ö", "ü", "ß",
                           "é", "ó", "ú", "𝔸", "@", "#", "$", "€", "?", "!", "/", "\\", "-", "~"};
    stringstream ret;
    default_random_engine dre(rseed);
    uniform_int_distribution<size_t> distribution(0, 49);
    for (size_t i = len; i > 0; --i) {
        ret << cl[distribution(dre)];
    }
    return ret.str();
}

int init(AppInit &appInit) {

    // Testing of encoding and crypto functions will happen with different random input strings
    string decoded;
    for (unsigned int rseed = 0; rseed < 10; ++rseed) {

        cout << "TESTING NOW WITH SEED " << rseed << endl;
        // generate a random string with this seed
        decoded = genRandomUnicode(100, rseed);

        // GROUP 1: Test correctness of the nawa::Encoding functions

        // TEST 1.1: HTML encoding
        string htmlDecoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";
        string htmlEncoded = Encoding::htmlEncode(htmlDecoded, true);
        string htmlEncoded2 = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
        assert(htmlEncoded.length() > htmlDecoded.length());
        assert(Encoding::htmlDecode(htmlEncoded) == htmlDecoded);
        assert(htmlDecoded == Encoding::htmlDecode(htmlEncoded2));
        string htmlEncodedRand = Encoding::htmlEncode(decoded, true);
        string htmlEncodedRand2 = Encoding::htmlEncode(decoded, false);
        //cout << decoded << endl << htmlEncodedRand << endl << Encoding::htmlDecode(htmlEncodedRand) << endl;
        assert(Encoding::htmlDecode(htmlEncodedRand) == decoded);
        assert(Encoding::htmlDecode(htmlEncodedRand2) == decoded);
        cout << "TEST 1.1 passed" << endl;

        // TEST 1.2: URL encoding
        string urlDecoded = "bla bla bla!??xyzäßédsfsdf ";
        auto urlEncoded = Encoding::urlEncode(urlDecoded);
        auto urlEncodedRand = Encoding::urlEncode(decoded);
        assert(Encoding::urlDecode(urlEncoded) == urlDecoded);
        assert(Encoding::urlDecode(urlEncodedRand) == decoded);
        cout << "TEST 1.2 passed" << endl;

        // TEST 1.3: BASE64 encoding
        auto base64Encoded = Encoding::base64Encode(decoded, 80, "\r\n");
        //cout << decoded << endl << base64Encoded << endl << Encoding::base64Decode(base64Encoded) << endl;
        assert(Encoding::isBase64(base64Encoded, true));
        assert(Encoding::base64Decode(base64Encoded) == decoded);
        cout << "TEST 1.3 passed" << endl;

        // TEST 1.4: quoted-printable encoding
        auto qpEncoded = Encoding::quotedPrintableEncode(decoded);
        //cout << decoded << endl << qpEncoded << endl << Encoding::quotedPrintableDecode(qpEncoded) << endl;
        assert(Encoding::quotedPrintableDecode(qpEncoded) == decoded);
        cout << "TEST 1.4 passed" << endl;

        // GROUP 2: Test correctness of nawa::Crypto functions

        // TEST 2.1: password hashing using bcrypt
        auto hashedPw = Crypto::passwordHash(decoded, Engines::BcryptHashingEngine(8));
        auto startTime = chrono::steady_clock::now();
        assert(Crypto::passwordVerify(decoded, hashedPw));
        auto elapsed = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime);
        //cout << decoded << endl << hashedPw << endl;
        cout << "TEST 2.1 passed, took " << elapsed.count() << " µs" << endl;

        // TEST 2.2: password hashing using argon2
        hashedPw = Crypto::passwordHash(decoded,
                                        Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2ID, 2, 1 << 16,
                                                                     2, "", 40));
        auto hashedPw_i = Crypto::passwordHash(decoded,
                                               Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2I));
        auto hashedPw_d = Crypto::passwordHash(decoded,
                                               Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2D));
        startTime = chrono::steady_clock::now();
        cout << decoded << endl << hashedPw << endl;
        cout << hashedPw_i << endl << hashedPw_d << endl;
        //auto he = Engines::Argon2HashingEngine();
        assert(Crypto::passwordVerify(decoded, hashedPw));
        assert(Crypto::passwordVerify(decoded, hashedPw_i));
        assert(Crypto::passwordVerify(decoded, hashedPw_d));
        elapsed = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime);
        cout << "TEST 2.2 passed, took " << elapsed.count() << " µs" << endl;
    }

    // GROUP 3: Test functions in nawa::Utils

    // TEST 3.1: Time conversions
    time_t currentTime = time(nullptr);
    string testTimeStr1 = "Thu,  7 Nov 2019 16:29:50 +0100";
    if (read_http_time(make_http_time(currentTime)) != currentTime) {
        cerr << "TEST 3.1.1 FAILED: currentTime = " << currentTime << "; make_http_time(currentTime) = "
             << make_http_time(currentTime) << "; read_http_time(...) = "
             << read_http_time(make_http_time(currentTime)) << endl;
        return 1;
    }
    if (read_smtp_time(make_smtp_time(currentTime)) != currentTime) {
        cerr << "TEST 3.1.2 FAILED: currentTime = " << currentTime << "; make_smtp_time(currentTime) = "
             << make_smtp_time(currentTime) << "; read_smtp_time(...) = "
             << read_smtp_time(make_smtp_time(currentTime)) << endl;
        return 1;
    }
    assert(make_smtp_time(read_smtp_time(testTimeStr1)) == testTimeStr1);
    cout << "TEST 3.1 passed" << endl;

    // TEST 3.2: Test path splitting
    string t1 = "p1/p2/p3";
    string t2 = "/p1/p2/p3";
    string t3 = "/p1/p2/p3/";
    string t4 = "/p1/p2/p3?test=/xyz";
    string t5 = "/p1/p2/p3/?test=/xyz/";
    auto t1_split = split_path(t1);
    assert(t1_split == split_path(t2) && t1_split == split_path(t3) && t1_split == split_path(t4)
           && t1_split == split_path(t5));
    cout << "TEST 3.2 passed" << endl;

    return 1;
}

int handleRequest(Connection &) {
    return 0;
}
