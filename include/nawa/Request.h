/**
 * \file Request.h
 * \brief Class which represents request objects.
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

#ifndef NAWA_NAWAREQUEST_H
#define NAWA_NAWAREQUEST_H

#include <string>
#include <fastcgi++/request.hpp>
#include <nawa/RequestHandler.h>

namespace nawa {
    /**
     * Represents request objects.
     */
    class Request {
    public:

        /**
         * Container for POST-submitted files. This class cannot currently be used for storing files not provided
         * by libfastcgipp, however, this is scheduled to change with v1.
         */
        class File {
        public:
            /**
             * Constructs the object from a libfastcgipp File container.
             * @param file libfastcgipp File container
             */
            explicit File(const Fastcgipp::Http::File<char>& file);
            std::string filename; /**< Original file name (submitted by sender) */
            std::string contentType; /**< Content-Type string */
            size_t size; /**< File size in bytes */
            const std::unique_ptr<char[]>& dataPtrRef; /**< Reference to a unique_ptr to the first byte of the memory area */
            /**
             * Copy the file into a std::string
             * @return std::string containing the whole file
             */
            std::string copyFile();
            /**
             * Write the file to disk.
             * @param path File name and path where to write the file.
             * @return true on success, false on failure
             */
            bool writeFile(const std::string& path);
        };

        /**
         * Accessor for environment variables.
         */
        class Env {
        protected:
            RequestHandler& requestHandler;
        public:
            explicit Env(RequestHandler& request) : requestHandler(request) {}
            /**
             * Get an environment variable.
             * @param envVar Name of the environment variable.
             * @return Content of the environment variable. Empty string if not set.
             */
            std::string operator [](const std::string& envVar) const;
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
            /**
             * Get the server address as a libfastcgipp Address object. Use ["serverAddress"] to access it as a string.
             * @return The Address object.
             * @deprecated
             */
            Fastcgipp::Address getServerAddr() const;
            /**
             * Get the remote address as a libfastcgipp Address object. Use ["remoteAddress"] to access it as a string.
             * @return  The Address object.
             * @deprecated
             */
            Fastcgipp::Address getRemoteAddr() const;

            // mark Connection as a friend so it can access RequestHandler through Env
            // TODO find another solution as this looks a bit like a dirty hack?
            friend Connection;
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
            RequestHandler& requestHandler;
            Source source;
            std::multimap<std::basic_string<char>, std::basic_string<char>> data;
        public:
            GPC(RequestHandler& request, Source source);
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
            std::string operator [](const std::string& gpcVar) const;
            /**
             * Get all GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Vector of values. Empty if not set.
             */
            std::vector<std::string> getVector(const std::string& gpcVar) const;
            /**
             * Get the number of submitted GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Number of occurrences.
             */
            unsigned long count(const std::string& gpcVar) const;
            /**
             * Get a reference to the GET, POST, or COOKIE multimap.
             * @return Reference to the multimap.
             */
            std::multimap<std::string, std::string>& getMultimap();
            /**
             * Get constant begin iterator to the multimap containing all GET, POST, or COOKIE data.
             * @return Iterator to the first element of the multimap.
             */
            std::multimap<std::string, std::string>::const_iterator begin() const;
            /**
             * Get constant end iterator to the multimap containing all GET, POST, or COOKIE data.
             * @return Iterator to the end of the multimap.
             */
            std::multimap<std::string, std::string>::const_iterator end() const;
        };
        /**
         * Specialized accessor for POST that also allows accessing files (and in future, maybe, the raw POST data).
         */
        class Post: public GPC {
        public:
            explicit Post(RequestHandler& request);
            ~Post() override = default;
            /**
             * Get the raw POST data (availability depends on the raw_access setting in the config).
             * @return Reference to a string containing the raw POST data if available, otherwise the string is empty.
             */
            std::string& getRaw() const;
            /**
             * Get the POST content type as submitted by the browser
             * @return String containing the POST content type
             */
            std::string getContentType() const;
            /**
             * Get all POST files with the given name.
             * @param postVar Name of the files.
             * @return Vector of files. Empty if no file with the given name exists.
             */
            std::vector<Request::File> getFileVector(const std::string& postVar) const;
        };

        const Request::Env env; /**< The Env object you should use to access environment variables. */
        const Request::GPC get; /**< The GPC object you should use to access GET variables. */
        const Request::Post post; /**< The Post object you should use to access POST variables. */
        const Request::GPC cookie; /**< The GPC object you should use to access COOKIE variables. */
        explicit Request(RequestHandler& request);
    };
}




#endif //NAWA_NAWAREQUEST_H