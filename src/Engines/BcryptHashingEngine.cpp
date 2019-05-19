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

#include <soru/Engines/BcryptHashingEngine.h>
#include <soru/UserException.h>
#include <cstring>
#include "../libs/libbcrypt/bcrypt.h"

soru::Engines::BcryptHashingEngine::BcryptHashingEngine(int workFactor, std::string _salt) : workFactor(workFactor) {
    salt = std::move(_salt);
}

std::string soru::Engines::BcryptHashingEngine::generateHash(std::string input) const {
    char bcsalt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];

    // use the user-defined salt if necessary
    if(!salt.empty()) {
        std::string salt_res = salt;
        salt_res.resize(BCRYPT_HASHSIZE, '\0');
        std::memcpy(hash, salt_res.c_str(), BCRYPT_HASHSIZE);
    }
    else if(bcrypt_gensalt(workFactor, bcsalt) != 0) {
        throw UserException("soru::Engines::BcryptHashingEngine::generateHash", 10,
                "Could not generate a salt (unknown bcrypt failure).");
    }

    if(bcrypt_hashpw(input.c_str(), bcsalt, hash) != 0) {
        throw UserException("soru::Engines::BcryptHashingEngine::generateHash", 11,
                "Could not hash this password (unknown bcrypt failure).");
    }
    return std::string(hash, 60);
}

bool soru::Engines::BcryptHashingEngine::verifyHash(std::string input, std::string hash) const {
    // return value of bcrypt_checkpw is -1 on failure, 0 on match, and >0 if not matching
    int ret = bcrypt_checkpw(input.c_str(), hash.c_str());
    return ret == 0;
}
