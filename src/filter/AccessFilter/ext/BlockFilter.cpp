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
 * \file BlockFilter.cpp
 * \brief Implementation of the BlockFilter class.
 */

#include <nawa/filter/AccessFilter/ext/BlockFilter.h>

using namespace nawa;
using namespace std;

struct BlockFilter::Data {
    unsigned int status = 404;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(BlockFilter)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(BlockFilter)

NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL(BlockFilter, AccessFilter)

NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(BlockFilter, AccessFilter)

NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL(BlockFilter, AccessFilter)

NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(BlockFilter, AccessFilter)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(BlockFilter, status, unsigned int)
