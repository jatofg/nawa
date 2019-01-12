//
// Created by tobias on 12/01/19.
//

#ifndef QSF_APP_H
#define QSF_APP_H

#include "qsf/Connection.h"

extern "C" int init();
extern "C" int handleRequest(Qsf::Connection& connection);

#endif //QSF_APP_H
