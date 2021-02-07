/**
 * \file encoding_crypto.cpp
 * \brief Unit tests for the nawa::Encoding and nawa::Crypto functions.
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
#include <nawa/hashing/HashingEngine/impl/Argon2HashingEngine.h>
#include <nawa/util/Crypto.h>
#include <nawa/util/Encoding.h>
#include <sstream>

using namespace nawa;
using namespace std;

namespace {
    bool isInputDataInitialized = false;
    vector<string> inputData;

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

    void initializeInputDataIfNotYetDone() {
        if (isInputDataInitialized) {
            return;
        }
        for (unsigned int rseed = 0; rseed < 10; ++rseed) {
            inputData.push_back(genRandomUnicode(100, rseed));
        }
        isInputDataInitialized = true;
    }
}

TEST_CASE("nawa::Encoding functions", "[encoding]") {
    initializeInputDataIfNotYetDone();
    string decoded = GENERATE(from_range(inputData));

    SECTION("HTML encoding") {
        string htmlDecoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";
        string htmlEncoded = Encoding::htmlEncode(htmlDecoded, true);
        string htmlEncoded2 = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
        REQUIRE(htmlEncoded.length() > htmlDecoded.length());
        REQUIRE(Encoding::htmlDecode(htmlEncoded) == htmlDecoded);
        REQUIRE(htmlDecoded == Encoding::htmlDecode(htmlEncoded2));
        string htmlEncodedRand = Encoding::htmlEncode(decoded, true);
        string htmlEncodedRand2 = Encoding::htmlEncode(decoded, false);
        REQUIRE(Encoding::htmlDecode(htmlEncodedRand) == decoded);
        REQUIRE(Encoding::htmlDecode(htmlEncodedRand2) == decoded);
    }

    SECTION("URL encoding") {
        string urlDecoded = "bla bla bla!??xyzäßédsfsdf ";
        auto urlEncoded = Encoding::urlEncode(urlDecoded);
        auto urlEncodedRand = Encoding::urlEncode(decoded);
        REQUIRE(Encoding::urlDecode(urlEncoded) == urlDecoded);
        REQUIRE(Encoding::urlDecode(urlEncodedRand) == decoded);
    }

    SECTION("Base64 encoding") {
        auto base64Encoded = Encoding::base64Encode(decoded, 80, "\r\n");
        REQUIRE(Encoding::isBase64(base64Encoded, true));
        REQUIRE(Encoding::base64Decode(base64Encoded) == decoded);
    }

    SECTION("quoted-printable encoding") {
        auto hashedPw = Crypto::passwordHash(decoded, Engines::BcryptHashingEngine(8));
        //auto startTime = chrono::steady_clock::now();
        REQUIRE(Crypto::passwordVerify(decoded, hashedPw));
        //auto elapsed = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime);
    }
}

TEST_CASE("nawa::Crypto functions", "[crypto]") {
    initializeInputDataIfNotYetDone();
    string decoded = GENERATE(from_range(inputData));

    SECTION("bcrypt password hashing") {
        auto hashedPw = Crypto::passwordHash(decoded, Engines::BcryptHashingEngine(8));
        //auto startTime = chrono::steady_clock::now();
        REQUIRE(Crypto::passwordVerify(decoded, hashedPw));
        //auto elapsed = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime);
    }

    SECTION("argon2 password hashing") {
        auto hashedPw = Crypto::passwordHash(decoded,
                                        Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2ID, 2, 1 << 16,
                                                                     2, "", 40));
        auto hashedPw_i = Crypto::passwordHash(decoded,
                                               Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2I));
        auto hashedPw_d = Crypto::passwordHash(decoded,
                                               Engines::Argon2HashingEngine(Engines::Argon2HashingEngine::ARGON2D));
        // auto startTime = chrono::steady_clock::now();
        REQUIRE(Crypto::passwordVerify(decoded, hashedPw));
        REQUIRE(Crypto::passwordVerify(decoded, hashedPw_i));
        REQUIRE(Crypto::passwordVerify(decoded, hashedPw_d));
        // auto elapsed = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime);
    }

}