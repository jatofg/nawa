/**
 * \file ConnectionInitContainer.h
 * \brief Container used by request handlers to initiate the nawa::Connection object.
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

#ifndef NAWA_CONNECTIONINITCONTAINER_H
#define NAWA_CONNECTIONINITCONTAINER_H

#include <nawa/config/Config.h>
#include <nawa/connection/FlushCallbackContainer.h>
#include <nawa/request/RequestInitContainer.h>

namespace nawa {
    /**
     * Internal container filled by the RequestHandler with prerequisites for creating Connection and Request objects.
     */
    struct ConnectionInitContainer {
        /**
         * Callback function which takes a `nawa::FlushCallbackContainer` and flushes the response to the user.
         */
        FlushCallbackFunction flushCallback;
        Config config;                    /**< The NAWA config. */
        RequestInitContainer requestInit; /**< The RequestInitContainer containing necessary request data. */
    };
}// namespace nawa

#endif//NAWA_CONNECTIONINITCONTAINER_H
