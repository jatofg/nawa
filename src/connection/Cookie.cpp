/**
 * \file Cookie.cpp
 * \brief Implementation of the Cookie class.
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

#include <nawa/connection/Cookie.h>

using namespace nawa;
using namespace std;

struct Cookie::Data {
    string content; /**< Content of the cookie. */
    optional<time_t> expires; /**< Expiry date as time_t structure. 0 for no explicit expiry (session cookie). */
    optional<unsigned long> maxAge; /**< Set Max-Age attribute. */
    optional<string> domain; /**< Set Domain attribute. */
    optional<string> path; /**< Set Path attribute. */
    bool secure = false; /**< Set Secure attribute. */
    bool httpOnly = false; /**< Set HttpOnly attribute. */
    SameSite sameSite = SameSite::OFF;  /**< Set SameSite attribute. 1 for lax, >1 for strict. */
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Cookie)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(Cookie)

NAWA_COPY_CONSTRUCTOR_IMPL(Cookie)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Cookie)

NAWA_MOVE_CONSTRUCTOR_IMPL(Cookie)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Cookie)

Cookie::Cookie(string c) : Cookie() {
    data->content = move(c);
}

Cookie &Cookie::setContent(string content) noexcept {
    data->content = move(content);
    return *this;
}

string Cookie::getContent() const noexcept {
    return data->content;
}

Cookie &Cookie::setExpires(time_t expires) noexcept {
    data->expires = expires;
    return *this;
}

optional<time_t> Cookie::getExpires() const noexcept {
    return data->expires;
}

Cookie &Cookie::setMaxAge(optional<unsigned long> maxAge) noexcept {
    data->maxAge = maxAge;
    return *this;
}

optional<unsigned long> Cookie::getMaxAge() const noexcept {
    return data->maxAge;
}

Cookie &Cookie::setDomain(optional<string> domain) noexcept {
    data->domain = move(domain);
    return *this;
}

optional<string> Cookie::getDomain() const noexcept {
    return data->domain;
}

Cookie &Cookie::setPath(optional<string> path) noexcept {
    data->path = move(path);
    return *this;
}

optional<string> Cookie::getPath() const noexcept {
    return data->path;
}

Cookie &Cookie::setSecure(bool secure) noexcept {
    data->secure = secure;
    return *this;
}

bool Cookie::getSecure() const noexcept {
    return data->secure;
}

Cookie &Cookie::setHttpOnly(bool httpOnly) noexcept {
    data->httpOnly = httpOnly;
    return *this;
}

bool Cookie::getHttpOnly() const noexcept {
    return data->httpOnly;
}

Cookie &Cookie::setSameSite(Cookie::SameSite sameSite) noexcept {
    data->sameSite = sameSite;
    return *this;
}

Cookie::SameSite Cookie::getSameSite() const noexcept {
    return data->sameSite;
}
