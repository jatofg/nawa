//
// Created by tobias on 03/10/18.
//

#ifndef QSF_QSFEXCEPTION_H
#define QSF_QSFEXCEPTION_H

#include <string>
#include <sstream>
#include <bits/exception.h>

namespace Qsf {
    class SysException: public std::exception {
        std::string message;
    public:
        SysException(std::string file, unsigned int line) {
            std::stringstream ret;
            ret << "QSF: SysException in " << file << " at line " << line;
            message = ret.str();
        }
        SysException(std::string file, unsigned int line, std::string _message) {
            std::stringstream ret;
            ret << "QSF: SysException in " << file << " at line " << line << ": " << _message;
            message = ret.str();
        }
        virtual const char* what() const noexcept {
            return message.c_str();
        }
    };
}




#endif //QSF_QSFEXCEPTION_H
