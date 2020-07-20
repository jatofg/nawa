/**
 * \file RequestHandlerLegacy.h
 * \brief Class which connects NAWA to the fastcgi/web server communication library.
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

#ifndef NAWA_REQUESTHANDLERLEGACY_H
#define NAWA_REQUESTHANDLERLEGACY_H

#include <fastcgi++/request.hpp>
#include <nawa/Config.h>
#include <nawa/AppInit.h>

namespace nawa {
    class Request;
    class Connection;

    // Types of functions that need to be accessed from NAWA applications
    typedef int init_t(nawa::AppInit& appInit); /**< Type for the init() function of NAWA apps. */
    typedef int handleRequest_t(nawa::Connection& connection); /**< Type for the handleRequest(Connection) function of NAWA apps. */

    /**
     * Class which connects NAWA to the fastcgi/web server communication library.
     */
    class RequestHandlerLegacy : public Fastcgipp::Request<char> {
        // declare Request friend so it can access private members inherited from Fastcgipp::Request
        friend class nawa::Request;
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
        void flush(nawa::Connection& connection);
        /**
         * Function that decides what happens to POST data if there is any.
         * @return Always returns false so that the fastcgi library will still create the POST map.
         */
        bool inProcessor() override;
        /**
         * Take over the config and dlopen handle to the app library file from main.
         * @param cfg Reference to the Config object representing the NAWA config file(s).
         * @param appOpen dlopen handle which will be used to load the app handleRequest(...) function.
         */
        static void setAppRequestHandler(const nawa::Config &cfg, void *appOpen);
        /**
         * Take over the AppInit struct filled by the init() function of the app.
         * @param _appInit AppInit struct as filled by the app.
         */
        static void setConfig(const nawa::AppInit &_appInit);
        /**
         * Reset the pointer to the AppInit to avoid a segfault on termination and clear session data.
         */
        static void destroyEverything();
        /**
         * Construct the RequestHandler object by passing the postMax (as set by setConfig(...)) to the fastcgi library.
         */
        RequestHandlerLegacy();
    };
}

#endif //NAWA_REQUESTHANDLERLEGACY_H
