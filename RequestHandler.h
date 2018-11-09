//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_REQUESTHANDLER_H
#define QSF_REQUESTHANDLER_H
#define QSF_RAWPOST_NEVER 0
#define QSF_RAWPOST_NONSTANDARD 1
#define QSF_RAWPOST_ALWAYS 2

#include <fastcgi++/request.hpp>

namespace Qsf {
    class Request;
    class Response;

    // Types of functions that need to be accessed from QSF applications
    typedef int init_t();
    typedef int handleRequest_t(Qsf::Request& request, Qsf::Response& response);

    class RequestHandler : public Fastcgipp::Request<char> {
        friend class Qsf::Request;
        static size_t postMax;
        static uint rawPostAccess;
        //static handleRequest_t* appHandleRequest;
        std::string postContentType;
        std::string rawPost;
    public:
        bool response() override;
        void flush(Qsf::Response& response);
        bool inProcessor() override;
        static void setConfig(size_t pm, uint rpa, void* appOpen);
        RequestHandler();
    };
}

#endif //QSF_REQUESTHANDLER_H
