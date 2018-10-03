//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_QSFREQUEST_H
#define QSF_QSFREQUEST_H
#define QSF_REQ_GET 1
#define QSF_REQ_POST 2
#define QSF_REQ_COOKIE 3

#include <string>
#include <fastcgi++/request.hpp>
#include "RequestHandler.h"

class QsfRequest {
public:
    class Env {
    protected:
        RequestHandler& request;
    public:
        Env(RequestHandler& request) : request(request) {}
        std::string operator [](std::string envVar);
    };
    class GPC {
    protected:
        RequestHandler& request;
        uint source;
        std::multimap<std::basic_string<char>, std::basic_string<char>> data;
    public:
        GPC(RequestHandler& request, uint source);
        virtual ~GPC() {}
        std::string operator [](std::string gpcVar);
        std::vector<std::string> getVector(std::string gpcVar);
    };
    class Post: public GPC {
    public:
        Post(RequestHandler& request);
        virtual ~Post() {}
        std::string getRaw();
    };
    /* TODO cookie setting (fits better in Response, probably) */
    const QsfRequest::Env env;
    const QsfRequest::GPC get;
    const QsfRequest::Post post;
    const QsfRequest::GPC cookie;
    QsfRequest(RequestHandler& request);
};


#endif //QSF_QSFREQUEST_H
