/**
 * \file Request.cpp
 * \brief Implementation of the Request class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <soru/Utils.h>
#include <soru/Request.h>
#include <soru/SysException.h>

std::string soru::Request::Env::operator[](std::string envVar) const {
    std::string ret;
    if(envVar == "host") ret = requestHandler.environment().host; // server hostname
    else if(envVar == "userAgent") ret = requestHandler.environment().userAgent; // user agent string
    else if(envVar == "acceptContentTypes") ret = requestHandler.environment().acceptContentTypes; // content types accepted by client
    else if(envVar == "acceptCharsets") ret = requestHandler.environment().acceptCharsets; // charsets accepted by client
    else if(envVar == "authorization") ret = requestHandler.environment().authorization; // http authorization string
    else if(envVar == "referer") ret = requestHandler.environment().referer; // referral url
    else if(envVar == "contentType") ret = requestHandler.environment().contentType; // content type of data from client
    else if(envVar == "root") ret = requestHandler.environment().root; // http root directory
    else if(envVar == "scriptName") ret = requestHandler.environment().scriptName; // filename of script relative to http root
    else if(envVar == "requestMethod") {
        switch(requestHandler.environment().requestMethod) {
            case Fastcgipp::Http::RequestMethod::ERROR:
                ret = "ERROR";
                break;
            case Fastcgipp::Http::RequestMethod::HEAD:
                ret = "HEAD";
                break;
            case Fastcgipp::Http::RequestMethod::GET:
                ret = "GET";
                break;
            case Fastcgipp::Http::RequestMethod::POST:
                ret = "POST";
                break;
            case Fastcgipp::Http::RequestMethod::PUT:
                ret = "PUT";
                break;
            case Fastcgipp::Http::RequestMethod::DELETE:
                ret = "DELETE";
                break;
            case Fastcgipp::Http::RequestMethod::TRACE:
                ret = "TRACE";
                break;
            case Fastcgipp::Http::RequestMethod::OPTIONS:
                ret = "OPTIONS";
                break;
            case Fastcgipp::Http::RequestMethod::CONNECT:
                ret = "CONNECT";
                break;
        }
    }
    else if(envVar == "requestUri") ret = requestHandler.environment().requestUri; // request uri
    else if(envVar == "serverAddress") {
        std::stringstream stm;
        stm << requestHandler.environment().serverAddress;
        ret = stm.str();
    }
    else if(envVar == "remoteAddress") {
        std::stringstream stm;
        stm << requestHandler.environment().remoteAddress;
        ret = stm.str();
    }
    else if(envVar == "serverPort") ret = std::to_string(requestHandler.environment().serverPort);
    else if(envVar == "remotePort") ret = std::to_string(requestHandler.environment().remotePort);
    else if(envVar == "ifModifiedSince") ret = std::to_string(requestHandler.environment().ifModifiedSince); // unix timestamp
    return ret;
}

std::vector<std::string> soru::Request::Env::getAcceptLanguages() const {
    return requestHandler.environment().acceptLanguages;
}

std::vector<std::string> soru::Request::Env::getRequestPath() const {
    // TODO is this function really necessary? get RP via operator[] when switched to Universal?
    // TODO what does environment().pathInfo do?
    //return requestHandler.environment().pathInfo;
    return soru::split_path(requestHandler.environment().requestUri);
}

Fastcgipp::Address soru::Request::Env::getServerAddr() const {
    return requestHandler.environment().serverAddress;
}

Fastcgipp::Address soru::Request::Env::getRemoteAddr() const {
    return requestHandler.environment().remoteAddress;
}

soru::Request::GPC::GPC(RequestHandler &request, uint source)
        : requestHandler(request), source(source) {
    switch (source) {
        case SORU_REQ_COOKIE:
            data = request.environment().cookies;
            break;
        case SORU_REQ_POST:
            data = request.environment().posts;
            break;
        case SORU_REQ_GET:
            data = request.environment().gets;
            break;
        default:
            throw soru::SysException(__FILE__, __LINE__, "Invalid source for QsfRequest::GPC given");
    }
}

std::string soru::Request::GPC::operator[](std::string gpcVar) const {
    auto e = data.find(gpcVar);
    if(e != data.end()) return e->second;
    else return "";
}

std::vector<std::string> soru::Request::GPC::getVector(std::string gpcVar) const {
    std::vector<std::string> ret;
    auto e = data.equal_range(gpcVar);
    for(auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

unsigned long soru::Request::GPC::count(std::string gpcVar) const {
    return data.count(gpcVar);
}

std::multimap<std::string, std::string>& soru::Request::GPC::getMultimap() {
    return data;
}

std::multimap<std::string, std::string>::const_iterator soru::Request::GPC::begin() const {
    return data.begin();
}

std::multimap<std::string, std::string>::const_iterator soru::Request::GPC::end() const {
    return data.end();
}

soru::Request::Request(RequestHandler &request)
        : env(request), get(request, SORU_REQ_GET), post(request), cookie(request, SORU_REQ_COOKIE) {}

soru::Request::Post::Post(RequestHandler &request) : GPC(request, SORU_REQ_POST) {}

std::string& soru::Request::Post::getRaw() const {
    return requestHandler.rawPost;
}

std::vector<soru::Request::File> soru::Request::Post::getFileVector(std::string postVar) const {
    std::vector<soru::Request::File> ret;
    auto e = requestHandler.environment().files.equal_range(postVar);
    for(auto it = e.first; it != e.second; ++it) {
        soru::Request::File tmp(it->second);
        ret.push_back(tmp);
    }
    return ret;
}

std::string soru::Request::Post::getContentType() const {
    return requestHandler.postContentType;
}

soru::Request::File::File(const Fastcgipp::Http::File<char> &file) : filename(file.filename),
        contentType(file.contentType), size(file.size), dataPtrRef(file.data) {}

std::string soru::Request::File::copyFile() {
    return std::string(dataPtrRef.get(), size);
}

bool soru::Request::File::writeFile(std::string path) {
    std::ofstream outfile;
    std::ios_base::iostate exceptionMask = outfile.exceptions() | std::ios::failbit;
    outfile.exceptions(exceptionMask);
    try {
        outfile.open(path, std::ofstream::out | std::ofstream::binary);
        outfile.write(dataPtrRef.get(), size);
        outfile.close();
    }
    catch(std::ios_base::failure& e) {
        return false;
    }
    return true;
}
