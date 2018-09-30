//
// Created on 30/09/18. All rights reserved.
//

#ifndef QSF_REQUESTHANDLER_H
#define QSF_REQUESTHANDLER_H


#include <fastcgi++/request.hpp>

class RequestHandler : public Fastcgipp::Request<wchar_t> {
    friend class QsfRequest;
    bool response();
};


#endif //QSF_REQUESTHANDLER_H
