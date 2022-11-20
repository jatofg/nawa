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
 * \file Env.cpp
 * \brief Implementation of the request::Env class.
 */

#include <nawa/request/Env.h>
#include <nawa/request/RequestInitContainer.h>
#include <nawa/util/utils.h>
#include <unordered_map>

using namespace nawa;
using namespace std;

struct request::Env::Data {
    unordered_map<string, string> environment;

    explicit Data(RequestInitContainer const& initContainer) : environment(initContainer.environment) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(request, Env)

request::Env::Env(RequestInitContainer const& initContainer) {
    data = make_unique<Data>(initContainer);
}

std::string request::Env::operator[](std::string const& envVar) const {
    if (data->environment.count(envVar)) {
        return data->environment.at(envVar);
    }
    return {};
}

std::vector<std::string> request::Env::getRequestPath() const {
    return utils::splitPath(operator[]("REQUEST_URI"));
}
