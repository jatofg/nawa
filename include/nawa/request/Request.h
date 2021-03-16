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

#include <nawa/internal/fwdecl.h>
#include <nawa/internal/macros.h>
#include <nawa/request/Env.h>
#include <nawa/request/GPC/GPC.h>
#include <nawa/request/GPC/ext/Post.h>
#include <string>

namespace nawa {
    /**
     * Represents request objects.
     */
    class Request {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Request);

        /**
         * Initialize a Request object from a RequestInitContainer.
         * @param initContainer The RequestInitContainer with data needed to create a Request object.
         */
        explicit Request(RequestInitContainer const& initContainer);

        /**
         * The Env object you should use to access environment variables.
         * @return Reference to the object.
         */
        [[nodiscard]] request::Env const& env() const noexcept;

        /**
         * The GPC object you should use to access the request's GET variables.
         * @return Reference to the object.
         */
        [[nodiscard]] request::GPC const& get() const noexcept;

        /**
         * The Post object you should use to access the request's POST variables.
         * @return Reference to the object.
         */
        [[nodiscard]] request::Post const& post() const noexcept;

        /**
         * The GPC object you should use to access the request's COOKIE variables.
         * @return Reference to the object.
         */
        [[nodiscard]] request::GPC const& cookie() const noexcept;
    };
}// namespace nawa

#endif//NAWA_NAWAREQUEST_H
