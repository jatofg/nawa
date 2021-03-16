/**
 * \file BcryptHashingEngine.cpp
 * \brief Implementation of the hashing::BcryptHashingEngine class.
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
#include <libbcrypt/bcrypt.h>
#include <nawa/Exception.h>
#include <nawa/hashing/HashingEngine/impl/BcryptHashingEngine.h>

using namespace nawa;
using namespace std;

struct hashing::BcryptHashingEngine::Data {
    int workFactor;
    string salt;

    Data(int workFactor, string salt) : workFactor(workFactor), salt(move(salt)) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(hashing, BcryptHashingEngine)

hashing::BcryptHashingEngine::BcryptHashingEngine(int workFactor, string salt) {
    data = make_unique<Data>(workFactor, move(salt));
}

string hashing::BcryptHashingEngine::generateHash(string input) const {
    char bcsalt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];

    // use the user-defined salt if necessary
    if (!data->salt.empty()) {
        string salt_res = data->salt;
        salt_res.resize(BCRYPT_HASHSIZE, '\0');
        memcpy(hash, salt_res.c_str(), BCRYPT_HASHSIZE);
    } else if (bcrypt_gensalt(data->workFactor, bcsalt) != 0) {
        throw Exception(__PRETTY_FUNCTION__, 10,
                        "Could not generate a salt (unknown bcrypt failure).");
    }

    if (bcrypt_hashpw(input.c_str(), bcsalt, hash) != 0) {
        throw Exception(__PRETTY_FUNCTION__, 11,
                        "Could not hash this password (unknown bcrypt failure).");
    }
    return string(hash, 60);
}

bool hashing::BcryptHashingEngine::verifyHash(string input, string hash) const {
    // return value of bcrypt_checkpw is -1 on failure, 0 on match, and >0 if not matching
    int ret = bcrypt_checkpw(input.c_str(), hash.c_str());
    return ret == 0;
}
