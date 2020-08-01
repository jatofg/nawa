/**
 * \file Request.cpp
 * \brief Implementation of the Request class.
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

#include <nawa/Utils.h>
#include <nawa/Request.h>

using namespace nawa;
using namespace std;

string Request::Env::operator[](const string &envVar) const {
    if (environment.count(envVar)) {
        return environment.at(envVar);
    }
    return string();
}

vector<string> Request::Env::getAcceptLanguages() const {
    return acceptLanguages;
}

vector<string> Request::Env::getRequestPath() const {
    return split_path(operator[]("requestUri"));
}

Request::Env::Env(const RequestInitContainer &initContainer) : environment(initContainer.environment),
                                                               acceptLanguages(initContainer.acceptLanguages) {}

Request::GPC::GPC(const RequestInitContainer &requestInit, Source source_)
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

string Request::GPC::operator[](const string &gpcVar) const {
    auto e = dataMap.find(gpcVar);
    if (e != dataMap.end()) return e->second;
    else return "";
}

vector<string> Request::GPC::getVector(const string &gpcVar) const {
    vector<string> ret;
    auto e = dataMap.equal_range(gpcVar);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

unsigned long Request::GPC::count(const string &gpcVar) const {
    return dataMap.count(gpcVar);
}

multimap<string, string> &Request::GPC::getMultimap() {
    return dataMap;
}

multimap<string, string>::const_iterator Request::GPC::begin() const {
    return dataMap.begin();
}

multimap<string, string>::const_iterator Request::GPC::end() const {
    return dataMap.end();
}

Request::Request(const RequestInitContainer &initContainer)
        : env(initContainer), get(initContainer, GPC::Source::GET), post(initContainer),
          cookie(initContainer, GPC::Source::COOKIE) {}

Request::Post::Post(const RequestInitContainer &requestInit) : GPC(requestInit, GPC::Source::POST),
                                                               contentType(requestInit.postContentType),
                                                               rawPostCallback(requestInit.rawPostCallback),
                                                               fileVectorCallback(requestInit.fileVectorCallback) {}

string Request::Post::getRaw() const {
    return rawPostCallback();
}

vector<File> Request::Post::getFileVector(const string &postVar) const {
    return fileVectorCallback(postVar);

//    vector<File> ret;
//    auto e = requestHandler.environment().files.equal_range(postVar);
//    for(auto it = e.first; it != e.second; ++it) {
//        Request::File tmp(it->second);
//        ret.push_back(tmp);
//    }
//    return ret;
}

string Request::Post::getContentType() const {
    return contentType;
}
