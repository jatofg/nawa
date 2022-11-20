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
 * \file Cookie.h
 * \brief Structure for creating Cookie objects. Content can be set by using a constructor.
 */

#ifndef NAWA_COOKIE_H
#define NAWA_COOKIE_H

#include <ctime>
#include <nawa/internal/macros.h>
#include <optional>
#include <string>

namespace nawa {
    /**
     * Structure for creating Cookie objects. Content can be set by using a constructor.
     */
    class Cookie {
        NAWA_PRIVATE_DATA()
    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Cookie);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(Cookie);

        NAWA_COPY_CONSTRUCTOR_DEF(Cookie);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Cookie);

        NAWA_MOVE_CONSTRUCTOR_DEF(Cookie);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Cookie);

        enum class SameSite {
            OFF,
            LAX,
            STRICT
        };

        /**
         * Create a cookie and directly set the content.
         * @param c Content of the cookie.
         */
        explicit Cookie(std::string c);

        /**
         * Content of the cookie.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Cookie, content, std::string);

        /**
         * Set (optional) expiry date as time_t structure. Session cookies usually do not have an expiry date, meaning
         * that they are deleted when the browser is closed.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Cookie, expires, std::optional<time_t>);

        /**
         * Set (optional) Max-Age attribute.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Cookie, maxAge, std::optional<unsigned long>);

        /**
         * Set (optional) Domain attribute.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Cookie, domain, std::optional<std::string>);

        /**
         * Set (optional) Path attribute.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Cookie, path, std::optional<std::string>);

        /**
         * Set Secure attribute.
         * @return Reference to element.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(Cookie, secure, bool);

        /**
         * Set HttpOnly attribute.
         * @return Reference to element.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(Cookie, httpOnly, bool);

        /**
         * Set SameSite attribute.
         * @return Reference to element.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(Cookie, sameSite, SameSite);
    };
}// namespace nawa

#endif//NAWA_COOKIE_H
