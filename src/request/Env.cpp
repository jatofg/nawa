/**
 * \file Env.cpp
 * \brief Implementation of the request::Env class.
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

#include <nawa/request/Env.h>
#include <nawa/util/Utils.h>

using namespace nawa;
using namespace std;

string request::Env::operator[](const string &envVar) const {
    if (environment.count(envVar)) {
        return environment.at(envVar);
    }
    return string();
}

vector<string> request::Env::getAcceptLanguages() const {
    return acceptLanguages;
}

vector<string> request::Env::getRequestPath() const {
    return split_path(operator[]("REQUEST_URI"));
}

request::Env::Env(const RequestInitContainer &initContainer) : environment(initContainer.environment),
                                                               acceptLanguages(initContainer.acceptLanguages) {}