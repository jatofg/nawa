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
 * \file HttpRequestHandler.h
 * \brief A request handler which creates a development web server.
 */

#ifndef NAWA_HTTPREQUESTHANDLER_H
#define NAWA_HTTPREQUESTHANDLER_H

#include <nawa/RequestHandler/RequestHandler.h>

namespace nawa {
    class HttpRequestHandler : public RequestHandler {
        NAWA_PRIVATE_DATA()

    public:
        /**
         * Construct a FastcgiRequestHandler object. May throw a nawa::Exception on failure.
         * @param handleRequestFunction The handleRequest function of the app.
         * @param config The config.
         * @param concurrency Concurrency level (number of worker threads).
         */
        HttpRequestHandler(std::shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction, Config config,
                           int concurrency);

        // explicit destructor with implementation in source file is needed to destruct the unique_ptr
        ~HttpRequestHandler() override;

        void start() override;

        void stop() noexcept override;

        void terminate() noexcept override;

        void join() noexcept override;
    };
}// namespace nawa

#endif//NAWA_HTTPREQUESTHANDLER_H
