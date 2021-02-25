/**
 * \file GPC.cpp
 * \brief Implementation of the request::GPC class.
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

#include <nawa/request/GPC/GPC.h>
#include <nawa/request/RequestInitContainer.h>

using namespace nawa;
using namespace std;

struct request::GPC::Data {
    Source source;
    std::unordered_multimap<std::string, std::string> dataMap;

    explicit Data(Source source) : source(source) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(request, GPC)

request::GPC::GPC(const RequestInitContainer &requestInit, Source source) {
    data = make_unique<Data>(source);

    switch (source) {
        case Source::COOKIE:
            data->dataMap = requestInit.cookieVars;
            break;
        case Source::POST:
            data->dataMap = requestInit.postVars;
            break;
        default:
            data->dataMap = requestInit.getVars;
    }
}

string request::GPC::operator[](const string &gpcVar) const {
    auto e = data->dataMap.find(gpcVar);
    if (e != data->dataMap.end()) return e->second;
    else return "";
}

vector<string> request::GPC::getVector(const string &gpcVar) const {
    vector<string> ret;
    auto e = data->dataMap.equal_range(gpcVar);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

size_t request::GPC::count(const string &gpcVar) const {
    return data->dataMap.count(gpcVar);
}

unordered_multimap<string, string> const &request::GPC::getMultimap() const {
    return data->dataMap;
}

unordered_multimap<string, string>::const_iterator request::GPC::begin() const {
    return data->dataMap.begin();
}

unordered_multimap<string, string>::const_iterator request::GPC::end() const {
    return data->dataMap.end();
}

request::GPC::operator bool() const {
    return !data->dataMap.empty();
}