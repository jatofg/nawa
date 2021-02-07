/**
 * \file BlockFilter.h
 * \brief Structure defining a request blocking filter.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#ifndef NAWA_BLOCKFILTER_H
#define NAWA_BLOCKFILTER_H

#include <nawa/filter/AccessFilter/AccessFilter.h>

namespace nawa {
    /**
     * Defines a request blocking filter.
     */
    struct BlockFilter : public AccessFilter {
        /**
         * The HTTP status that will be sent to the client if the request is blocked.
         */
        unsigned int status = 404;
    };
}

#endif //NAWA_BLOCKFILTER_H
