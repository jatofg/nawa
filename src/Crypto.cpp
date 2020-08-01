/**
 * \file Crypto.cpp
 * \brief Implementation of the Crypto class.
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

#include <nawa/Crypto.h>
#include <nawa/Utils.h>
#include <nawa/Exception.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

using namespace nawa;
using namespace std;

std::string Crypto::sha1(const std::string &input, bool hex) {
    auto sha1Base = (const unsigned char *) input.c_str();
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    size_t sha1Size = input.size();
    SHA1(sha1Base, sha1Size, sha1Hash);
    std::string ret((char *) sha1Hash, SHA_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::sha224(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char *) input.c_str();
    unsigned char sha2Hash[SHA224_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA224(sha2Base, sha2Size, sha2Hash);
    std::string ret((char *) sha2Hash, SHA224_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::sha256(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char *) input.c_str();
    unsigned char sha2Hash[SHA256_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA256(sha2Base, sha2Size, sha2Hash);
    std::string ret((char *) sha2Hash, SHA256_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::sha384(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char *) input.c_str();
    unsigned char sha2Hash[SHA384_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA384(sha2Base, sha2Size, sha2Hash);
    std::string ret((char *) sha2Hash, SHA384_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::sha512(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char *) input.c_str();
    unsigned char sha2Hash[SHA512_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA512(sha2Base, sha2Size, sha2Hash);
    std::string ret((char *) sha2Hash, SHA512_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::md5(const std::string &input, bool hex) {
    auto md5Base = (const unsigned char *) input.c_str();
    unsigned char md5Hash[MD5_DIGEST_LENGTH];
    size_t md5Size = input.size();
    MD5(md5Base, md5Size, md5Hash);
    std::string ret((char *) md5Hash, MD5_DIGEST_LENGTH);
    if (hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Crypto::passwordHash(const std::string &password, const Engines::HashingEngine &hashingEngine) {
    // use the provided HashingEngine for generation
    return hashingEngine.generateHash(password);
}

bool Crypto::passwordVerify(const std::string &password, const std::string &hash,
                            const Engines::HashTypeTable &hashTypeTable) {
    if (hash.empty()) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Cannot verify an empty hash");
    }

    auto verifyer = hashTypeTable.getEngine(hash);
    if (verifyer.use_count() == 0) {
        throw Exception(__PRETTY_FUNCTION__, 2,
                        "Could not determine a HashingEngine that is able to verify the given hash");
    }

    return verifyer->verifyHash(password, hash);
}
