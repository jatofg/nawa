/**
 * \file AccessFilter.cpp
 * \brief Implementation of the AccessFilter class.
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

#include <nawa/filter/AccessFilter/AccessFilter.h>

using namespace nawa;
using namespace std;

struct AccessFilter::Data {
    bool invert = false;
    std::vector<std::vector<std::string>> pathFilter;
    bool invertPathFilter = false;
    std::vector<std::string> extensionFilter;
    bool invertExtensionFilter = false;
    bool regexFilterEnabled = false;
    std::regex regexFilter;
    std::string response;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(AccessFilter)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(AccessFilter)

NAWA_COPY_CONSTRUCTOR_IMPL(AccessFilter)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(AccessFilter)

NAWA_MOVE_CONSTRUCTOR_IMPL(AccessFilter)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(AccessFilter)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AccessFilter, invert, bool)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilter, pathFilter, vector<vector<string>>)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AccessFilter, invertPathFilter, bool)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilter, extensionFilter, vector<string>)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AccessFilter, invertExtensionFilter, bool)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AccessFilter, regexFilterEnabled, bool)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilter, regexFilter, regex)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilter, response, string)
