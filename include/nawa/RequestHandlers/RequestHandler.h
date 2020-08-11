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
#include <nawa/AccessFilterList.h>

namespace nawa {
    // forward declarations
    class Connection;

    using HandleRequestFunction = std::function<int(nawa::Connection &)>;

    class RequestHandler {
        HandleRequestFunction handleRequestFunction;
        std::unique_ptr<AccessFilterList> accessFilters;
    protected:
        Config config;
    public:
        virtual ~RequestHandler();

        /**
         * Create a new request handler object according to the config. May throw a nawa::Exception on failure (passed on
         * from the constructor of the specific request handler). The constructor of a request handler should set up
         * everything that's necessary already here (as the constructor is the only function of a request handler
         * which is executed with full privileges).
         * @param handleRequestFunction The handleRequest function of the app.
         * @param config The config.
         * @param concurrency Concurrency level (number of worker threads).
         * @return A unique_ptr to the request handler.
         */
        static std::unique_ptr<RequestHandler>
        newRequestHandler(HandleRequestFunction handleRequestFunction, Config config, int concurrency);

        /**
         * Set the handleRequest function of the app.
         * @param handleRequestFunction The request handling function of the app.
         */
        void setAppRequestHandler(HandleRequestFunction handleRequestFunction) noexcept;

        /**
         * Set access filters for static request filtering.
         * @param accessFilterList The access filters.
         */
        void setAccessFilters(AccessFilterList accessFilterList) noexcept;

        /**
         * Set the config.
         * @param config The config.
         */
        void setConfig(Config config) noexcept;

        /**
         * Start request handling. Must not block and return immediately after request handling has started
         * (in separate threads). May throw a nawa::Exception on failure, but ideally, all actions which could lead to
         * errors (or preliminary checks to avoid errors) should be done in the constructor, to avoid unnecessary
         * initialization steps.
         */
        virtual void start() = 0;

        /**
         * Stop request handling after current requests have been served. Must not block and return immediately after
         * the shutdown has been initiated.
         */
        virtual void stop() noexcept = 0;

        /**
         * Enforce termination of request handling. Must not block and return immediately after the termination of
         * request handling has been initiated (nevertheless, the termination should only take a few milliseconds
         * after this function has been called).
         */
        virtual void terminate() noexcept = 0;

        /**
         * Block until request handling has terminated. This is the only function that should block.
         */
        virtual void join() noexcept = 0;

        /**
         * Handle request by processing the filters and calling the app's handleRequest function, if necessary.
         * @param connection The current Connection object.
         */
        void handleRequest(Connection &connection);
    };
}

#endif //NAWA_REQUESTHANDLER_H
