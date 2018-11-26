//
// Created by tobias on 23/11/18.
//

#include "Session.h"
#include "Utils.h"
#include <random>
#include <openssl/sha.h>

Qsf::Session::Session(Qsf::Connection &connection) : connection(connection) {
    // do not initialize session yet -> extra function
    // receive from connection.request or generate session key
    // check for or generate corresponding object?
    // multiple threads can access ONE SINGLE session at once! -> locking on the values!
    // lock while reading and writing
}

std::string Qsf::Session::generateID() {
    std::stringstream base;

    // Add 2 ints from random_device (should be in fact /dev/urandom), giving us (in general) 64 bits of entropy
    std::random_device rd;
    base << rd() << rd();

    // Add client IP
    base << connection.request.env["remoteAddress"];

    // Calculate SHA1
    // TODO create Qsf::Crypto namespace containing functions for sha1 etc.
    auto sha1BaseStr = base.str();
    auto sha1Base = (const unsigned char*) sha1BaseStr.c_str();
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    size_t sha1Size = sha1BaseStr.size();
    SHA1(sha1Base, sha1Size, sha1Hash);

    return hex_dump(std::string((char*)sha1Hash, SHA_DIGEST_LENGTH));
}
