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

struct request::Post::Impl {
    std::string contentType;
    std::shared_ptr<std::string> rawPost;
    std::unordered_multimap<std::string, File> fileMap;

    explicit Impl(const RequestInitContainer &requestInit) : contentType(requestInit.postContentType),
                                                             rawPost(requestInit.rawPost),
                                                             fileMap(requestInit.postFiles) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(request, Post)

request::Post::Post(const RequestInitContainer &requestInit) : GPC(requestInit, GPC::Source::POST) {
    impl = make_unique<Impl>(requestInit);
}

request::Post::operator bool() const {
    return !(getMultimap().empty() && impl->fileMap.empty());
}

shared_ptr<string> request::Post::getRaw() const {
    return impl->rawPost;
}

string request::Post::getContentType() const {
    return impl->contentType;
}

bool request::Post::hasFiles() const {
    return !impl->fileMap.empty();
}

File request::Post::getFile(const string &postVar) const {
    auto e = impl->fileMap.find(postVar);
    if (e != impl->fileMap.end()) return e->second;
    return File();
}

vector<File> request::Post::getFileVector(const string &postVar) const {
    vector<File> ret;
    auto e = impl->fileMap.equal_range(postVar);
    for (auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

size_t request::Post::countFiles(const string &postVar) const {
    return impl->fileMap.count(postVar);
}

std::unordered_multimap<std::string, File> const &request::Post::getFileMultimap() const {
    return impl->fileMap;
}