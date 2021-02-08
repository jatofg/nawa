/**
 * \file Post.cpp
 * \brief Implementation of the request::Post class.
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

#include <nawa/request/GPC/ext/Post.h>

using namespace nawa;
using namespace std;

request::Post::Post(const RequestInitContainer &requestInit) : GPC(requestInit, GPC::Source::POST),
                                                               contentType(requestInit.postContentType),
                                                               rawPost(requestInit.rawPost),
                                                               fileMap(requestInit.postFiles) {}

request::Post::operator bool() const {
    return !(dataMap.empty() && fileMap.empty());
}

shared_ptr<string> request::Post::getRaw() const {
    return rawPost;
}

string request::Post::getContentType() const {
    return contentType;
}

bool request::Post::hasFiles() const {
    return !fileMap.empty();
}

File request::Post::getFile(const string &postVar) const {
    auto e = fileMap.find(postVar);
    if (e != fileMap.end()) return e->second;
    return File();
}

vector<File> request::Post::getFileVector(const string &postVar) const {
    vector<File> ret;
    auto e = fileMap.equal_range(postVar);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

size_t request::Post::countFiles(const string &postVar) const {
    return fileMap.count(postVar);
}

std::unordered_multimap<std::string, File> const &request::Post::getFileMultimap() const {
    return fileMap;
}