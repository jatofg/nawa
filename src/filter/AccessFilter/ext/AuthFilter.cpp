/**
 * \file AuthFilter.cpp
 * \brief Implementation of the AuthFilter class.
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

#include <nawa/filter/AccessFilter/ext/AuthFilter.h>

using namespace nawa;
using namespace std;

struct AuthFilter::Data {
    std::function<bool(std::string, std::string)> authFunction;
    std::string authName;
    bool useSessions = false;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(AuthFilter)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(AuthFilter)

NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL(AuthFilter, AccessFilter)

NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(AuthFilter, AccessFilter)

NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL(AuthFilter, AccessFilter)

NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(AuthFilter, AccessFilter)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AuthFilter, authFunction, function<bool(string, string)>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(AuthFilter, authName, string)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(AuthFilter, useSessions, bool)
