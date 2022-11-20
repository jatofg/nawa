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
 * \file encoding_crypto.cpp
 * \brief Unit tests for the nawa::Encoding and nawa::Crypto functions.
 */

#include <catch2/catch.hpp>
#include <nawa/hashing/HashingEngine/impl/Argon2HashingEngine.h>
#include <nawa/util/crypto.h>
#include <nawa/util/encoding.h>
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
                               "s", "t", "u", "v", "w", "x", "y", "z", " ", "\t", "<", ">", "\"", "=", "ä", "ö", "ü",
                               "ß",
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
}// namespace

TEST_CASE("nawa::encoding functions", "[unit][encoding]") {
    SECTION("punycode encoding") {
        string punycodeEncoded = "xn--bcher-kuflich-erwrben-c2b9jut.xy";
        string punycodeDecoded = "bücher-käuflich-erwérben.xy";
        string asciiDomain = "example.com";
        CHECK(encoding::punycodeEncode(punycodeDecoded) == punycodeEncoded);
        CHECK(encoding::punycodeDecode(punycodeEncoded) == punycodeDecoded);
        CHECK(encoding::punycodeEncode(asciiDomain) == asciiDomain);
    }

    initializeInputDataIfNotYetDone();
    string decoded = GENERATE(from_range(inputData));

    SECTION("HTML encoding") {
        string htmlDecoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";
        string htmlEncoded = encoding::htmlEncode(htmlDecoded, true);
        string htmlEncoded2 = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
        CHECK(htmlEncoded.length() > htmlDecoded.length());
        CHECK(encoding::htmlDecode(htmlEncoded) == htmlDecoded);
        CHECK(htmlDecoded == encoding::htmlDecode(htmlEncoded2));
        string htmlEncodedRand = encoding::htmlEncode(decoded, true);
        string htmlEncodedRand2 = encoding::htmlEncode(decoded, false);
        CHECK(encoding::htmlDecode(htmlEncodedRand) == decoded);
        CHECK(encoding::htmlDecode(htmlEncodedRand2) == decoded);
    }

    SECTION("URL encoding") {
        string urlDecoded = "bla bla bla!??xyzäßédsfsdf ";
        auto urlEncoded = encoding::urlEncode(urlDecoded);
        auto urlEncodedRand = encoding::urlEncode(decoded);
        CHECK(encoding::urlDecode(urlEncoded) == urlDecoded);
        CHECK(encoding::urlDecode(urlEncodedRand) == decoded);
    }

    SECTION("Base64 encoding") {
        auto base64Encoded = encoding::base64Encode(decoded, 80, "\r\n");
        CHECK(encoding::isBase64(base64Encoded, true));
        CHECK(encoding::base64Decode(base64Encoded) == decoded);
    }

    SECTION("quoted-printable encoding") {
        auto quotedPrintableEncoded = encoding::quotedPrintableEncode(decoded);
        auto quotedPrintableEncodedWithUnixLineEnding = encoding::quotedPrintableEncode(decoded, "\n");
        auto quotedPrintableEncodedReplaceCrlf = encoding::quotedPrintableEncode(decoded, "\r\n", true);
        auto qEncoded = encoding::quotedPrintableEncode(decoded, "\r\n", false, true);
        auto qEncodedReplaceCrlf = encoding::quotedPrintableEncode(decoded, "\r\n", true, true);
        CHECK(encoding::quotedPrintableDecode(quotedPrintableEncoded) == decoded);
        CHECK(encoding::quotedPrintableDecode(quotedPrintableEncodedWithUnixLineEnding) == decoded);
        CHECK(encoding::quotedPrintableDecode(quotedPrintableEncodedReplaceCrlf) == decoded);
        CHECK(encoding::quotedPrintableDecode(qEncoded, true) == decoded);
        CHECK(encoding::quotedPrintableDecode(qEncodedReplaceCrlf, true) == decoded);
    }
}

TEST_CASE("nawa::crypto functions", "[unit][crypto]") {
    initializeInputDataIfNotYetDone();
    string decoded = GENERATE(from_range(inputData));

    SECTION("bcrypt password hashing") {
        auto hashedPw = crypto::passwordHash(decoded, hashing::BcryptHashingEngine(8));
        CHECK(crypto::passwordVerify(decoded, hashedPw));
    }

    SECTION("argon2 password hashing") {
        auto hashedPw = crypto::passwordHash(decoded,
                                             hashing::Argon2HashingEngine(
                                                     hashing::Argon2HashingEngine::Algorithm::ARGON2ID, 2, 1 << 16,
                                                     2, "", 40));
        auto hashedPw_i = crypto::passwordHash(decoded,
                                               hashing::Argon2HashingEngine(
                                                       hashing::Argon2HashingEngine::Algorithm::ARGON2I));
        auto hashedPw_d = crypto::passwordHash(decoded,
                                               hashing::Argon2HashingEngine(
                                                       hashing::Argon2HashingEngine::Algorithm::ARGON2D));
        CHECK(crypto::passwordVerify(decoded, hashedPw));
        CHECK(crypto::passwordVerify(decoded, hashedPw_i));
        CHECK(crypto::passwordVerify(decoded, hashedPw_d));
    }
}
