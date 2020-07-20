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
        class FastcgippRequestAdapter;
        struct FastcgippManagerAdapter;
        std::unique_ptr<FastcgippRequestAdapter> fastcgippRequest;
        std::unique_ptr<FastcgippManagerAdapter> fastcgippManager;
    public:
        FastcgiRequestHandler(HandleRequestFunction handleRequestFunction, Config config, int concurrency);

    };
}

#endif //NAWA_FASTCGIREQUESTHANDLER_H
