/**
 * \file Connection.h
 * \brief Response object to be passed back to NAWA and accessor to the request.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#include <string>
#include <unordered_map>
#include <sstream>
#include <nawa/Cookie.h>
#include <nawa/Request.h>
#include <nawa/Session.h>

namespace nawa {
    /**
     * Response object to be passed back to NAWA and accessor to the request.
     */
    class Connection {
        std::string bodyString;
        std::unordered_map<std::string, std::string> headers;
        std::unordered_map<std::string, Cookie> cookies;
        bool isFlushed = false;
        Cookie cookiePolicy;
        void clearStream();
        void mergeStream();
    public:
        const nawa::Request& request; /**< Access the Request object representing the current request. */
        nawa::Session session;
        /**
         * Access the NAWA configuration. This is a copy of the Config object that contains the values of the config file
         * which was read at the startup of NAWA. You can use the Config::set method to change values at runtime, however,
         * these changes only affect the current connection.
         */
        nawa::Config config;
        std::stringstream response; /**< Stringstream that allows you to write stuff to the HTTP body comfortably. */
        /**
         * Create a Connection object.
         * @param request Reference to the request object (needed to import cookies and flush the response).
         * @param config Reference to the Config object containing the NAWA configuration.
         */
        Connection(Request& request, Config& config);
        /**
         * Set the HTTP response body (everything that comes after the headers). This will overwrite everything
         * that was set previously. You can use the Response object as an ostream instead.
         * @param content Complete content of the HTTP body.
         */
        void setBody(std::string content);
        /**
         * Send a file from disk to the client. This will automatically set the content-type, content-length, and
         * last-modified headers and replace the existing HTTP response body (if any) with the contents of the file.
         * You are responsible to check request headers (such as accepts and if-modified-since). If the file cannot
         * be read, a UserException with error code 1 will be thrown.
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
         * Set the HTTP status code. It will be passed to the web server without checking for validity. For known
         * status codes, the textual description will be appended.
         * @param status The HTTP status code to pass to the web server.
         */
        void setStatus(unsigned int status);
        /**
         * Set an HTTP header or overwrite an existing one with the same key (keys are case-insensitive and will be
         * converted to lowercase). Please note that the content-type header will be automatically set to
         * "text/html; charset=utf-8", but can be overwritten, of course. There will be no checking or validation,
         * please make sure that the content of the header is valid (value valid for key, no newlines, ...)
         * @param key Key of the HTTP header (case-insensitive).
         * @param value Value of the HTTP header.
         */
        void setHeader(std::string key, std::string value);
        /**
         * Unset the HTTP header with the specified key if it exists (otherwise do nothing). Please note that you can
         * only unset headers that were previously set in NAWA (including content-type), not those that are set by,
         * for example, the web server.
         * @param key Key of the HTTP header (case-insensitive).
         */
        void unsetHeader(std::string key);
        /**
         * Set a new HTTP cookie or overwrite the cookie with the given key. Create a Cookie object first, setting at
         * least the content of the cookie. This function may throw a UserException with error code 1 if the key or
         * cookie content contain illicit characters.
         * @param key Key of the cookie. Valid characters in the key (as regex): [A-Za-z0-9!#$%&'*+\-.^_`|~]
         * @param cookie Cookie object containing the value and options of the cookie.
         * Valid characters in the cookie content (as regex): [A-Za-z0-9!#$%&'()*+\-.\/:<=>?@[\]^_`{|}~]
         */
        void setCookie(const std::string &key, Cookie cookie);
        /**
         * Unset an HTTP cookie that was previously set using setCookie(). Will just do nothing if no cookie with the
         * given key exists.
         * @param key Key of the cookie.
         */
        void unsetCookie(const std::string &key);
        /**
         * This method can be used to set default attributes for cookies. Setting a boolean attribute to true means
         * that the corresponding attribute will be sent for all cookies, regardless of the value specified in the
         * Cookie object itself. For sameSite, the policy will be overridden if sameSite > 0 in the Cookie object.
         * Values of string attributes will be used as default values if the corresponding attributes are not
         * customized in the Cookie object and ignored otherwise. The content attribute will be ignored.
         * @param policy Cookie object containing the default attributes.
         */
        void setCookiePolicy(Cookie policy);
        /**
         * Get the raw HTTP source of the request. This function is intended to be used primarily by NAWA itself.
         * @return A string containing the raw HTTP source (containing headers, incl. cookies, and the body)
         */
        std::string getRaw();
        /**
         * Flush the Response object, i.e., send headers and body to the client and reset it.
         * Please note that you cannot set cookies and headers anymore after flushing.
         * Attempts to do so will be silently ignored.\n
         * NOTE: Flushing might not work as expected with Apache 2.4 and mod_proxy_fcgi
         */
        void flushResponse();
    };
}

// TODO add stream modifiers if needed (or remove the stream modifiers altogether and advise to use .body)
//  - it is actually impossible to implement everything, e.g. integers, floats, ... -> better let the stream handle this

#endif //NAWA_RESPONSE_H
