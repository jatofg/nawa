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
 * \file AppInit.cpp
 * \brief Implementation of the AppInit class.
 */

#include <nawa/AppInit.h>

using namespace nawa;
using namespace std;

struct AppInit::Data {
    Config config;
    AccessFilterList accessFilters;
    size_t numThreads;

    Data(Config config, size_t numThreads) : config(std::move(config)), numThreads(numThreads) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(AppInit)

AppInit::AppInit(Config config, size_t numThreads) {
    data = make_unique<Data>(std::move(config), numThreads);
}

Config& nawa::AppInit::config() {
    return data->config;
}

AccessFilterList& nawa::AppInit::accessFilters() {
    return data->accessFilters;
}

size_t nawa::AppInit::getNumThreads() {
    return data->numThreads;
}
