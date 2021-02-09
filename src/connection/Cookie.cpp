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

struct Cookie::Impl {
    // rework cookie handling (defaults, ...)
    string content; /**< Content of the cookie. */
    optional<time_t> expires = 0; /**< Expiry date as time_t structure. 0 for no explicit expiry (session cookie). */
    optional<unsigned long> maxAge = 0; /**< Set Max-Age attribute. */
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
    impl->content = move(c);
}

Cookie &Cookie::setContent(string content) noexcept {
    impl->content = move(content);
    return *this;
}

string Cookie::getContent() const noexcept {
    return impl->content;
}

Cookie &Cookie::setExpires(time_t expires) noexcept {
    impl->expires = expires;
    return *this;
}

optional<time_t> Cookie::getExpires() const noexcept {
    return impl->expires;
}

Cookie &Cookie::setMaxAge(optional<unsigned long> maxAge) noexcept {
    impl->maxAge = maxAge;
    return *this;
}

optional<unsigned long> Cookie::getMaxAge() const noexcept {
    return impl->maxAge;
}

Cookie &Cookie::setDomain(optional<string> domain) noexcept {
    impl->domain = move(domain);
    return *this;
}

optional<string> Cookie::getDomain() const noexcept {
    return impl->domain;
}

Cookie &Cookie::setPath(optional<string> path) noexcept {
    impl->path = move(path);
    return *this;
}

optional<string> Cookie::getPath() const noexcept {
    return impl->path;
}

Cookie &Cookie::setSecure(bool secure) noexcept {
    impl->secure = secure;
    return *this;
}

bool Cookie::getSecure() const noexcept {
    return impl->secure;
}

Cookie &Cookie::setHttpOnly(bool httpOnly) noexcept {
    impl->httpOnly = httpOnly;
    return *this;
}

bool Cookie::getHttpOnly() const noexcept {
    return impl->httpOnly;
}

Cookie &Cookie::setSameSite(Cookie::SameSite sameSite) noexcept {
    impl->sameSite = sameSite;
    return *this;
}

Cookie::SameSite Cookie::getSameSite() const noexcept {
    return impl->sameSite;
}
