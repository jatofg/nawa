/**
 * \file RequestHandler.h
 * \brief Handles and serves incoming requests via the NAWA app.
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

#ifndef NAWA_REQUESTHANDLER_H
#define NAWA_REQUESTHANDLER_H

#include <nawa/Config.h>
#include <nawa/AppInit.h>

namespace nawa {
    // forward declarations
    class Request;
    class Connection;

    // Types of functions that need to be accessed from NAWA applications
    // TODO does not belong here, move it to somewhere else
    using init_t = int(AppInit&); /**< Type for the init() function of NAWA apps. */
    using handleRequest_t = int(Connection&); /**< Type for the handleRequest(Connection) function of NAWA apps. */

    using HandleRequestFunction = std::function<int(nawa::Connection&)>;

    class RequestHandler {
        HandleRequestFunction handleRequestFunction;
    protected:
        Config config;
        // TODO if a segfault happens during shutdown, use a unique_ptr and destroy manually
        AppInit appInit;
    public:
        /**
         * Set the handleRequest function of the app.
         * @param handleRequestFunction The request handling function of the app.
         */
        void setAppRequestHandler(HandleRequestFunction handleRequestFunction);
        /**
         * Take over the AppInit struct filled by the init() function of the app.
         * @param _appInit AppInit struct as filled by the app.
         */
        void setAppInit(AppInit appInit);
        /**
         * Set the config.
         * @param config The config.
         */
        void setConfig(Config config);
        /**
         * Clear session data
         */
        static void destroyEverything(); // TODO still necessary to do this in RequestHandler? find a better place!
        /**
         * Flush response to the browser. This function will be invoked by Connection::flushResponse().
         * @param connection Reference to the Connection object the response will be read from.
         */
        //virtual void flush(nawa::Connection& connection) = 0; // TODO should instead be handled by a callback
        /**
         * Start request handling.
         */
        virtual void start() = 0;
        /**
         * Stop request handling.
         */
        virtual void stop() = 0;
        /**
         * Enforce termination of request handling.
         */
        virtual void terminate() = 0;
        /**
         * Block until request handling has shut down.
         */
        virtual void join() = 0;
        /**
         * Handle request by processing the filters and calling the app's handleRequest function, if necessary
         * @param connection The current Connection object.
         */
        void handleRequest(Connection& connection);
    private:
        /**
         * Apply the filters set by the app (through AppInit), if filtering is enabled.
         * @param connection Reference to the connection object to read the request from and write the response to,
         * if the request has to be filtered.
         * @return True if the request has been filtered and a response has already been set by this function
         * (and the app should not be invoked on this request). False if the app should handle this request.
         */
        bool applyFilters(Connection &connection);
    };
}

#endif //NAWA_REQUESTHANDLER_H
