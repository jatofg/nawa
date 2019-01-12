//
// Created by tobias on 03/12/18.
//

#include "qsf/Crypto.h"
#include "qsf/Utils.h"
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "../libs/libbcrypt/bcrypt.h"
#include "qsf/UserException.h"

std::string Qsf::Crypto::sha1(const std::string &input, bool hex) {
    auto sha1Base = (const unsigned char*) input.c_str();
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    size_t sha1Size = input.size();
    SHA1(sha1Base, sha1Size, sha1Hash);
    std::string ret((char*) sha1Hash, SHA_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::sha224(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char*) input.c_str();
    unsigned char sha2Hash[SHA224_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA224(sha2Base, sha2Size, sha2Hash);
    std::string ret((char*) sha2Hash, SHA224_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::sha256(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char*) input.c_str();
    unsigned char sha2Hash[SHA256_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA256(sha2Base, sha2Size, sha2Hash);
    std::string ret((char*) sha2Hash, SHA256_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::sha384(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char*) input.c_str();
    unsigned char sha2Hash[SHA384_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA384(sha2Base, sha2Size, sha2Hash);
    std::string ret((char*) sha2Hash, SHA384_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::sha512(const std::string &input, bool hex) {
    auto sha2Base = (const unsigned char*) input.c_str();
    unsigned char sha2Hash[SHA512_DIGEST_LENGTH];
    size_t sha2Size = input.size();
    SHA512(sha2Base, sha2Size, sha2Hash);
    std::string ret((char*) sha2Hash, SHA512_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::md5(const std::string &input, bool hex) {
    auto md5Base = (const unsigned char*) input.c_str();
    unsigned char md5Hash[MD5_DIGEST_LENGTH];
    size_t md5Size = input.size();
    MD5(md5Base, md5Size, md5Hash);
    std::string ret((char*) md5Hash, MD5_DIGEST_LENGTH);
    if(hex) {
        return hex_dump(ret);
    }
    return ret;
}

std::string Qsf::Crypto::passwordHash(const std::string &password, int cost) {
    if(password.empty()) {
        throw UserException("Qsf::Cryto::passwordHash", 1, "Trying to hash empty password");
    }
    if(cost <= 0) {
        throw UserException("Qsf::Crypto::passwordHash", 3, "Cost factor invalid.");
    }

    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];

    // use the functions provided by libbcrypt to generate a salt and a hash
    // they will return negative integers in case of a failure, 0 on success
    if(bcrypt_gensalt(cost, salt) != 0 || bcrypt_hashpw(password.c_str(), salt, hash) != 0) {
        throw UserException("Qsf::Crypto::passwordHash", 2, "Could not hash this password (unknown bcrypt failure).");
    }

    return std::string(hash, 60);
}

bool Qsf::Crypto::passwordVerify(const std::string &password, const std::string &hash) {
    if(hash.empty()) {
        throw UserException("Qsf::Crypto::passwordVerify", 1, "Cannot verify an empty hash");
    }
    if(password.empty()) {
        return false;
    }

    // return value of bcrypt_checkpw is -1 on failure, 0 on match, and >0 if not matching
    int ret = bcrypt_checkpw(password.c_str(), hash.c_str());


    if(ret < 0) {
        throw UserException("Qsf::Crypto::passwordVerify", 2, "Could not check this password (unknown bcrypt failure");
    }
    else if(ret == 0) {
        return true;
    }

    return false;
}
