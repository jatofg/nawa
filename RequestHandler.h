//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_REQUESTHANDLER_H
#define QSF_REQUESTHANDLER_H
#define QSF_RAWPOST_NEVER 0
#define QSF_RAWPOST_NONSTANDARD 1
#define QSF_RAWPOST_ALWAYS 2

#include <fastcgi++/request.hpp>
#include "Config.h"

namespace Qsf {
    class Request;
    class Connection;

    // Types of functions that need to be accessed from QSF applications
    typedef int init_t();
    typedef int handleRequest_t(Qsf::Connection& connection);

    class RequestHandler : public Fastcgipp::Request<char> {
        friend class Qsf::Request;
        static size_t postMax;
        static uint rawPostAccess;
        static Qsf::Config config;
        //static handleRequest_t* appHandleRequest;
        std::string postContentType;
        std::string rawPost;
    public:
        bool response() override;
        void flush(Qsf::Connection& connection);
        bool inProcessor() override;
//        static void setConfig(size_t pm, uint rpa, void* appOpen);
        static void setConfig(const Qsf::Config& cfg, void* appOpen);
        RequestHandler();
    };
}

#endif //QSF_REQUESTHANDLER_H
