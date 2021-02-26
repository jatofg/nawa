/**
 * \file AccessFilterList.cpp
 * \brief Implementation of the AccessFilterList class.
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

#include <nawa/filter/AccessFilterList.h>

using namespace nawa;
using namespace std;

struct AccessFilterList::Data {
    bool filtersEnabled = false;
    std::vector<BlockFilter> blockFilters;
    std::vector<AuthFilter> authFilters;
    std::vector<ForwardFilter> forwardFilters;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(AccessFilterList)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_COPY_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(AccessFilterList)

NAWA_MOVE_CONSTRUCTOR_IMPL(AccessFilterList)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(AccessFilterList)

bool &nawa::AccessFilterList::filtersEnabled() noexcept {
    return data->filtersEnabled;
}

bool nawa::AccessFilterList::filtersEnabled() const noexcept {
    return data->filtersEnabled;
}

std::vector<BlockFilter> &nawa::AccessFilterList::blockFilters() noexcept {
    return data->blockFilters;
}

std::vector<BlockFilter> const &nawa::AccessFilterList::blockFilters() const noexcept {
    return data->blockFilters;
}

std::vector<AuthFilter> &nawa::AccessFilterList::authFilters() noexcept {
    return data->authFilters;
}

std::vector<AuthFilter> const &nawa::AccessFilterList::authFilters() const noexcept {
    return data->authFilters;
}

std::vector<ForwardFilter> &nawa::AccessFilterList::forwardFilters() noexcept {
    return data->forwardFilters;
}

std::vector<ForwardFilter> const &nawa::AccessFilterList::forwardFilters() const noexcept {
    return data->forwardFilters;
}
