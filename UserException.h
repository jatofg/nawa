//
// Created by tobias on 04/12/18.
//

#ifndef QSF_USEREXCEPTION_H
#define QSF_USEREXCEPTION_H

#include <bits/exception.h>

namespace Qsf {
    class UserException: public std::exception {
        const char* inFunction;
        int errorCode;
        const char* message;
    public:
        UserException(const char* inFunction, int errorCode, const char* message);
        UserException(const char* inFunction, int errorCode);
        virtual const char* what() const noexcept;
    };
}

#endif //QSF_USEREXCEPTION_H
