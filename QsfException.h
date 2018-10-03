//
// Created by tobias on 03/10/18.
//

#ifndef QSF_QSFEXCEPTION_H
#define QSF_QSFEXCEPTION_H


#include <bits/exception.h>
#include <string>
#include <sstream>

class QsfException: public std::exception {
    const char* file;
    uint line;
    const char* message;
public:
    QsfException(const char* file, uint line);
    QsfException(const char* file, uint line, const char* message);
    virtual const char* what() const noexcept;
};


#endif //QSF_QSFEXCEPTION_H
