//
// Created on 30/09/18. All rights reserved.
//

#include "QsfRequest.h"

std::wstring QsfRequest::Env::operator[](std::string envVar) {
    std::wstring ret;
    if (envVar == "referer") ret = request.environment().referer;
    return ret;
}
