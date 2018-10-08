//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_QSFREQUEST_H
#define QSF_QSFREQUEST_H
#define QSF_REQ_GET 1
#define QSF_REQ_POST 2
#define QSF_REQ_COOKIE 3

#include <string>
#include <fastcgi++/request.hpp>
#include "RequestHandler.h"

namespace Qsf {
    /**
     * Represents request objects.
     */
    class Request {
    public:

        /**
         * Container for POST-submitted files.
         */
        class File {
        public:
            /**
             * Constructs the object from a libfastcgipp File container.
             * @param file libfastcgipp File container
             */
            explicit File(const Fastcgipp::Http::File<char>& file);
            /**
             * Constructs the object from a data pointer.
             * @param dataPtr Pointer to the first byte of the memory area containing the file
             * @param size Size of the file in bytes
             */
            File(std::unique_ptr<char[]>& dataPtr, size_t size);
            std::string filename; /**< Original file name (submitted by sender) */
            std::string contentType; /**< Content-Type string */
            size_t size; /**< File size in bytes */
            const std::unique_ptr<char[]>& dataPtr; /**< Reference to a unique_ptr to the first byte of the memory area */
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
            bool writeFile(std::string path);
        };

        /**
         * Accessor for environment variables.
         */
        class Env {
        protected:
            RequestHandler& request;
        public:
            explicit Env(RequestHandler& request) : request(request) {}
            /**
             * Get an environment variable.
             * @param envVar Name of the environment variable.
             * @return Content of the environment variable. Empty string if not set.
             */
            std::string operator [](std::string envVar) const;
            /**
             * Receive the languages accepted by the client (from HTTP Header).
             * @return Vector of strings containing the accepted languages. Empty if not set.
             */
            std::vector<std::string> getAcceptLanguages() const;
            /**
             * Request path.
             * @return Vector of strings containing the elements of the path.
             */
            std::vector<std::string> getPathInfo() const;
            /**
             * Get the server address as a libfastcgipp Address object. Use ["serverAddress"] to access it as a string.
             * @return The Address object.
             * @deprecated
             */
            Fastcgipp::Http::Address getServerAddr() const;
            /**
             * Get the remote address as a libfastcgipp Address object. Use ["remoteAddress"] to access it as a string.
             * @return  The Address object.
             * @deprecated
             */
            Fastcgipp::Http::Address getRemoteAddr() const;
        };
        /**
         * Accessor for GET, POST, and COOKIE variables.
         */
        class GPC {
        protected:
            RequestHandler& request;
            uint source;
            std::multimap<std::basic_string<char>, std::basic_string<char>> data;
        public:
            GPC(RequestHandler& request, uint source);
            virtual ~GPC() {}
            /**
             * Get a GET, POST, or COOKIE variable. If the query contains more than one variable of the same name,
             * only one of them (usually the first definition) will be returned. For accessing all definitions,
             * please use getVector().
             * @param gpcVar Name of the variable.
             * @return Value of the variable. Empty string if not set (or empty - use count() for checking whether the variable is set).
             */
            std::string operator [](std::string gpcVar) const;
            /**
             * Get all GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Vector of values. Empty if not set.
             */
            std::vector<std::string> getVector(std::string gpcVar) const;
            /**
             * Get the number of submitted GET, POST, or COOKIE variables with the given name.
             * @param gpcVar Name of the variables.
             * @return Number of occurrences.
             */
            unsigned long count(std::string gpcVar) const;
        };
        /**
         * Specialized accessor for POST that also allows accessing files (and in future, maybe, the raw POST data).
         */
        class Post: public GPC {
        public:
            explicit Post(RequestHandler& request);
            virtual ~Post() {}
            /**
             * Get the raw POST data. Not implemented yet.
             * @return Empty string.
             * @todo Implementation.
             */
            std::string getRaw() const;
            /**
             * Get all POST files with the given name.
             * @param postVar Name of the files.
             * @return Vector of files. Empty if no file with the given name exists.
             */
            std::vector<Request::File> getFileVector(std::string postVar) const;
        };

        /* TODO cookie setting (fits better in Response, probably) */
        const Request::Env env; /**< The Env object you should use to access environment variables. */
        const Request::GPC get; /**< The GPC object you should use to access GET variables. */
        const Request::Post post; /**< The Post object you should use to access POST variables. */
        const Request::GPC cookie; /**< The GPC object you should use to access COOKIE variables. */
        Request(RequestHandler& request);
    };
}




#endif //QSF_QSFREQUEST_H
