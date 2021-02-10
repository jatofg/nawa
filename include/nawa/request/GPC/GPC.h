/**
 * \file GPC.h
 * \brief Accessor class for GET, POST, and COOKIE variables.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#ifndef NAWA_GPC_H
#define NAWA_GPC_H

#include <nawa/internal/macros.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace nawa {
    class RequestInitContainer;
}

namespace nawa::request {
    /**
     * Accessor for GET, POST, and COOKIE variables.
     */
    class GPC {
        NAWA_PRIVATE_IMPL_DEF()

    public:
        enum class Source {
            GET,
            POST,
            COOKIE
        };

        NAWA_DEFAULT_DESTRUCTOR_DEF(GPC);

        GPC(const RequestInitContainer &requestInit, Source source);

        /**
         * Get a GET, POST, or COOKIE variable. If the query contains more than one variable of the same name,
         * only one of them (usually the first definition) will be returned. For accessing all definitions,
         * please use getVector(). Complexity is logarithmic, so if you want to access a value multiple times,
         * saving it in a variable is a good idea.
         * @param gpcVar Name of the variable.
         * @return Value of the variable. Empty string if not set
         * (or empty - use count() for checking whether the variable is set).
         */
        std::string operator[](const std::string &gpcVar) const;

        /**
         * Get all GET, POST, or COOKIE variables with the given name.
         * @param gpcVar Name of the variables.
         * @return Vector of values. Empty if not set.
         */
        [[nodiscard]] std::vector<std::string> getVector(const std::string &gpcVar) const;

        /**
         * Get the number of submitted GET, POST, or COOKIE variables with the given name.
         * @param gpcVar Name of the variables.
         * @return Number of occurrences.
         */
        [[nodiscard]] size_t count(const std::string &gpcVar) const;

        /**
         * Get a reference to the GET, POST, or COOKIE multimap.
         * @return Reference to the multimap.
         */
        [[nodiscard]] std::unordered_multimap<std::string, std::string> const &getMultimap() const;

        /**
         * Get constant begin iterator to the multimap containing all GET, POST, or COOKIE data.
         * @return Iterator to the first element of the multimap.
         */
        [[nodiscard]] std::unordered_multimap<std::string, std::string>::const_iterator begin() const;

        /**
         * Get constant end iterator to the multimap containing all GET, POST, or COOKIE data.
         * @return Iterator to the end of the multimap.
         */
        [[nodiscard]] std::unordered_multimap<std::string, std::string>::const_iterator end() const;

        /**
         * Shortcut to check for the existence of GET/POST/COOKIE values (including files in the case of POST).
         * @return True if GET/POST/COOKIE values are available.
         */
        explicit virtual operator bool() const;
    };
}

#endif //NAWA_GPC_H
