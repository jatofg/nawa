/**
 * \file Cookie.h
 * \brief Structure for creating Cookie objects. Content can be set by using a constructor.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_COOKIE_H
#define QSF_COOKIE_H

#include <string>
#include <ctime>

namespace Qsf {
    /**
     * Structure for creating Cookie objects. Content can be set by using a constructor.
     */
    struct Cookie {
        /**
         * Create an empty Cookie.
         */
        Cookie() = default;
        /**
         * Create a cookie and directly set the content.
         * @param c Content of the cookie.
         */
        explicit Cookie(std::string c) {
            content = std::move(c);
        }
        std::string content; /**< Content of the cookie. */
        std::time_t expires = 0; /**< Expiry date as time_t structure. 0 for no explicit expiry (session cookie). */
        unsigned long maxAge = 0; /**< Set Max-Age attribute. */
        std::string domain; /**< Set Domain attribute. */
        std::string path; /**< Set Path attribute. */
        bool secure = false; /**< Set Secure attribute. */
        bool httpOnly = false; /**< Set HttpOnly attribute. */
        unsigned int sameSite = 0;  /**< Set SameSite attribute. 1 for lax, >1 for strict. */
    };
}

#endif //QSF_COOKIE_H
