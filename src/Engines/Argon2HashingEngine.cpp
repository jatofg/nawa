/**
 * \file Argon2HashingEngine.cpp
 * \brief Implementation of the Argon2HashingEngine class.
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

#include <cstring>
#include <random>
#include <regex>
#include <argon2.h>
#include <nawa/Encoding.h>
#include <nawa/Exception.h>
#include <nawa/Engines/Argon2HashingEngine.h>

using namespace nawa;
using namespace std;

Engines::Argon2HashingEngine::Argon2HashingEngine(Engines::Argon2HashingEngine::Algorithm algorithm,
                                                  uint32_t timeCost, uint32_t memoryCost, uint32_t parallelism,
                                                  string _salt, size_t hashLen)
        : algorithm(algorithm), timeCost(timeCost), memoryCost(memoryCost), parallelism(parallelism), hashLen(hashLen) {
    salt = move(_salt);
}

string Engines::Argon2HashingEngine::generateHash(string input) const {

    // check validity of parameters
    if (!salt.empty() && salt.length() < ARGON2_MIN_SALT_LENGTH) {
        throw Exception(__PRETTY_FUNCTION__, 10,
                        "Provided user-defined salt is not long enough");
    }

    string actualSalt = salt;
    if (salt.empty()) {
        // generate random salt (16 bytes)
        random_device rd;
        stringstream sstr;
        if (rd.entropy() == 32) {
            uniform_int_distribution<uint32_t> distribution(0, 0xffffffff);
            for (int i = 0; i < 4; ++i) {
                uint32_t val = distribution(rd);
                sstr << (char) (val & 0xff);
                sstr << (char) ((val >> 8) & 0xff);
                sstr << (char) ((val >> 16) & 0xff);
                sstr << (char) ((val >> 24) & 0xff);
            }
        } else {
            uniform_int_distribution<uint16_t> distribution(0, 0xffff);
            for (int i = 0; i < 8; ++i) {
                uint16_t val = distribution(rd);
                sstr << (char) (val & 0xff);
                sstr << (char) ((val >> 8) & 0xff);
            }
        }
        actualSalt = sstr.str();
    }

    int errorCode = 0;
    size_t encodedHashCeil = 50 + (actualSalt.length() * 4) / 3 + (hashLen * 4) / 3;
    char c_hash[encodedHashCeil];
    memset(c_hash, '\0', encodedHashCeil);

    switch (algorithm) {
        case ARGON2I:
            errorCode = argon2i_hash_encoded(timeCost, memoryCost, parallelism, (void *) input.c_str(), input.length(),
                                             (void *) actualSalt.c_str(), actualSalt.length(), hashLen, c_hash,
                                             encodedHashCeil);
            break;
        case ARGON2D:
            errorCode = argon2d_hash_encoded(timeCost, memoryCost, parallelism, (void *) input.c_str(), input.length(),
                                             (void *) actualSalt.c_str(), actualSalt.length(), hashLen, c_hash,
                                             encodedHashCeil);
            break;
        case ARGON2ID:
            errorCode = argon2id_hash_encoded(timeCost, memoryCost, parallelism, (void *) input.c_str(), input.length(),
                                              (void *) actualSalt.c_str(), actualSalt.length(), hashLen, c_hash,
                                              encodedHashCeil);
            break;
    }

    // error handling
    if (errorCode != ARGON2_OK) {
        throw Exception(__PRETTY_FUNCTION__, 10,
                        string("Argon2 error: ") + argon2_error_message(errorCode));
    }

    return string(c_hash);
}

bool Engines::Argon2HashingEngine::verifyHash(string input, string hash) const {

    // split the hash and create a new object with the properties of the hash
    regex rgx(
            R"(\$argon2(i|d|id)\$(v=([0-9]+))?\$m=([0-9]+),t=([0-9]+),p=([0-9]+)\$([A-Za-z0-9+\/]+={0,2})\$([A-Za-z0-9+\/]+={0,2}))");
    smatch matches;
    regex_match(hash, matches, rgx);
    Algorithm algorithm1;
    uint32_t version1;
    uint32_t memoryCost1;
    uint32_t timeCost1;
    uint32_t parallelism1;
    string salt1;
    string hash1;
    if (matches.size() == 9) {
        if (matches[1] == "d") algorithm1 = ARGON2D;
        else if (matches[1] == "id") algorithm1 = ARGON2ID;
        else algorithm1 = ARGON2I;
        try {
            version1 = stoul(matches[3]);
            memoryCost1 = stoul(matches[4]);
            timeCost1 = stoul(matches[5]);
            parallelism1 = stoul(matches[6]);
            salt1 = Encoding::base64Decode(matches[7]);
            hash1 = Encoding::base64Decode(matches[8]);
        }
        catch (...) {
            return false;
        }
    } else {
        return false;
    }
    if (version1 != 19) {
        return false;
    }

    auto engine1 = Argon2HashingEngine(algorithm1, timeCost1, memoryCost1, parallelism1, salt1, hash1.length());
    string inputHash;
    try {
        inputHash = engine1.generateHash(input);
        inputHash = Encoding::base64Decode(inputHash.substr(inputHash.find_last_of('$') + 1));
    }
    catch (const Exception &) {
        return false;
    }

    if (hash1.length() != inputHash.length())
        return false;

    auto u1 = (const unsigned char *) hash1.c_str();
    auto u2 = (const unsigned char *) inputHash.c_str();

    int ret = 0;
    for (int i = 0; i < hash1.length(); ++i)
        ret |= (u1[i] ^ u2[i]);

    return ret == 0;
}
