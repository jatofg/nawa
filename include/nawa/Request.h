/**
 * \file Request.h
 * \brief Class which represents request objects.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#ifndef NAWA_NAWAREQUEST_H
#define NAWA_NAWAREQUEST_H

#include <string>
#include <nawa/Config.h>
#include <nawa/File.h>

namespace nawa {
    using RawPostCallbackFunction = std::function<std::string()>;
    using FileVectorCallbackFunction = std::function<std::vector<File>(const std::string &)>;

    /**
     * Internal container filled by the RequestHandler with prerequisites for creating Connection and Request objects.
     */
    struct RequestInitContainer {
        /**
         * Environment variables, see \ref environmentmanual
         */
        std::unordered_map<std::string, std::string> environment; // TODO use a <std::any> vector instead?
        /**
         * Vector containing languages accepted by the browser.
         */
        std::vector<std::string> acceptLanguages; // TODO improve this, should be part of environment
        std::multimap<std::string, std::string> getVars; /**< The HTTP GET vars. */
        /**
         * The HTTP POST vars, only if it is in standard format (content type `multipart/form-data` or
         * `application/x-www-form-urlencoded`). Files are excluded and handled separately by fileVectorCallback.
         */
        std::multimap<std::string, std::string> postVars;
        std::multimap<std::string, std::string> cookieVars; /**< The HTTP COOKIE vars. */
        std::string postContentType; /**< The HTTP POST content type. */
        /**
         * A function which returns a std::string containing the raw POST data. Raw data does not have to be available
         * when the config option {"post", "raw_access"} is set to "never", or when it's set to "nonstandard" and the
         * POST content type is neither `multipart/form-data` nor `application/x-www-form-urlencoded`.
         */
        RawPostCallbackFunction rawPostCallback; // TODO maybe everything should just be provided as it is and split here?
        /**
         * A function which takes the POST key as argument and returns a std::vector containing files submitted via POST
         * with the given key (as nawa::File objects).
         */
        FileVectorCallbackFunction fileVectorCallback;
    };

    /**
     * Represents request objects.
     */
    class Request {
    public:
        /**
         * Accessor for environment variables.
         */
        class Env {
        public:
            explicit Env(const RequestInitContainer &initContainer);

            /**
             * Get an environment variable. For a list of environment variables, see \ref environmentmanual
             * @param envVar Name of the environment variable.
             * @return Content of the environment variable. Empty string if not set.
             */
            std::string operator[](const std::string &envVar) const;

            /**
             * Receive the languages accepted by the client (from HTTP Header).
             * @return Vector of strings containing the accepted languages. Empty if not set.
             */
            std::vector<std::string> getAcceptLanguages() const;

            /**
             * Request path. Use ["requestUri"] to access it as a string.
             * @return Vector of strings containing the elements of the path.
             */
            std::vector<std::string> getRequestPath() const;

        private:
            std::unordered_map<std::string, std::string> environment; // TODO use a <std::any> vector instead?
            std::vector<std::string> acceptLanguages; // TODO improve this, should be part of environment
        };

        /**
         * Accessor for GET, POST, and COOKIE variables.
         */
        class GPC {
        public:
            enum class Source {
                GET,
                POST,
                COOKIE
            };
        protected:
            Source source;
            std::multimap<std::basic_string<char>, std::basic_string<char>> dataMap;
        public:
            GPC(const RequestInitContainer &requestInit, Source source);

            virtual ~GPC() = default;

            /**
             * Get a GET, POST, or COOKIE variable. If the query contains more than one variable of the same name,
             * only one of them (usually the first definition) will be returned. For accessing all definitions,
             * please use getVector(). Complexity is logarithmic, so if you want to access a value multiple times,
             * saving it as a variable is a good idea.
             * @param gpcVar Name of the variable.
             * @return Value of the variable. Empty string if not set
             * (or empty - use count() for checking whether the variable is set).
             */
            std::string operator[](const std::string &gpcVar) const;

            /**
             * Get all GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Vector of values. Empty if not set.
             */
            [[nodiscard]] std::vector<std::string> getVector(const std::string &gpcVar) const;

            /**
             * Get the number of submitted GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Number of occurrences.
             */
            [[nodiscard]] unsigned long count(const std::string &gpcVar) const;

            /**
             * Get a reference to the GET, POST, or COOKIE multimap.
             * @return Reference to the multimap.
             */
            std::multimap<std::string, std::string> &getMultimap();

            /**
             * Get constant begin iterator to the multimap containing all GET, POST, or COOKIE data.
             * @return Iterator to the first element of the multimap.
             */
            [[nodiscard]] std::multimap<std::string, std::string>::const_iterator begin() const;

            /**
             * Get constant end iterator to the multimap containing all GET, POST, or COOKIE data.
             * @return Iterator to the end of the multimap.
             */
            [[nodiscard]] std::multimap<std::string, std::string>::const_iterator end() const;
        };

        /**
         * Specialized accessor for POST that also allows accessing files (and in future, maybe, the raw POST data).
         */
        class Post : public GPC {
            std::string contentType;
            RawPostCallbackFunction rawPostCallback;
            FileVectorCallbackFunction fileVectorCallback;
        public:
            explicit Post(const RequestInitContainer &requestInit);

            ~Post() override = default;

            /**
             * Get the raw POST data (availability may depend on the raw_access setting in the config).
             * @return Reference to a string containing the raw POST data if available, otherwise the string is empty.
             */
            [[nodiscard]] std::string getRaw() const;

            /**
             * Get the POST content type as submitted by the browser
             * @return String containing the POST content type
             */
            [[nodiscard]] std::string getContentType() const;

            /**
             * Get all POST files with the given name.
             * @param postVar Name of the files.
             * @return Vector of files. Empty if no file with the given name exists.
             */
            [[nodiscard]] std::vector<File> getFileVector(const std::string &postVar) const;
        };

        const Request::Env env; /**< The Env object you should use to access environment variables. */
        const Request::GPC get; /**< The GPC object you should use to access GET variables. */
        const Request::Post post; /**< The Post object you should use to access POST variables. */
        const Request::GPC cookie; /**< The GPC object you should use to access COOKIE variables. */
        explicit Request(const RequestInitContainer &initContainer);
    };
}


#endif //NAWA_NAWAREQUEST_H
