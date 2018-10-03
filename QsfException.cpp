//
// Created by tobias on 03/10/18.
//

#include "QsfException.h"

const char *QsfException::what() const noexcept {
    std::stringstream ret;
    ret << "QSF: Exception in " << file << " at line " << line << ": " << message;
    return ret.str().c_str();
}

QsfException::QsfException(const char *file, uint line) : file(file), line(line) {
    message = "No message provided";
}

QsfException::QsfException(const char *file, uint line, const char *message) : file(file), line(line), message(message) {
}
