/**
 * \file Connection.h
 * \brief Response object to be passed back to NAWA and accessor to the request.
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

#ifndef NAWA_RESPONSE_H
#define NAWA_RESPONSE_H

#include <nawa/config/Config.h>
#include <nawa/connection/Cookie.h>
#include <nawa/internal/fwdecl.h>
#include <nawa/internal/macros.h>
#include <nawa/request/Request.h>
#include <nawa/session/Session.h>
#include <sstream>
#include <string>
#include <unordered_map>

namespace nawa {
    /**
     * Response object to be passed back to NAWA and accessor to the request.
     */
    class Connection {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Connection);

        /**
         * The Request object representing the current request.
         * @return Reference to the Request object.
         */
        nawa::Request const &request() const noexcept;

        /**
         * The Session object for accessing the current session.
         * @return Reference to the Session object.
         */
        nawa::Session &session() noexcept;

        /**
         * The Session object for accessing the current session.
         * @return Reference to the Session object.
         */
        [[nodiscard]] nawa::Session const &session() const noexcept;

        /**
         * Access the NAWA configuration. This is a copy of the Config object that contains the values of the config file
         * which was read at the startup of NAWA. You can use the Config::set method to change values at runtime, however,
         * these changes only affect the current connection.
         * @return Reference to the Config object.
         */
        nawa::Config &config() noexcept;

        /**
         * Access the NAWA configuration. This is a copy of the Config object that contains the values of the config file
         * which was read at the startup of NAWA.
         * @return Reference to the Config object.
         */
        [[nodiscard]] nawa::Config const &config() const noexcept;

        /**
         * Stream which allows you to write stuff to the HTTP body comfortably.
         * @return Reference to the response ostream.
         */
        std::ostream &responseStream() noexcept;

        /**
         * Create a Connection object.
         * @param connectionInit The ConnectionInitContainer object containing the necessary parameters.
         */
        explicit Connection(ConnectionInitContainer const &connectionInit);

        /**
         * Set the HTTP response body (everything that comes after the headers). This will overwrite everything
         * that was set previously. You can use responseStream() instead to get a stream to write to.
         * @param content Complete content of the HTTP body.
         */
        void setResponseBody(std::string content);

        /**
         * Set the HTTP status code. It will be passed to the web server without checking for validity. For known
         * status codes, the textual description will be appended.
         * @param status The HTTP status code to pass to the web server.
         */
        void setStatus(unsigned int status);

        /**
         * Set an HTTP header or overwrite all existing ones with the same key (keys are case-insensitive and will be
         * converted to lowercase). Please note that the content-type header will be automatically set to
         * "text/html; charset=utf-8", but can be overwritten, of course. There will be no checking or validation,
         * please make sure that the content of the header is valid (value valid for key, no newlines, ...)
         * @param key Key of the HTTP header (case-insensitive).
         * @param value Value of the HTTP header.
         */
        void setHeader(std::string key, std::string value);

        /**
         * Add an HTTP header with the given key (case-insensitive, see notes for setHeader). If headers with the
         * given key have already been set, they will not be overwritten.
         * @param key Key of the HTTP header (case-insensitive).
         * @param value Value of the HTTP header.
         */
        void addHeader(std::string key, std::string value);

        /**
         * Unset all HTTP headers with the specified key if it exists (otherwise do nothing). Please note that you can
         * only unset headers that were previously set in NAWA (including content-type), not those that are set by,
         * for example, the web server.
         * @param key Key of the HTTP header(s) (case-insensitive).
         */
        void unsetHeader(std::string key);

        /**
         * Set a new HTTP cookie or overwrite the cookie with the given key. Create a Cookie object first, setting at
         * least the content of the cookie. This function may throw a nawa::Exception with error code 1 if the key or
         * cookie content contain illicit characters. Works only before the response has been flushed.
         * For more information about cookie setting and handling, see \ref environmentmanual
         * @param key Key of the cookie. Valid characters in the key (as regex): [A-Za-z0-9!#$%&'*+\-.^_`|~]
         * @param cookie Cookie object containing the value and options of the cookie.
         * Valid characters in the cookie content (as regex): [A-Za-z0-9!#$%&'()*+\-.\/:<=>?@[\]^_`{|}~]
         */
        void setCookie(const std::string &key, Cookie cookie);

        /**
         * Set a new HTTP cookie or overwrite the cookie with the given key. This function will create a Cookie 
         * object with default attributes and the given content. It may throw a nawa::Exception with error code 1 if
         * the key or cookie content contain illicit characters. Works only before the response has been flushed.
         * For more information about cookie setting and handling, see \ref environmentmanual
         * @param key Key of the cookie. Valid characters in the key (as regex): [A-Za-z0-9!#$%&'*+\-.^_`|~]
         * @param cookieContent Cookie object containing the value and options of the cookie.
         */
        void setCookie(const std::string &key, std::string cookieContent);

        /**
         * Unset an HTTP cookie that was previously set using setCookie(). Will just do nothing if no cookie with the
         * given key exists. Won't remove a cookie from the user's browser, just undoes the `setCookie` operation.
         * @param key Key of the cookie.
         */
        void unsetCookie(const std::string &key);

        /**
         * This method can be used to set default attributes for cookies. All attributes which are not set or enabled
         * in a cookie will be taken from the policy cookie, if set there. The content attribute will be ignored.
         * @param policy Cookie object containing default attributes for cookies.
         */
        void setCookiePolicy(Cookie policy);

        /**
         * Send a file from disk to the client. This will automatically set the content-type, content-length, and
         * last-modified headers and replace the existing HTTP response body (if any) with the contents of the file.
         * You are responsible to check request headers (such as accepts and if-modified-since). If the file cannot
         * be read, a nawa::Exception with error code 1 will be thrown.
         * @param path Path to the file, including the file name of course (better use absolute paths).
         * @param contentType The content-type string (such as image/png). If left empty, NAWA will try to guess the
         * content type itself (this will only work for a few common file types), and use "application/octet-stream"
         * if that fails. Content type guessing is done solely based on the file extension.
         * @param forceDownload Ask the browser to download the file by sending "content-disposition: attachment".
         * @param downloadFilename Preferred filename for saving the file on the disk of the client. If this parameter
         * is set, and forceDownload is set to false, a "content-disposition: inline" header will be sent.
         * @param checkIfModifiedSince Check the if-modified-since header, if sent by the client, and compare it to
         * the modification date of the file. Prepare a not-modified response and clear the body if the file has not
         * been modified. Using this parameter only makes sense if the client requested exactly this file, of course.
         */
        void sendFile(const std::string &path, const std::string &contentType = "", bool forceDownload = false,
                      const std::string &downloadFilename = "", bool checkIfModifiedSince = false);

        /**
         * Get the response body.
         * @return The response body.
         */
        std::string getResponseBody();

        /**
         * Get the HTTP response status.
         * @return The HTTP response status.
         */
        [[nodiscard]] unsigned int getStatus() const;

        /**
         * Get a map of all response headers.
         * @param includeCookies Also include set-cookie headers for the cookies.
         * @return A map of all headers.
         */
        [[nodiscard]] std::unordered_multimap<std::string, std::string> getHeaders(bool includeCookies = true) const;

        /**
         * Flush the Response object, i.e., send headers and body to the client and reset it.
         * Please note that you cannot set cookies and headers anymore after flushing.
         * Attempts to do so will be silently ignored.\n
         * NOTE: Flushing might not work as expected with Apache 2.4 and mod_proxy_fcgi
         */
        void flushResponse();
    };
}

#endif //NAWA_RESPONSE_H
