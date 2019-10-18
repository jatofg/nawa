/**
 * \file BcryptHashingEngine.h
 * \brief Hashing engine for password hashing using bcrypt.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#ifndef NAWA_BCRYPTHASHINGENGINE_H
#define NAWA_BCRYPTHASHINGENGINE_H

#include <nawa/Engines/HashingEngine.h>

namespace nawa {
    namespace Engines {

        /**
         * Hashing engine for password hashing using bcrypt. This engine class should not be used directly, but only
         * through the corresponding Nawa::Crypto functions for password hashing.
         */
        class BcryptHashingEngine: public HashingEngine {
            /**
             * The bcrypt work factor.
             */
            int workFactor;
            std::string salt;
        public:
            /**
             * Create a new generator object and set the work factor.
             * @param workFactor The work factor determines how much effort is needed in order to generate the salt.
             * A higher work factor increases security of the hash, but it will also slow down the hash generation.\n
             * The work factor must be between (including) 4 and 31, if it is not, it will default to 12.
             * @param salt The salt to use. Please leave this empty (default), a good random salt will be used then.
             * If you are setting the salt, please note that at most BCRYPT_HASHSIZE characters of the given string
             * will be used, the rest will be discarded.
             */
            explicit BcryptHashingEngine(int workFactor = 10, std::string salt = "");
            /**
             * Generate a hash of the input string with the given salt or, if empty, with a random one based on the
             * work factor.
             *
             * This function might throw an exception with error code 10 (salt generation failed), or 11 (hash
             * generation failed).
             * @param input The input string to hash.
             * @return A bcrypt hash (60 characters) in the standard format, including WF and salt ("$2a$...").
             */
            std::string generateHash(std::string input) const override;
            /**
             * Check if the given bcrypt hash in standard format (e.g., "$2a$..."), is a hash of the input string.
             * This function is designed in a way that it should not be vulnerable to timing attacks.
             * @param input The input string.
             * @param hash The hash to verify.
             * @return True if it matches, false otherwise.
             */
            bool verifyHash(std::string input, std::string hash) const override;
        };

    }
}

#endif //NAWA_BCRYPTHASHINGENGINE_H
