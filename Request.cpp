//
// Created on 30/09/18. All rights reserved.
//

#include "Request.h"
#include "Exception.h"

std::string Qsf::Request::Env::operator[](std::string envVar) {
    std::string ret;
    if (envVar == "referer") ret = request.environment().referer;
    return ret;
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

Qsf::Request::Request(RequestHandler &request)
        : env(request), get(request, QSF_REQ_GET), post(request), cookie(request, QSF_REQ_COOKIE) {

}

Qsf::Request::Post::Post(RequestHandler &request) : GPC(request, QSF_REQ_POST) {

}

std::string Qsf::Request::Post::getRaw() {
    auto postBuffer = request.environment().postBuffer();
    std::string ret(postBuffer.data(), postBuffer.size());
    return ret;
}
