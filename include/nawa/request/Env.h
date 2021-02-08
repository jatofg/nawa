/**
 * \file Env.h
 * \brief Accessor class for environment variables.
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

#ifndef NAWA_ENV_H
#define NAWA_ENV_H

#include <nawa/request/RequestInitContainer.h>
#include <string>
#include <vector>

namespace nawa::request {
    /**
     * Accessor for environment variables.
     */
    class Env {
    public:
        explicit Env(const RequestInitContainer &initContainer);

        /**
         * Get an environment variable. For a list of environment variables, see \ref environmentmanual
         * @param envVar Name of the environment variable.
         * @return Content of the environment variable. Empty string if not set.
         */
        std::string operator[](const std::string &envVar) const;

        /**
         * Receive the languages accepted by the client (from HTTP Header). Works with fastcgi request handler only.
         * @return Vector of strings containing the accepted languages. Empty if not set.
         * @deprecated Please use ["accept-language"] instead to access the header directly.
         */
        [[nodiscard]] std::vector<std::string> getAcceptLanguages() const;

        /**
         * Request path. Use ["REQUEST_URI"] to access it as a string.
         * @return Vector of strings containing the elements of the path.
         */
        [[nodiscard]] std::vector<std::string> getRequestPath() const;

    private:
        std::unordered_map<std::string, std::string> environment;
        std::vector<std::string> acceptLanguages;
    };
}

#endif //NAWA_ENV_H
