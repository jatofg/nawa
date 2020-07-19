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

#include <fastcgi++/request.hpp>

namespace nawa {
    class FastcgiRequestHandler : public Fastcgipp::Request<char> {
    public:
        /**
         * Run handleRequest(Connection) function of the loaded app upon a request.
         * @return Returns true to satisfy the fastcgi library.
         */
        bool response() override;

    };
}

#endif //NAWA_FASTCGIREQUESTHANDLER_H
