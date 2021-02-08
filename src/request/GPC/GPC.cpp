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

using namespace nawa;
using namespace std;

request::GPC::GPC(const RequestInitContainer &requestInit, Source source_)
        : source(source_) {
    switch (source_) {
        case Source::COOKIE:
            dataMap = requestInit.cookieVars;
            break;
        case Source::POST:
            dataMap = requestInit.postVars;
            break;
        default:
            dataMap = requestInit.getVars;
    }
}

string request::GPC::operator[](const string &gpcVar) const {
    auto e = dataMap.find(gpcVar);
    if (e != dataMap.end()) return e->second;
    else return "";
}

vector<string> request::GPC::getVector(const string &gpcVar) const {
    vector<string> ret;
    auto e = dataMap.equal_range(gpcVar);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

size_t request::GPC::count(const string &gpcVar) const {
    return dataMap.count(gpcVar);
}

unordered_multimap<string, string> const &request::GPC::getMultimap() const {
    return dataMap;
}

unordered_multimap<string, string>::const_iterator request::GPC::begin() const {
    return dataMap.begin();
}

unordered_multimap<string, string>::const_iterator request::GPC::end() const {
    return dataMap.end();
}

request::GPC::operator bool() const {
    return !dataMap.empty();
}