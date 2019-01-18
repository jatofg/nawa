/**
 * \file Request.cpp
 * \brief Implementation of the Request class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <fstream>
#include "qsf/Request.h"
#include "qsf/SysException.h"

std::string Qsf::Request::Env::operator[](std::string envVar) const {
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

std::vector<std::string> Qsf::Request::Env::getAcceptLanguages() const {
    return requestHandler.environment().acceptLanguages;
}

std::vector<std::string> Qsf::Request::Env::getPathInfo() const {
    return requestHandler.environment().pathInfo;
}

Fastcgipp::Http::Address Qsf::Request::Env::getServerAddr() const {
    return requestHandler.environment().serverAddress;
}

Fastcgipp::Http::Address Qsf::Request::Env::getRemoteAddr() const {
    return requestHandler.environment().remoteAddress;
}

Qsf::Request::GPC::GPC(RequestHandler &request, uint source)
        : requestHandler(request), source(source) {
    switch (source) {
        case QSF_REQ_COOKIE:
            data = request.environment().cookies;
            break;
        case QSF_REQ_POST:
            data = request.environment().posts;
            break;
        case QSF_REQ_GET:
            data = request.environment().gets;
            break;
        default:
            throw Qsf::SysException(__FILE__, __LINE__, "Invalid source for QsfRequest::GPC given");
    }
}

std::string Qsf::Request::GPC::operator[](std::string gpcVar) const {
    auto e = data.find(gpcVar);
    if(e != data.end()) return e->second;
    else return "";
}

std::vector<std::string> Qsf::Request::GPC::getVector(std::string gpcVar) const {
    std::vector<std::string> ret;
    auto e = data.equal_range(gpcVar);
    for(auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

unsigned long Qsf::Request::GPC::count(std::string gpcVar) const {
    return data.count(gpcVar);
}

std::multimap<std::string, std::string>& Qsf::Request::GPC::getMultimap() {
    return data;
}

std::multimap<std::string, std::string>::const_iterator Qsf::Request::GPC::begin() const {
    return data.begin();
}

std::multimap<std::string, std::string>::const_iterator Qsf::Request::GPC::end() const {
    return data.end();
}

Qsf::Request::Request(RequestHandler &request)
        : env(request), get(request, QSF_REQ_GET), post(request), cookie(request, QSF_REQ_COOKIE) {}

Qsf::Request::Post::Post(RequestHandler &request) : GPC(request, QSF_REQ_POST) {}

std::string& Qsf::Request::Post::getRaw() const {
    return requestHandler.rawPost;
}

std::vector<Qsf::Request::File> Qsf::Request::Post::getFileVector(std::string postVar) const {
    std::vector<Qsf::Request::File> ret;
    auto e = requestHandler.environment().files.equal_range(postVar);
    for(auto it = e.first; it != e.second; ++it) {
        Qsf::Request::File tmp(it->second);
        ret.push_back(tmp);
    }
    return ret;
}

std::string Qsf::Request::Post::getContentType() const {
    return requestHandler.postContentType;
}

Qsf::Request::File::File(const Fastcgipp::Http::File<char> &file) : filename(file.filename),
        contentType(file.contentType), size(file.size), dataPtrRef(file.data) {}

std::string Qsf::Request::File::copyFile() {
    return std::string(dataPtrRef.get(), size);
}

bool Qsf::Request::File::writeFile(std::string path) {
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
