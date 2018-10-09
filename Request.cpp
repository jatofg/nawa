//
// Created on 30/09/18. All rights reserved.
//

#include <fstream>
#include "Request.h"
#include "Exception.h"

std::string Qsf::Request::Env::operator[](std::string envVar) const {
    std::string ret;
    if(envVar == "host") ret = request.environment().host; // server hostname
    else if(envVar == "userAgent") ret = request.environment().userAgent; // user agent string
    else if(envVar == "acceptContentTypes") ret = request.environment().acceptContentTypes; // content types accepted by client
    else if(envVar == "acceptCharsets") ret = request.environment().acceptCharsets; // charsets accepted by client
    else if(envVar == "authorization") ret = request.environment().authorization; // http authorization string
    else if(envVar == "referer") ret = request.environment().referer; // referral url
    else if(envVar == "contentType") ret = request.environment().contentType; // content type of data from client
    else if(envVar == "root") ret = request.environment().root; // http root directory
    else if(envVar == "scriptName") ret = request.environment().scriptName; // filename of script relative to http root
    else if(envVar == "requestMethod") {
        switch(request.environment().requestMethod) {
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
    else if(envVar == "requestUri") ret = request.environment().requestUri; // request uri
    else if(envVar == "serverAddress") {
        std::stringstream stm;
        stm << request.environment().serverAddress;
        ret = stm.str();
    }
    else if(envVar == "remoteAddress") {
        std::stringstream stm;
        stm << request.environment().remoteAddress;
        ret = stm.str();
    }
    else if(envVar == "serverPort") ret = std::to_string(request.environment().serverPort);
    else if(envVar == "remotePort") ret = std::to_string(request.environment().remotePort);
    else if(envVar == "ifModifiedSince") ret = std::to_string(request.environment().ifModifiedSince); // unix timestamp
    return ret;
}

std::vector<std::string> Qsf::Request::Env::getAcceptLanguages() const {
    return request.environment().acceptLanguages;
}

std::vector<std::string> Qsf::Request::Env::getPathInfo() const {
    return request.environment().pathInfo;
}

Fastcgipp::Http::Address Qsf::Request::Env::getServerAddr() const {
    return request.environment().serverAddress;
}

Fastcgipp::Http::Address Qsf::Request::Env::getRemoteAddr() const {
    return request.environment().remoteAddress;
}

Qsf::Request::GPC::GPC(RequestHandler &request, uint source)
        : request(request), source(source) {
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
            throw Qsf::Exception(__FILE__, __LINE__, "Invalid source for QsfRequest::GPC given");
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

Qsf::Request::Request(RequestHandler &request)
        : env(request), get(request, QSF_REQ_GET), post(request), cookie(request, QSF_REQ_COOKIE) {}

Qsf::Request::Post::Post(RequestHandler &request) : GPC(request, QSF_REQ_POST) {}

std::string& Qsf::Request::Post::getRaw() const {
    return request.rawPost;
}

std::vector<Qsf::Request::File> Qsf::Request::Post::getFileVector(std::string postVar) const {
    std::vector<Qsf::Request::File> ret;
    auto e = request.environment().files.equal_range(postVar);
    for(auto it = e.first; it != e.second; ++it) {
        Qsf::Request::File tmp(it->second);
        ret.push_back(tmp);
    }
    return ret;
}

std::string Qsf::Request::Post::getContentType() const {
    return request.postContentType;
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
