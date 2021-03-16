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
#include <nawa/request/RequestInitContainer.h>

using namespace nawa;
using namespace std;

struct request::Post::Data {
    std::string contentType;
    std::shared_ptr<std::string> rawPost;
    std::unordered_multimap<std::string, File> fileMap;

    explicit Data(RequestInitContainer const& requestInit) : contentType(requestInit.postContentType),
                                                             rawPost(requestInit.rawPost),
                                                             fileMap(requestInit.postFiles) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(request, Post)

request::Post::Post(RequestInitContainer const& requestInit) : GPC(requestInit, GPC::Source::POST) {
    data = make_unique<Data>(requestInit);
}

request::Post::operator bool() const {
    return !(getMultimap().empty() && data->fileMap.empty());
}

shared_ptr<string const> request::Post::getRaw() const {
    return data->rawPost;
}

string request::Post::getContentType() const {
    return data->contentType;
}

bool request::Post::hasFiles() const {
    return !data->fileMap.empty();
}

optional<File> request::Post::getFile(string const& key) const {
    auto e = data->fileMap.find(key);
    if (e != data->fileMap.end())
        return e->second;
    return nullopt;
}

vector<File> request::Post::getFileVector(string const& key) const {
    vector<File> ret;
    auto e = data->fileMap.equal_range(key);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

size_t request::Post::countFiles(string const& key) const {
    return data->fileMap.count(key);
}

std::unordered_multimap<std::string, File> const& request::Post::getFileMultimap() const {
    return data->fileMap;
}
