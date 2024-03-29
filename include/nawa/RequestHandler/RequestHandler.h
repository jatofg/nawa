/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file RequestHandler.h
 * \brief Handles and serves incoming requests via the NAWA app.
 */

#ifndef NAWA_REQUESTHANDLER_H
#define NAWA_REQUESTHANDLER_H

#include <memory>
#include <nawa/RequestHandler/HandleRequestFunction.h>
#include <nawa/config/Config.h>
#include <nawa/filter/AccessFilterList.h>
#include <nawa/internal/fwdecl.h>
#include <nawa/internal/macros.h>
#include <optional>

namespace nawa {
    class RequestHandler {
        NAWA_PRIVATE_DATA()

    protected:
        NAWA_DEFAULT_CONSTRUCTOR_DEF(RequestHandler);

    public:
        /**
         * The overridden virtual destructor must terminate request handling and join worker threads, if not yet done.
         * It also removes all stored session data to avoid a segfault during shutdown.
         */
        virtual ~RequestHandler();

        /**
         * Create a new request handler object according to the config. May throw a nawa::Exception on failure (passed on
         * from the constructor of the specific request handler). The constructor of a request handler should set up
         * everything that's necessary already here (as the constructor is the only function of a request handler
         * which is executed with full privileges).
         * @param handleRequestFunction A HandleRequestFunctionWrapper containing the handleRequest function of the app.
         * @param config The config.
         * @param concurrency Concurrency level (number of worker threads).
         * @return A unique_ptr to the request handler.
         */
        static std::unique_ptr<RequestHandler>
        newRequestHandler(std::shared_ptr<HandleRequestFunctionWrapper> const& handleRequestFunction, Config config,
                          int concurrency);

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
         * Set or replace the handleRequest function of the app (thread-safe, blocking).
         * @param handleRequestFunction The request handling function of the app.
         * @deprecated Might be made private and non-blocking in v0.8 (or later), use reconfigure() instead.
         */
        void setAppRequestHandler(std::shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction) noexcept;

        /**
         * Set or replace access filters for static request filtering (thread-safe, blocking).
         * @param accessFilters The access filters.
         */
        void setAccessFilters(AccessFilterList accessFilters) noexcept;

        /**
         * Get a pointer for reading the config.
         * @return Pointer to the config.
         */
        [[nodiscard]] std::shared_ptr<Config const> getConfig() const noexcept;

        /**
         * Set or replace the config (thread-safe, blocking).
         * @param config The config.
         * @deprecated Might be made private and non-blocking in v0.8 (or later), use reconfigure() instead.
         */
        void setConfig(Config config) noexcept;

        /**
         * Reconfigure the request handler (thread-safe, blocking).
         * @param handleRequestFunction A HandleRequestFunctionWrapper containing the handleRequest function of the app.
         * @param accessFilters The access filters.
         * @param config The config.
         */
        void reconfigure(std::optional<std::shared_ptr<HandleRequestFunctionWrapper>> handleRequestFunction,
                         std::optional<AccessFilterList> accessFilters, std::optional<Config> config) noexcept;

        /**
         * Reconfigure the request handler (thread-safe, blocking).
         * @param handleRequestFunction The request handling function of the app.
         * @param accessFilters The access filters.
         * @param config The config.
         */
        void reconfigure(HandleRequestFunction handleRequestFunction, std::optional<AccessFilterList> accessFilters,
                         std::optional<Config> config) noexcept;

        /**
         * Start request handling. Must not block and return immediately if request handling is already running.
         * May throw a nawa::Exception on failure during startup, but ideally, all actions which could lead to
         * errors (or preliminary checks to avoid errors) should be done in the constructor, to avoid unnecessary
         * initialization steps. Behavior of calling this function after join() has already been called is undefined,
         * ideally, it should throw a nawa::Exception (proposed error code: 10). Please not that request handlers
         * currently do NOT have to support restarting (i.e., calling stop, then start). The default FastCGI and HTTP
         * request handlers do not support this and will not start request handling once again after stopping.
         */
        virtual void start() = 0;

        /**
         * Stop request handling after current requests have been served. Must not block and return immediately after
         * the shutdown has been initiated. Must do nothing if request handling has already stopped or even joined.
         */
        virtual void stop() noexcept = 0;

        /**
         * Enforce termination of request handling. Must not block and return immediately after the termination of
         * request handling has been initiated (nevertheless, the termination should only take a few milliseconds
         * after this function has been called). Must do nothing if request handling has already stopped or even
         * joined.
         */
        virtual void terminate() noexcept = 0;

        /**
         * Restart request handling. Can optionally be supported by request handlers (the default request handlers for
         * FastCGI and HTTP currently do NOT support this). If unsupported, this function does not have to be
         * implemented, and will just do nothing.
         */
        virtual void restart() noexcept {}

        /**
         * Block until request handling has terminated. This is the only function that should block. If join has
         * already been called, this function must return immediately, without throwing exceptions. By joining,
         * the RequestHandler object becomes defunct and cannot be reused afterwards.
         */
        virtual void join() noexcept = 0;

        /**
         * Handle request by processing the filters and calling the app's handleRequest function, if necessary.
         * Internal function which should only be used by request handlers.
         * @param connection The current Connection object.
         */
        void handleRequest(Connection& connection);
    };
}// namespace nawa

#endif//NAWA_REQUESTHANDLER_H
