/**
 * \file Cookie.h
 * \brief Structure for creating Cookie objects. Content can be set by using a constructor.
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
        NAWA_PRIVATE_IMPL_DEF()
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
         * Set content of the cookie.
         * @param content Content of the cookie.
         * @return This Cookie object.
         */
        Cookie &setContent(std::string content) noexcept;

        /**
         * Get content of the cookie.
         * @return Content of the cookie.
         */
        [[nodiscard]] std::string getContent() const noexcept;

        /**
         * Set (optional) expiry date as time_t structure. Session cookies usually do not have an expiry date, meaning
         * that they are deleted when the browser is closed.
         * @param expires Expiry date as time_t structure.
         * @return This Cookie object.
         */
        Cookie &setExpires(std::time_t expires) noexcept;

        /**
         * Get (optional) expiry date.
         * @return Optional expiry date.
         */
        [[nodiscard]] std::optional<std::time_t> getExpires() const noexcept;

        /**
         * Set (optional) Max-Age attribute.
         * @param maxAge Optional Max-Age attribute.
         * @return This Cookie object.
         */
        Cookie &setMaxAge(std::optional<unsigned long> maxAge) noexcept;

        /**
         * Get (optional) Max-Age attribute.
         * @return Optional Max-Age attribute.
         */
        [[nodiscard]] std::optional<unsigned long> getMaxAge() const noexcept;

        /**
         * Set (optional) Domain attribute.
         * @param domain Optional Domain attribute.
         * @return This Cookie object.
         */
        Cookie &setDomain(std::optional<std::string> domain) noexcept;

        /**
         * Get (optional) Domain attribute.
         * @return Optional Domain attribute.
         */
        [[nodiscard]] std::optional<std::string> getDomain() const noexcept;

        /**
         * Set (optional) Path attribute.
         * @param path Optional Path attribute.
         * @return This Cookie object.
         */
        Cookie &setPath(std::optional<std::string> path) noexcept;

        /**
         * Get (optional) Path attribute.
         * @return Optional Path attribute.
         */
        [[nodiscard]] std::optional<std::string> getPath() const noexcept;

        /**
         * Set Secure attribute.
         * @param secure Secure attribute.
         * @return This Cookie object.
         */
        Cookie &setSecure(bool secure) noexcept;

        /**
         * Get Secure attribute.
         * @return Secure attribute.
         */
        [[nodiscard]] bool getSecure() const noexcept;

        /**
         * Set HttpOnly attribute.
         * @param httpOnly HttpOnly attribute.
         * @return This Cookie object.
         */
        Cookie &setHttpOnly(bool httpOnly) noexcept;

        /**
         * Get HttpOnly attribute.
         * @return HttpOnly attribute.
         */
        [[nodiscard]] bool getHttpOnly() const noexcept;

        /**
         * Set SameSite attribute.
         * @param sameSite SameSite attribute.
         * @return This Cookie object.
         */
        Cookie &setSameSite(SameSite sameSite) noexcept;

        /**
         * Get SameSite attribute.
         * @return SameSite attribute.
         * @return This Cookie object.
         */
        [[nodiscard]] SameSite getSameSite() const noexcept;
    };
}

#endif //NAWA_COOKIE_H
