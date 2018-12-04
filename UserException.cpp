//
// Created by tobias on 04/12/18.
//

#include "UserException.h"
#include <string>
#include <sstream>

Qsf::UserException::UserException(const char *inFunction, int errorCode, const char *message)
        : inFunction(inFunction), errorCode(errorCode), message(message) {

}

Qsf::UserException::UserException(const char *inFunction, int errorCode) : inFunction(inFunction), errorCode(errorCode) {
    message = "No message provided.";
}

const char *Qsf::UserException::what() const noexcept {
    std::stringstream ret;
    ret << "QSF: UserException #" << errorCode << " in " << inFunction << ": " << message;
    return ret.str().c_str();
}
