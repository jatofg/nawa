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

#include <fstream>
#include <nawa/Utils.h>
#include <nawa/Request.h>
#include <nawa/SysException.h>

std::string nawa::Request::Env::operator[](const std::string& envVar) const {
    std::string ret;
    auto const &renv = requestHandler.environment();
    if(envVar == "host") ret = renv.host; // server hostname
    else if(envVar == "userAgent") ret = renv.userAgent; // user agent string
    else if(envVar == "acceptContentTypes") ret = renv.acceptContentTypes; // content types accepted by client
    else if(envVar == "acceptCharsets") ret = renv.acceptCharsets; // charsets accepted by client
    else if(envVar == "authorization") ret = renv.authorization; // http authorization string
    else if(envVar == "referer") ret = renv.referer; // referral url
    else if(envVar == "contentType") ret = renv.contentType; // content type of data from client
    else if(envVar == "root") ret = renv.root; // http root directory
    else if(envVar == "scriptName") ret = renv.scriptName; // filename of script relative to http root
    else if(envVar == "requestMethod") {
        switch(renv.requestMethod) {
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
    else if(envVar == "requestUri") ret = renv.requestUri; // request uri
    else if(envVar == "serverAddress") {
        std::stringstream stm;
        stm << renv.serverAddress;
        ret = stm.str();
    }
    else if(envVar == "remoteAddress") {
        std::stringstream stm;
        stm << renv.remoteAddress;
        ret = stm.str();
    }
    else if(envVar == "serverPort") ret = std::to_string(renv.serverPort);
    else if(envVar == "remotePort") ret = std::to_string(renv.remotePort);
    else if(envVar == "ifModifiedSince") ret = std::to_string(renv.ifModifiedSince); // unix timestamp
    else if(envVar == "https") ret = renv.others.count("HTTPS") ? renv.others.at("HTTPS") : ""; // "on" if accessed via HTTPS
    else if(envVar == "serverName") ret = renv.others.count("SERVER_NAME") ? renv.others.at("SERVER_NAME") : ""; // server's FQDN
    else if(envVar == "serverSoftware") ret = renv.others.count("SERVER_SOFTWARE") ? renv.others.at("SERVER_SOFTWARE") : ""; // server software (e.g., Apache)
    else if(envVar == "baseUrl") {
        std::stringstream stm;
        auto https = renv.others.count("HTTPS");
        stm << (https ? "https://" : "http://")
            << renv.host;
        if((!https && renv.serverPort != 80) || (https && renv.serverPort != 443)) {
            stm << ":" << renv.serverPort;
        }
        ret = stm.str();
    }
    else if(envVar == "fullUrlWithQS") {
        auto baseUrl = (*this)["baseUrl"];
        ret = baseUrl + renv.requestUri;
    }
    else if(envVar == "fullUrlWithoutQS") {
        auto baseUrl = (*this)["baseUrl"];
        ret = baseUrl + renv.requestUri.substr(0, renv.requestUri.find_first_of('?'));
    }
    return ret;
}

std::vector<std::string> nawa::Request::Env::getAcceptLanguages() const {
    return requestHandler.environment().acceptLanguages;
}

std::vector<std::string> nawa::Request::Env::getRequestPath() const {
    // TODO is this function really necessary? get RP via operator[] when switched to Any?
    // TODO what does environment().pathInfo do?
    //return requestHandler.environment().pathInfo;
    return nawa::split_path(requestHandler.environment().requestUri);
}

Fastcgipp::Address nawa::Request::Env::getServerAddr() const {
    return requestHandler.environment().serverAddress;
}

Fastcgipp::Address nawa::Request::Env::getRemoteAddr() const {
    return requestHandler.environment().remoteAddress;
}

nawa::Request::GPC::GPC(RequestHandlerLegacy &request, Source source)
        : requestHandler(request), source(source) {
    switch (source) {
        case Source::COOKIE:
            data = request.environment().cookies;
            break;
        case Source::POST:
            data = request.environment().posts;
            break;
        case Source::GET:
            data = request.environment().gets;
            break;
        default:
            throw nawa::SysException(__FILE__, __LINE__, "Invalid source for nawa::Request::GPC given");
    }
}

std::string nawa::Request::GPC::operator[](const std::string& gpcVar) const {
    auto e = data.find(gpcVar);
    if(e != data.end()) return e->second;
    else return "";
}

std::vector<std::string> nawa::Request::GPC::getVector(const std::string& gpcVar) const {
    std::vector<std::string> ret;
    auto e = data.equal_range(gpcVar);
    for(auto it = e.first; it != e.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

unsigned long nawa::Request::GPC::count(const std::string& gpcVar) const {
    return data.count(gpcVar);
}

std::multimap<std::string, std::string>& nawa::Request::GPC::getMultimap() {
    return data;
}

std::multimap<std::string, std::string>::const_iterator nawa::Request::GPC::begin() const {
    return data.begin();
}

std::multimap<std::string, std::string>::const_iterator nawa::Request::GPC::end() const {
    return data.end();
}

nawa::Request::Request(RequestHandlerLegacy &request)
        : env(request), get(request, GPC::Source::GET), post(request), cookie(request, GPC::Source::COOKIE) {}

nawa::Request::Post::Post(RequestHandlerLegacy &request) : GPC(request, GPC::Source::POST) {}

std::string& nawa::Request::Post::getRaw() const {
    return requestHandler.rawPost;
}

std::vector<nawa::Request::File> nawa::Request::Post::getFileVector(const std::string& postVar) const {
    std::vector<nawa::Request::File> ret;
    auto e = requestHandler.environment().files.equal_range(postVar);
    for(auto it = e.first; it != e.second; ++it) {
        nawa::Request::File tmp(it->second);
        ret.push_back(tmp);
    }
    return ret;
}

std::string nawa::Request::Post::getContentType() const {
    return requestHandler.postContentType;
}

nawa::Request::File::File(const Fastcgipp::Http::File<char> &file) : filename(file.filename),
        contentType(file.contentType), size(file.size), dataPtrRef(file.data) {}

std::string nawa::Request::File::copyFile() {
    return std::string(dataPtrRef.get(), size);
}

bool nawa::Request::File::writeFile(const std::string& path) {
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
