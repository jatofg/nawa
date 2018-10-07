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

namespace Qsf {
    class Request {
    public:
        class Env {
        protected:
            RequestHandler& request;
        public:
            Env(RequestHandler& request) : request(request) {}
            std::string operator [](std::string envVar);
            std::vector<std::string> getAcceptLanguages();
            std::vector<std::string> getPathInfo();
        };
        class GPC {
        protected:
            RequestHandler& request;
            uint source;
            std::multimap<std::basic_string<char>, std::basic_string<char>> data;
        public:
            GPC(RequestHandler& request, uint source);
            virtual ~GPC() {}
            std::string operator [](std::string gpcVar) const;
            std::vector<std::string> getVector(std::string gpcVar) const;
        };
        class Post: public GPC {
        public:
            Post(RequestHandler& request);
            virtual ~Post() {}
            std::string getRaw();
        };
        /* TODO cookie setting (fits better in Response, probably) */
        const Request::Env env;
        const Request::GPC get;
        const Request::Post post;
        const Request::GPC cookie;
        Request(RequestHandler& request);
    };
}




#endif //QSF_QSFREQUEST_H
