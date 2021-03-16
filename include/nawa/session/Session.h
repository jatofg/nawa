/**
 * \file Session.h
 * \brief Class for managing sessions and getting and setting connection-independent session data.
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

#ifndef NAWA_SESSION_H
#define NAWA_SESSION_H

#include <any>
#include <memory>
#include <nawa/connection/Cookie.h>
#include <nawa/internal/fwdecl.h>
#include <nawa/internal/macros.h>

namespace nawa {
    /**
     * Class for managing sessions and getting and setting connection-independent session data.
     */
    class Session {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Session);

        /**
         * Construct a new Session object. This will just store the Connection reference in the object.
         * @param connection Reference to the current Connection (for getting and setting cookies).
         */
        explicit Session(Connection& connection);

        /**
         * Start the session (load existing session basing on a cookie sent by the client or create a new one).
         * This will send a session cookie to the client. The properties/attributes of the session cookie are determined
         * by, with decreasing precedence,\n
         *
         * 1. the Cookie object that may be passed as an optional parameter to this function and will be used as a
         *    template for the cookie. See the param description on how to use it.
         * 2. the configuration in the NAWA configuration file,
         * 3. and, of course, by the cookie policy that can be set via Connection::setCookiePolicy.
         *
         * The duration (keep-alive) of the session is defined in the NAWA config file, but can be overridden by setting
         * the attribute maxAge of the parameter object, see below.
         *
         * **IMPORTANT!** This function will NOT work correctly after flushing the response (as setting a session cookie
         * is impossible then). It is recommended to call start() directly in the beginning of your program.
         *
         * @param properties Template for the cookie sent to the client. You can use it to influence the behavior of
         * Session and to make sure the cookie is properly secured. The attributes will be used as follows:
         * - content: will be ignored (and replaced by the session ID)
         * - expires: set this to > 0 (e.g., to 1), if the Expires and Max-Age attributes should be set for the cookie.
         *   The value will be replaced by the proper expiry time. If 0, attribute inclusion can still be forced by the
         *   NAWA configuration or Connection::setCookiePolicy. Please note that if using setCookiePolicy, the attributes
         *   will be added after Session has set the cookie and the contents will thus be determined by the policy, not
         *   by the session. This may lead to unwanted behavior, so please make sure that you set this attribute to > 0
         *   here if you are using setCookiePolicy.
         * - maxAge: will be used as the session duration (inactive keep-alive, server-side!) if > 0. If expires == 0
         *   (and is also not overridden by the NAWA config), this attribute will be reset to 0 before setting the cookie.
         * - secure: send the Secure attribute with the cookie.
         * - httpOnly: send the HttpOnly attribute with the cookie.
         * - sameSite: set the SameSite attribute to lax (if sameSite == 1) or strict (if sameSite > 1).
         */
        void start(nawa::Cookie properties = Cookie());

        /**
         * Check whether a session is currently active (has been started).
         * @return True if session is established, false otherwise.
         */
        [[nodiscard]] bool established() const;

        /**
         * Check whether there exists a stored value for the given key. Please note that the behavior of this
         * function might differ from `[key].has_value()` - while this function will also return true if the key has
         * been set to an empty std::any, the latter one only returns true if the std::any contains an object.
         * @param key Key to check.
         * @return True if a value exists for this key, false otherwise. Always false if no session is active.
         */
        [[nodiscard]] bool isSet(std::string const& key) const;

        /**
         * Get the value at the given key (as a std::any object). To actually receive the stored object, use
         * `std::any_cast` (example: `std::any_cast<std::string>(conn.session["test"])`). You will have to
         * explicitly state the type of the stored object as a template argument of `std::any_cast`
         * (as C++ is statically typed).
         * @param key Key to get value for.
         * @return Value at key. If no value exists for that key or no session established, a std::any without value
         * will be returned.
         */
        std::any operator[](std::string const& key) const;

        /**
         * Set key to a value of type std::any. Throws a nawa::Exception with error code 1 if no session has been
         * established.
         * @param key Key to set.
         * @param value Value to set the key to.
         */
        void set(std::string key, const std::any& value);

        /**
         * Set key to a string value. This function exists for convenience and makes sure that you do not save a
         * const char* (c-style string) into a session (a terrible idea, as such a pointer would not be available
         * anymore on the next request and therefore cause a segmentation fault). For std::string values, the next
         * specialization (arbitrary type) will be used. The c-style string will be wrapped into a std::string, which
         * will be wrapped into a Any value. As this function internally calls
         * set(std::string, Any), a nawa::Exception with error code 1 will be thrown if no session has been
         * established.
         * @param key Key to set.
         * @param value C-string that will be used as the value (will be stored as a std::string!).
         */
        void set(std::string key, char const* value) {
            set(std::move(key), std::make_any<std::string>(value));
        }

        /**
         * Set key to a variable of an arbitrary type. This function exists just for convenience and will create a
         * new std::any from the value type and call set(std::string, std::any), and will therefore throw a
         * nawa::Exception with error code 1 if no session has been established. As you need to explicitly state the
         * type when receiving the value later on, explicitly constructing the desired type might make your code
         * more readable and less error-prone (if the value is directly constructed and not given as a variable).
         * @tparam T Type of the value. Can usually be deducted automatically by the compiler.
         * @param key Key to set.
         * @param value Value to set the key to.
         */
        template<typename T>
        void set(std::string key, T const& value) {
            set(std::move(key), std::any(value));
        }

        /**
         * Remove the session variable with the given key. Throws a nawa::Exception with error code 1 if no session has
         * been established.
         * @param key Key to remove.
         */
        void unset(std::string const& key);

        /**
         * Terminate and delete the currently existing session along with its data and dequeue the session cookie.
         * This function will do nothing if no session is currently active. After invalidating the current session,
         * you may start a new one, as long as the response has not already been flushed (as in this case, the cookie
         * header has already been sent).
         */
        void invalidate();
    };
}// namespace nawa

#endif//NAWA_SESSION_H
