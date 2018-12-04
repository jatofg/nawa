//
// Created by tobias on 03/10/18.
//

#ifndef QSF_QSFEXCEPTION_H
#define QSF_QSFEXCEPTION_H

#include <bits/exception.h>

namespace Qsf {
    class SysException: public std::exception {
        const char* file;
        unsigned int line;
        const char* message;
    public:
        SysException(const char* file, unsigned int line);
        SysException(const char* file, unsigned int line, const char* message);
        virtual const char* what() const noexcept;
    };
}




#endif //QSF_QSFEXCEPTION_H
