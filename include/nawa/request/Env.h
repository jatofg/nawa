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

#include <nawa/internal/macros.h>
#include <string>
#include <vector>

namespace nawa {
    class RequestInitContainer;
}

namespace nawa::request {
    /**
     * Accessor for environment variables.
     */
    class Env {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Env);

        /**
         * Create Env using a RequestInitContainer.
         * @param initContainer The RequestInitContainer.
         */
        explicit Env(RequestInitContainer const& initContainer);

        /**
         * Get an environment variable. For a list of environment variables, see \ref environmentmanual
         * @param envVar Name of the environment variable.
         * @return Content of the environment variable. Empty string if not set.
         */
        std::string operator[](std::string const& envVar) const;

        /**
         * Request path. Use ["REQUEST_URI"] to access it as a string.
         * @return Vector of strings containing the elements of the path.
         */
        [[nodiscard]] std::vector<std::string> getRequestPath() const;
    };
}// namespace nawa::request

#endif//NAWA_ENV_H
