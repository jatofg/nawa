/**
 * \file AuthFilter.h
 * \brief Structure defining an HTTP Basic Authentication filter.
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

#ifndef NAWA_AUTHFILTER_H
#define NAWA_AUTHFILTER_H

#include <nawa/filter/AccessFilter/AccessFilter.h>

namespace nawa {
    /**
     * Defines a filter that will request HTTP Basic Authentication if matching. After successful authentication,
     * forward filters will still be checked, and if no forward filter matches, the request can be processed by the app
     * normally. If authentication fails, a 403 page with the defined response or a standard error page will be sent.\n
     * Please note that this will not work out of the box with Apache2 and mod_proxy_fcgi, as Apache does not forward
     * the WWW-Authenticate header to the application. You can fix this by setting "CGIPassAuth On" for all request
     * URIs that need this feature in the Apache2 vhost config, e.g.:
     * \code{.unparsed}
     * <LocationMatch "^/test">
     *     CGIPassAuth On
     * </LocationMatch>
     * \endcode
     */
    class AuthFilter : public AccessFilter {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_OVERRIDE_DEF(AuthFilter);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(AuthFilter);

        NAWA_COPY_CONSTRUCTOR_DEF(AuthFilter);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(AuthFilter);

        NAWA_MOVE_CONSTRUCTOR_DEF(AuthFilter);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(AuthFilter);

        /**
         * The authentication function. It will be called with the provided user name as first parameter and the
         * provided user password as the second parameter. Access will be granted if the authentication function
         * returns true. If the function is not set, authorization will always be denied.
         * @return Reference to element.
         */
        std::function<bool(std::string, std::string)> &authFunction() noexcept;

        /**
         * The authentication function. It will be called with the provided user name as first parameter and the
         * provided user password as the second parameter. Access will be granted if the authentication function
         * returns true. If the function is not set, authorization will always be denied.
         * @return Reference to element.
         */
        [[nodiscard]] std::function<bool(std::string, std::string)> const &authFunction() const noexcept;

        /**
         * A short description of the required authentication that may be shown by the browser ("realm") (optional).
         * This value should only contain alphanumeric characters and must not contain double quotes or newlines.
         * It will not be checked for validity by NAWA, instead, authentication may fail and the server might even become
         * unaccessible in case of non-compliance. If this value comes from a user, make sure to check it thoroughly
         * (by the way, it shouldn't).
         * @return Reference to element.
         */
        std::string &authName() noexcept;

        /**
         * A short description of the required authentication that may be shown by the browser ("realm") (optional).
         * This value should only contain alphanumeric characters and must not contain double quotes or newlines.
         * It will not be checked for validity by NAWA, instead, authentication may fail and the server might even become
         * unaccessible in case of non-compliance. If this value comes from a user, make sure to check it thoroughly
         * (by the way, it shouldn't).
         * @return Reference to element.
         */
        [[nodiscard]] std::string const &authName() const noexcept;

        /**
         * Use sessions to remember the authenticated user. This will create a std::string session variable
         * "_nawa_authfilter[id]" (wherein [id] is the number of the filter), containing the user name.
         * You can use it in your application to find out which user has authenticated and delete it to log the user
         * out. This is usually not necessary (but might be more effective or extend the scope of the authorization,
         * and lets you access the user name), as browsers will usually send the authentication string on every single
         * request.
         * @return Reference to element.
         */
        bool &useSessions() noexcept;

        /**
         * Use sessions to remember the authenticated user. This will create a std::string session variable
         * "_nawa_authfilter[id]" (wherein [id] is the number of the filter), containing the user name.
         * You can use it in your application to find out which user has authenticated and delete it to log the user
         * out. This is usually not necessary (but might be more effective or extend the scope of the authorization,
         * and lets you access the user name), as browsers will usually send the authentication string on every single
         * request.
         * @return The element.
         */
        [[nodiscard]] bool useSessions() const noexcept;
    };
}

#endif //NAWA_AUTHFILTER_H
