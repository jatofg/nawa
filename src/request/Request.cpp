/**
 * \file Request.cpp
 * \brief Implementation of the Request class.
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

#include <nawa/request/Request.h>
#include <nawa/request/RequestInitContainer.h>

using namespace nawa;
using namespace std;

struct Request::Data {
    request::Env const env;
    request::GPC const get;
    request::Post const post;
    request::GPC const cookie;

    explicit Data(RequestInitContainer const& initContainer) : env(initContainer),
                                                               get(initContainer, request::GPC::Source::GET),
                                                               post(initContainer),
                                                               cookie(initContainer, request::GPC::Source::COOKIE) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Request)

Request::Request(RequestInitContainer const& initContainer) {
    data = make_unique<Data>(initContainer);
}

request::Env const& nawa::Request::env() const noexcept {
    return data->env;
}

request::GPC const& nawa::Request::get() const noexcept {
    return data->get;
}

request::Post const& nawa::Request::post() const noexcept {
    return data->post;
}

request::GPC const& nawa::Request::cookie() const noexcept {
    return data->cookie;
}
