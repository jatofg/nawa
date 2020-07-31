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
    class Connection;

    using HandleRequestFunction = std::function<int(nawa::Connection &)>;

    class RequestHandler {
        HandleRequestFunction handleRequestFunction;
    protected:
        Config config;
        AppInit appInit;
    public:
        virtual ~RequestHandler() = default;

        /**
         * Get a request handler object according to the config. May throw a UserException on failure (passed on
         * from the constructor of the specific request handler.
         * @param handleRequestFunction The handleRequest function of the app.
         * @param config The config.
         * @param concurrency Concurrency level (number of worker threads).
         * @return A unique_ptr to the request handler.
         */
        static std::unique_ptr<RequestHandler>
        getRequestHandler(HandleRequestFunction handleRequestFunction, Config config, int concurrency);

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
        void handleRequest(Connection &connection);
    };
}

#endif //NAWA_REQUESTHANDLER_H
