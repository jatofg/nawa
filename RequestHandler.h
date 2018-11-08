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
        static handleRequest_t* handleRequestApp;
        std::string postContentType;
        std::string rawPost;
    public:
        static init_t* initApp;
        bool response() override;
        void flush(Qsf::Response& response);
        bool inProcessor() override;
        static void setPostConfig(size_t pm, uint rpa);
        static void setAppPointers(init_t* init_f, handleRequest_t* handleRequest_f);
        RequestHandler();
    };
}

#endif //QSF_REQUESTHANDLER_H
