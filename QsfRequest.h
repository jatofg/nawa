//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_QSFREQUEST_H
#define QSF_QSFREQUEST_H

#include <string>
#include <fastcgi++/request.hpp>
#include "RequestHandler.h"

class QsfRequest {
public:
    class Env {
        RequestHandler& request;
    public:
        Env(RequestHandler& request) : request(request) {}
        std::wstring operator [](std::string envVar);
    };
    class Get {
        RequestHandler& request;
    public:
        Get(RequestHandler& request) : request(request) {}
    };
    class Post {
        RequestHandler& request;
    public:
        Post(RequestHandler& request) : request(request) {}
    };
    const QsfRequest::Env env;
    const QsfRequest::Get get;
    const QsfRequest::Post post;
public:
    QsfRequest(RequestHandler& request) : env(request), get(request), post(request) {}
};


#endif //QSF_QSFREQUEST_H
