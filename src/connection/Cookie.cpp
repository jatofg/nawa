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
 * \file Cookie.cpp
 * \brief Implementation of the Cookie class.
 */

#include <nawa/connection/Cookie.h>

using namespace nawa;
using namespace std;

struct Cookie::Data {
    string content;                    /**< Content of the cookie. */
    optional<time_t> expires;          /**< Expiry date as time_t structure. 0 for no explicit expiry (session cookie). */
    optional<unsigned long> maxAge;    /**< Set Max-Age attribute. */
    optional<string> domain;           /**< Set Domain attribute. */
    optional<string> path;             /**< Set Path attribute. */
    bool secure = false;               /**< Set Secure attribute. */
    bool httpOnly = false;             /**< Set HttpOnly attribute. */
    SameSite sameSite = SameSite::OFF; /**< Set SameSite attribute. 1 for lax, >1 for strict. */
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Cookie)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(Cookie)

NAWA_COPY_CONSTRUCTOR_IMPL(Cookie)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Cookie)

NAWA_MOVE_CONSTRUCTOR_IMPL(Cookie)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Cookie)

Cookie::Cookie(std::string c) : Cookie() {
    data->content = std::move(c);
}

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Cookie, content, string)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Cookie, expires, optional<time_t>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Cookie, maxAge, optional<unsigned long>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Cookie, domain, optional<string>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Cookie, path, optional<string>)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(Cookie, secure, bool)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(Cookie, httpOnly, bool)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(Cookie, sameSite, Cookie::SameSite)
