/**
 * \file Email.cpp
 * \brief Implementation of the Email class
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

#include <nawa/mail/Email/Email.h>

using namespace nawa;
using namespace std;

struct Email::Data {
    std::unordered_map<std::string, std::string> headers;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Email)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(Email)

NAWA_COPY_CONSTRUCTOR_IMPL(Email)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Email)

NAWA_MOVE_CONSTRUCTOR_IMPL(Email)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Email)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Email, headers, Email::HeadersMap)
