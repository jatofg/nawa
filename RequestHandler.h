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
    class RequestHandler : public Fastcgipp::Request<char> {
        friend class Qsf::Request;
        static size_t postMax;
        static uint rawPostAccess;
        std::string postContentType;
        std::string rawPost;
    public:
        bool response() override;
        bool inProcessor() override;
        static void setPostConfig(size_t pm, uint rpa);
        RequestHandler();
    };
}


#endif //QSF_REQUESTHANDLER_H
