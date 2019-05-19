/**
 * \file RequestHandler.h
 * \brief Class which connects QSF to the fastcgi/web server communication library.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SORU_REQUESTHANDLER_H
#define SORU_REQUESTHANDLER_H
#define SORU_RAWPOST_NEVER 0
#define SORU_RAWPOST_NONSTANDARD 1
#define SORU_RAWPOST_ALWAYS 2

#include <fastcgi++/request.hpp>
#include <soru/Config.h>
#include <soru/AppInit.h>

namespace soru {
    class Request;
    class Connection;

    // Types of functions that need to be accessed from QSF applications
    typedef int init_t(soru::AppInit& appInit); /**< Type for the init() function of QSF apps. */
    typedef int handleRequest_t(soru::Connection& connection); /**< Type for the handleRequest(Connection) function of QSF apps. */

    /**
     * Class which connects QSF to the fastcgi/web server communication library.
     */
    class RequestHandler : public Fastcgipp::Request<char> {
        // declare Request friend so it can access private members inherited from Fastcgipp::Request
        friend class soru::Request;
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
        void flush(soru::Connection& connection);
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
        static void setAppRequestHandler(const soru::Config &cfg, void *appOpen);
        /**
         * Take over the AppInit struct filled by the init() function of the app.
         * @param _appInit AppInit struct as filled by the app.
         */
        static void setConfig(const soru::AppInit &_appInit);
        /**
         * Reset the pointer to the AppInit to avoid a segfault on termination and clear session data.
         */
        static void destroyEverything();
        /**
         * Construct the RequestHandler object by passing the postMax (as set by setConfig(...)) to the fastcgi library.
         */
        RequestHandler();
        /**
         * Apply the filters set by the app (through AppInit), if filtering is enabled.
         * @param connection Reference to the connection object to read the request from and write the response to,
         * if the request has to be filtered.
         * @return True if the request has been filtered and a response has already been set by this function
         * (and the app should not be invoked on this request). False if the app should handle this request.
         */
        bool applyFilters(soru::Connection& connection);
    };
}

#endif //SORU_REQUESTHANDLER_H
