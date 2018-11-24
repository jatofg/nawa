//
// Created by tobias on 23/11/18.
//

#include "Session.h"
#include <random>
#include <openssl/sha.h>

Qsf::Session::Session(Qsf::Connection &connection) : connection(connection) {
    // do not initialize session yet -> extra function
    // receive from connection.env.request or generate session key
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
    auto sha1BaseStr = base.str();
    auto sha1Base = (const unsigned char*) sha1BaseStr.c_str();
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    size_t sha1Size = sha1BaseStr.size()-1;
    SHA1(sha1Base, sha1Size, sha1Hash);

    return std::string((char*)sha1Hash, SHA_DIGEST_LENGTH);
}
