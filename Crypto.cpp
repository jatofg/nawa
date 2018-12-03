//
// Created by tobias on 03/12/18.
//

#include "Crypto.h"
#include "Utils.h"
#include <openssl/sha.h>

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
