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

bool &nawa::AccessFilter::invert() noexcept {
    return data->invert;
}

bool nawa::AccessFilter::invert() const noexcept {
    return data->invert;
}

std::vector<std::vector<std::string>> &nawa::AccessFilter::pathFilter() noexcept {
    return data->pathFilter;
}

std::vector<std::vector<std::string>> const &nawa::AccessFilter::pathFilter() const noexcept {
    return data->pathFilter;
}

bool &nawa::AccessFilter::invertPathFilter() noexcept {
    return data->invertPathFilter;
}

bool nawa::AccessFilter::invertPathFilter() const noexcept {
    return data->invertPathFilter;
}

std::vector<std::string> &nawa::AccessFilter::extensionFilter() noexcept {
    return data->extensionFilter;
}

std::vector<std::string> const &nawa::AccessFilter::extensionFilter() const noexcept {
    return data->extensionFilter;
}

bool &nawa::AccessFilter::invertExtensionFilter() noexcept {
    return data->invertExtensionFilter;
}

bool nawa::AccessFilter::invertExtensionFilter() const noexcept {
    return data->invertExtensionFilter;
}

bool &nawa::AccessFilter::regexFilterEnabled() noexcept {
    return data->regexFilterEnabled;
}

bool nawa::AccessFilter::regexFilterEnabled() const noexcept {
    return data->regexFilterEnabled;
}

std::regex &nawa::AccessFilter::regexFilter() noexcept {
    return data->regexFilter;
}

std::regex const &nawa::AccessFilter::regexFilter() const noexcept {
    return data->regexFilter;
}

std::string &nawa::AccessFilter::response() noexcept {
    return data->response;
}

std::string const &nawa::AccessFilter::response() const noexcept {
    return data->response;
}
