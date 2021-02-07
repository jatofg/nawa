/**
 * \file AppInit.h
 * \brief Structure passed to the init function of nawa apps.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NAWA_APPINIT_H
#define NAWA_APPINIT_H

#include <nawa/config/Config.h>
#include <nawa/filter/AccessFilterList.h>

namespace nawa {
    struct AppInit {
        /**
         * You can use this to modify the default configuration for every request during initialization.
         */
        Config config;
        /**
         * List of static access filters that can be evaluated before forwarding a request to your app.
         */
        AccessFilterList accessFilters;
        /**
         * Contains the number of started worker threads for information purposes
         * (e.g., to determine dimension of thread pools).
         */
        size_t numThreads;
    };
}

#endif //NAWA_APPINIT_H
