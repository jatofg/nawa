/**
 * \file Argon2HashingEngine.cpp
 * \brief Implementation of the hashing::Argon2HashingEngine class.
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

#include <argon2.h>
#include <cstring>
#include <nawa/Exception.h>
#include <nawa/hashing/HashingEngine/impl/Argon2HashingEngine.h>
#include <nawa/util/Encoding.h>
#include <random>
#include <regex>

using namespace nawa;
using namespace std;

struct hashing::Argon2HashingEngine::Impl {
    Algorithm algorithm; /**< The Argon2 flavor to use. */
    uint32_t timeCost; /**< Number of iterations. */
    uint32_t memoryCost; /**< Memory usage in kiB. */
    uint32_t parallelism; /**< Number of threads used. */
    string salt; /**< User-defined salt. */
    size_t hashLen; /**< Desired length of the hash. */

    Impl(Algorithm algorithm, uint32_t timeCost, uint32_t memoryCost, uint32_t parallelism, string salt, size_t hashLen)
            : algorithm(algorithm), timeCost(timeCost), memoryCost(memoryCost), parallelism(parallelism),
              salt(move(salt)), hashLen(hashLen) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(hashing, Argon2HashingEngine)

hashing::Argon2HashingEngine::Argon2HashingEngine(hashing::Argon2HashingEngine::Algorithm algorithm,
                                                  uint32_t timeCost, uint32_t memoryCost, uint32_t parallelism,
                                                  string salt, size_t hashLen) {
    impl = make_unique<Impl>(algorithm, timeCost, memoryCost, parallelism, move(salt), hashLen);
}

string hashing::Argon2HashingEngine::generateHash(string input) const {

    // check validity of parameters
    if (!impl->salt.empty() && impl->salt.length() < ARGON2_MIN_SALT_LENGTH) {
        throw Exception(__PRETTY_FUNCTION__, 10,
                        "Provided user-defined salt is not long enough");
    }

    string actualSalt = impl->salt;
    if (impl->salt.empty()) {
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
    size_t encodedHashCeil = 50 + (actualSalt.length() * 4) / 3 + (impl->hashLen * 4) / 3;
    char c_hash[encodedHashCeil];
    memset(c_hash, '\0', encodedHashCeil);

    switch (impl->algorithm) {
        case Algorithm::ARGON2I:
            errorCode = argon2i_hash_encoded(impl->timeCost, impl->memoryCost, impl->parallelism,
                                             (void *) input.c_str(), input.length(),
                                             (void *) actualSalt.c_str(), actualSalt.length(), impl->hashLen, c_hash,
                                             encodedHashCeil);
            break;
        case Algorithm::ARGON2D:
            errorCode = argon2d_hash_encoded(impl->timeCost, impl->memoryCost, impl->parallelism,
                                             (void *) input.c_str(), input.length(),
                                             (void *) actualSalt.c_str(), actualSalt.length(), impl->hashLen, c_hash,
                                             encodedHashCeil);
            break;
        case Algorithm::ARGON2ID:
            errorCode = argon2id_hash_encoded(impl->timeCost, impl->memoryCost, impl->parallelism,
                                              (void *) input.c_str(), input.length(),
                                              (void *) actualSalt.c_str(), actualSalt.length(), impl->hashLen, c_hash,
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

bool hashing::Argon2HashingEngine::verifyHash(string input, string hash) const {

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
        if (matches[1] == "d") algorithm1 = Algorithm::ARGON2D;
        else if (matches[1] == "id") algorithm1 = Algorithm::ARGON2ID;
        else algorithm1 = Algorithm::ARGON2I;
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
