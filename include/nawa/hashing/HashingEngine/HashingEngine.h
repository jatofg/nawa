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
 * \file HashingEngine.h
 * \brief Abstract base class for hashing engines.
 */

#ifndef NAWA_HASHINGENGINE_H
#define NAWA_HASHINGENGINE_H

#include <string>

namespace nawa::hashing {

    /**
     * A hashing engine must implement these functions in order to be useful for password hashing. It may store
     * additional information (such as cost and salt) in its object, the user should be able to set them as
     * constructor parameters. It is expected that the salt is automatically generated, giving the user an option
     * to manually specify the salt is optional but recommended.
     */
    class HashingEngine {
    public:
        /**
         * Generate a (salted) hash basing on the input (usually the password to be hashed). Throw a
         * nawa::Exception with an error code >=10 in case of failure.
         * @param input The input string (e.g., user password).
         * @return The hash in a standard format, including the salt and all information necessary to check it
         * (e.g., "$2a$10$...").
         */
        [[nodiscard]] virtual std::string generateHash(std::string input) const = 0;

        /**
         * Check whether the input string has been used to produce the hash. This function should not need to throw
         * exceptions.
         * @param input The input string (e.g., password the user entered).
         * @param hash The stored hash containing all necessary information.
         * @return True if the input string matches the hash, false otherwise.
         */
        [[nodiscard]] virtual bool verifyHash(std::string input, std::string hash) const = 0;
    };

}// namespace nawa::hashing

#endif//NAWA_HASHINGENGINE_H
