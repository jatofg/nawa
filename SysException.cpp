//
// Created by tobias on 03/10/18.
//

#include "SysException.h"

const char *Qsf::SysException::what() const noexcept {
    std::stringstream ret;
    ret << "QSF: SysException in " << file << " at line " << line << ": " << message;
    return ret.str().c_str();
}

Qsf::SysException::SysException(const char *file, uint line) : file(file), line(line) {
    message = "No message provided";
}

Qsf::SysException::SysException(const char *file, uint line, const char *message) : file(file), line(line), message(message) {
}
