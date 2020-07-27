/**
 * \file FastcgiRequestHandler.h
 * \brief Class which connects NAWA to the fastcgi++ library.
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

#ifndef NAWA_FASTCGIREQUESTHANDLER_H
#define NAWA_FASTCGIREQUESTHANDLER_H

#include <nawa/RequestHandlers/RequestHandler.h>

namespace nawa {
    class FastcgiRequestHandler : public RequestHandler {
        struct FastcgippManagerAdapter;
        std::unique_ptr<FastcgippManagerAdapter> fastcgippManager;
    public:
        /**
         * Construct a FastcgiRequestHandler object. May throw a UserException on failure.
         * @param handleRequestFunction The handleRequest function of the app.
         * @param config The config.
         * @param concurrency Concurrency level (number of worker threads).
         */
        FastcgiRequestHandler(HandleRequestFunction handleRequestFunction, Config config, int concurrency);

        // explicit destructor with implementation in source file is needed to destruct the unique_ptr
        ~FastcgiRequestHandler() override;

        void start() override;

        void stop() override;

        void terminate() override;

        void join() override;
    };
}

#endif //NAWA_FASTCGIREQUESTHANDLER_H
