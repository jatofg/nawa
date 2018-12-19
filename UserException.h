//
// Created by tobias on 04/12/18.
//

#ifndef QSF_USEREXCEPTION_H
#define QSF_USEREXCEPTION_H

#include <string>
#include <sstream>

namespace Qsf {
    class UserException: public std::exception {
        int errorCode;
        std::string message;
    public:
        UserException(const std::string& inFunction, int errorCode, const std::string& _message) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "QSF: UserException #" << errorCode << " in " << inFunction << ": " << _message;
            message = mstream.str();
        }
        UserException(const std::string& inFunction, int errorCode) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "QSF: UserException #" << errorCode << " in " << inFunction;
            message = mstream.str();
        }
        virtual const int getErrorCode() const noexcept {
            return errorCode;
        }
        virtual const char* what() const noexcept {
            return message.c_str();
        }
    };
}

#endif //QSF_USEREXCEPTION_H
