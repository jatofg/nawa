/**
 * \file Request.h
 * \brief Class which represents request objects.
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

#ifndef NAWA_NAWAREQUEST_H
#define NAWA_NAWAREQUEST_H

#include <nawa/request/Env.h>
#include <nawa/request/GPC/GPC.h>
#include <nawa/request/GPC/ext/Post.h>
#include <string>

namespace nawa {
    class RequestInitContainer;

    /**
     * Represents request objects.
     */
    class Request {
    public:
        const request::Env env; /**< The Env object you should use to access environment variables. */
        const request::GPC get; /**< The GPC object you should use to access GET variables. */
        const request::Post post; /**< The Post object you should use to access POST variables. */
        const request::GPC cookie; /**< The GPC object you should use to access COOKIE variables. */
        explicit Request(const RequestInitContainer &initContainer);
    };
}

#endif //NAWA_NAWAREQUEST_H
