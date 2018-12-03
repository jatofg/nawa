//
// Created by tobias on 03/12/18.
//

#include "Crypto.h"
#include "Utils.h"
#include <openssl/sha.h>
#include <openssl/md5.h>

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
