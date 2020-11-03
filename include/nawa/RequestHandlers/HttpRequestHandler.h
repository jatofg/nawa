/**
 * \file HttpRequestHandler.h
 * \brief A request handler which creates a development web server.
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

#ifndef NAWA_HTTPREQUESTHANDLER_H
#define NAWA_HTTPREQUESTHANDLER_H

#include <nawa/RequestHandlers/RequestHandler.h>

namespace nawa {
    class HttpRequestHandler : public RequestHandler {
        struct HttpHandlerAdapter;
        std::unique_ptr<HttpHandlerAdapter> httpHandler;
    public:
        /**
         * Construct a FastcgiRequestHandler object. May throw a nawa::Exception on failure.
         * @param handleRequestFunction The handleRequest function of the app.
         * @param config_ The config.
         * @param concurrency Concurrency level (number of worker threads).
         */
        HttpRequestHandler(HandleRequestFunction handleRequestFunction, Config config_, int concurrency);

        // explicit destructor with implementation in source file is needed to destruct the unique_ptr
        ~HttpRequestHandler() override;

        void start() override;

        void stop() noexcept override;

        void terminate() noexcept override;

        void join() noexcept override;
    };
}

#endif //NAWA_HTTPREQUESTHANDLER_H
