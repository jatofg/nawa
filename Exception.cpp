//
// Created by tobias on 03/10/18.
//

#include "Exception.h"

const char *Qsf::Exception::what() const noexcept {
    std::stringstream ret;
    ret << "QSF: Exception in " << file << " at line " << line << ": " << message;
    return ret.str().c_str();
}

Qsf::Exception::Exception(const char *file, uint line) : file(file), line(line) {
    message = "No message provided";
}

Qsf::Exception::Exception(const char *file, uint line, const char *message) : file(file), line(line), message(message) {
}
