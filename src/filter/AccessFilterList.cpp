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
 * \file AccessFilterList.cpp
 * \brief Implementation of the AccessFilterList class.
 */

#include <nawa/filter/AccessFilterList.h>

using namespace nawa;
using namespace std;

struct AccessFilterList::Data {
    bool filtersEnabled = false;
    vector<BlockFilter> blockFilters;
    vector<AuthFilter> authFilters;
    vector<ForwardFilter> forwardFilters;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(AccessFilterList)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_COPY_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(AccessFilterList)

NAWA_MOVE_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(AccessFilterList)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AccessFilterList, filtersEnabled, bool)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilterList, blockFilters, vector<BlockFilter>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilterList, authFilters, vector<AuthFilter>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AccessFilterList, forwardFilters, vector<ForwardFilter>)
