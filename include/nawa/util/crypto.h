/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file crypto.h
 * \brief A bunch of useful cryptographic functions (esp. hashing), acting as a wrapper to C crypto libraries.
 */

#ifndef NAWA_CRYPTO_H
#define NAWA_CRYPTO_H

#include <nawa/hashing/HashTypeTable/HashTypeTable.h>
#include <nawa/hashing/HashTypeTable/impl/DefaultHashTypeTable.h>
#include <nawa/hashing/HashingEngine/HashingEngine.h>
#include <nawa/hashing/HashingEngine/impl/BcryptHashingEngine.h>
#include <string>

namespace nawa::crypto {
    /**
     * Get SHA-1 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation (40 chars). Otherwise, the hash will
     * be binary (20 bytes).
     * @return SHA-1 hash of input.
     */
    std::string sha1(std::string const& input, bool hex = true);

    /**
     * Get SHA-224 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
     * be binary (28 bytes).
     * @return Hash of input.
     */
    std::string sha224(std::string const& input, bool hex = true);

    /**
     * Get SHA-256 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
     * be binary (32 bytes).
     * @return Hash of input.
     */
    std::string sha256(std::string const& input, bool hex = true);

    /**
     * Get SHA-384 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
     * be binary (48 bytes).
     * @return Hash of input.
     */
    std::string sha384(std::string const& input, bool hex = true);

    /**
     * Get SHA-512 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
     * be binary (64 bytes).
     * @return Hash of input.
     */
    std::string sha512(std::string const& input, bool hex = true);

    /**
     * Get MD5 hash of a string.
     * @param input String to hash.
     * @param hex If true (default), get hash in hexadecimal representation (32 chars). Otherwise, the hash will
     * be binary (16 bytes).
     * @return Hash of input.
     */
    std::string md5(std::string const& input, bool hex = true);

    /**
     * Create a (hopefully) secure password hash using a hash algorithm (bcrypt by default). \n
     * This function returns one-way hashes with pseudo-random salts. Use passwordVerify to validate a password.\n
     * May throw a nawa::Exception on failure, the error codes are defined by the underlying hashing engine, but
     * should not be lower than 10.
     * Argon2 hashing might be added later, probably in a separate function.
     * @param password The password to hash.
     * @param hashingEngine The hashing engine (containing the hashing algorithm) to use. This will (currently)
     * default to bcrypt. You can pass an object to a hashing engine if you want to use another algorithm or in case
     * you wish to customize hashing properties (such as using your own salt or changing the work factor).
     * @return Hash of the password (in case of bcrypt: 60 characters).
     */
    std::string passwordHash(std::string const& password,
                             hashing::HashingEngine const& hashingEngine = hashing::BcryptHashingEngine());

    /**
     * Validate a password with a matching hashing engine (determined by a HashTypeTable). The underlying function
     * of the hashing engine should be designed in a way that prevents timing attacks (given that the compiler did
     * no bad optimizations).\n
     * May throw a nawa::Exception on failure. Error codes: 1 (empty input hash), 2 (no hashing engine able to
     * verify the given hash could be determined by the HashTypeTable).
     * @param password Password (user input) to be verified.
     * @param hash Hash (e.g., from a database) to verify the user password against.
     * @param hashTypeTable Object providing a function that is able to determine a hashing engine that can verify
     * the given hash. The default HashTypeTable shipped with NAWA recognizes all hashing engines that also ship
     * with NAWA.
     * @return True if the password matches, false otherwise.
     */
    bool passwordVerify(std::string const& password, std::string const& hash,
                        hashing::HashTypeTable const& hashTypeTable = hashing::DefaultHashTypeTable());

}// namespace nawa::crypto

#endif//NAWA_CRYPTO_H
