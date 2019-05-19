//
// Created by tobias on 21/01/19.
//

#ifndef QSF_APPINIT_H
#define QSF_APPINIT_H

#include <qsf/AccessFilterList.h>

namespace soru {
    struct AppInit {
        /**
         * You can use this to modify the default configuration for every request during initialization.
         */
        Config config;
        /**
         * List of static access filters that can be evaluated before forwarding a request to your app.
         */
        AccessFilterList accessFilters;
    };
}

#endif //QSF_APPINIT_H
