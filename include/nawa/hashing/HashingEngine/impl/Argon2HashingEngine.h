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
 * \file BcryptHashingEngine.h
 * \brief Hashing engine for password hashing using Argon2.
 */

#ifndef NAWA_ARGON2HASHINGENGINE_H
#define NAWA_ARGON2HASHINGENGINE_H

#include <nawa/hashing/HashingEngine/HashingEngine.h>
#include <nawa/internal/macros.h>

namespace nawa::hashing {

    class Argon2HashingEngine : public HashingEngine {
    private:
        NAWA_PRIVATE_DATA()

    public:
        /**
         * The Argon2 flavor to use.
         */
        enum class Algorithm {
            ARGON2I,
            ARGON2D,
            ARGON2ID
        };

        NAWA_DEFAULT_DESTRUCTOR_DEF(Argon2HashingEngine);

        /**
         * Create a new Argon2 hash generator and set the parameters.
         * @param algorithm The Argon2 flavor to use.
         * @param timeCost Number of iterations.
         * @param memoryCost Memory usage in kiB.
         * @param parallelism Number of threads used.
         * @param salt User-defined salt. Please leave this empty, a good salt will be generated automatically.
         * @param hashLen Desired hash lenght, 32 by default.
         */
        explicit Argon2HashingEngine(Algorithm algorithm = Algorithm::ARGON2ID, uint32_t timeCost = 2,
                                     uint32_t memoryCost = (1 << 16), uint32_t parallelism = 1, std::string salt = "",
                                     size_t hashLen = 32);

        /**
         * Generate a hash of the input string with the given salt or, if empty, with a random one, and taking into
         * account the properties.
         *
         * This function might throw an exception with error code 10 (hash invalid), or 11 (argon2 error).
         * @param input The input string to hash.
         * @return An Argon2 hash in standard format (starting with $argon2id$, for example, and in base64 format).
         * The hash itself is, by default, 32 bytes (64 hash characters) long.
         */
        [[nodiscard]] std::string generateHash(std::string input) const override;

        /**
         * Verify if the given Argon2 hash, given in encoded format ("$argon2..."), has been generated using the
         * input string. This function will create a new engine object according to the properties of the encoded
         * string internally.
         * It is designed in a way that it should not be vulnerable to timing attacks.
         * @param input The input string.
         * @param hash The hash to verify.
         * @return True if it matches, false otherwise.
         */
        [[nodiscard]] bool verifyHash(std::string input, std::string hash) const override;
    };

}// namespace nawa::hashing

#endif//NAWA_ARGON2HASHINGENGINE_H
