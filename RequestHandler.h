//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_REQUESTHANDLER_H
#define QSF_REQUESTHANDLER_H


#include <fastcgi++/request.hpp>

namespace Qsf {
    class Request;
    class RequestHandler : public Fastcgipp::Request<char> {
        static size_t postMax;
        friend class Qsf::Request;
        bool response();
    public:
        static void setPostMax(size_t pm);
        RequestHandler();
    };
}


#endif //QSF_REQUESTHANDLER_H
