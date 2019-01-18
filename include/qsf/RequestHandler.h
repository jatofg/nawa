/**
 * \file RequestHandler.h
 * \brief Class which connects QSF to the fastcgi/web server communication library.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_REQUESTHANDLER_H
#define QSF_REQUESTHANDLER_H
#define QSF_RAWPOST_NEVER 0
#define QSF_RAWPOST_NONSTANDARD 1
#define QSF_RAWPOST_ALWAYS 2

#include <fastcgi++/request.hpp>
#include "Config.h"

namespace Qsf {
    class Request;
    class Connection;

    // Types of functions that need to be accessed from QSF applications
    typedef int init_t(); /**< Type for the init() function of QSF apps. */
    typedef int handleRequest_t(Qsf::Connection& connection); /**< Type for the handleRequest(Connection) function of QSF apps. */

    /**
     * Class which connects QSF to the fastcgi/web server communication library.
     */
    class RequestHandler : public Fastcgipp::Request<char> {
        // declare Request friend so it can access private members inherited from Fastcgipp::Request
        friend class Qsf::Request;
        static size_t postMax; /**< Maximum post size, in bytes, read from the config by setConfig(...). */
        /**
         * Integer value referring to the raw post access level, as described by the QSF_RAWPOST_* macros.
         */
        static uint rawPostAccess;
        /**
         * The config read from the config file will be stored here statically. The Config object will be copied
         * upon each request into a non-static member of Connection, so it can be modified at runtime.
         */
        static Qsf::Config config;
        std::string postContentType; /**< Content type submitted by the browser in the request, set by inProcessor() */
        std::string rawPost; /**< Raw POST request, set by inProcessor() if requested. */
    public:
        /**
         * Run handleRequest(Connection) function of the loaded app upon a request.
         * @return Returns true to satisfy the fastcgi library.
         */
        bool response() override;
        /**
         * Flush response to the browser. This function will be invoked by Connection::flushResponse().
         * @param connection Reference to the Connection object the response will be read from.
         */
        void flush(Qsf::Connection& connection);
        /**
         * Function that decides what happens to POST data if there is any.
         * @return Always returns false so that the fastcgi library will still create the POST map.
         */
        bool inProcessor() override;
        /**
         * Take over the config and dlopen handle to the app library file from main.
         * @param cfg Reference to the Config object representing the QSF config file(s).
         * @param appOpen dlopen handle which will be used to load the app handleRequest(...) function.
         */
        static void setConfig(const Qsf::Config& cfg, void* appOpen);
        /**
         * Construct the RequestHandler object by passing the postMax (as set by setConfig(...)) to the fastcgi library.
         */
        RequestHandler();
    };
}

#endif //QSF_REQUESTHANDLER_H
