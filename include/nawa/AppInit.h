/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file AppInit.h
 * \brief Structure passed to the init function of nawa apps.
 */

#ifndef NAWA_APPINIT_H
#define NAWA_APPINIT_H

#include <nawa/config/Config.h>
#include <nawa/filter/AccessFilterList.h>
#include <nawa/internal/macros.h>

namespace nawa {
    class AppInit {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(AppInit);

        /**
         * Create a new AppInit object.
         * @param config The config.
         * @param numThreads Number of threads.
         */
        AppInit(Config config, size_t numThreads);

        /**
         * You can use this to modify the default configuration for every request during initialization.
         * @return Reference to the configuration.
         */
        Config& config();

        /**
         * Contains the number of started worker threads for information purposes (e.g., to determine dimension of
         * thread pools).
         * @return Reference to the access filters container.
         */
        AccessFilterList& accessFilters();

        /**
         * Contains the number of started worker threads for information purposes (e.g., to determine dimension of
         * thread pools).
         * @return Number of started worker threads.
         */
        size_t getNumThreads();
    };
}// namespace nawa

#endif//NAWA_APPINIT_H
