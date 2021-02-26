/**
 * \file ForwardFilter.cpp
 * \brief Implementation of the ForwardFilter class.
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

#include <nawa/filter/AccessFilter/ext/ForwardFilter.h>

using namespace nawa;
using namespace std;

struct ForwardFilter::Data {
    std::string basePath;
    BasePathExtension basePathExtension = BasePathExtension::BY_FILENAME;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(ForwardFilter)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(ForwardFilter)

NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL(ForwardFilter, AccessFilter)

NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(ForwardFilter, AccessFilter)

NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL(ForwardFilter, AccessFilter)

NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(ForwardFilter, AccessFilter)

std::string &nawa::ForwardFilter::basePath() noexcept {
    return data->basePath;
}

std::string const &nawa::ForwardFilter::basePath() const noexcept {
    return data->basePath;
}

ForwardFilter::BasePathExtension &nawa::ForwardFilter::basePathExtension() noexcept {
    return data->basePathExtension;
}

ForwardFilter::BasePathExtension nawa::ForwardFilter::basePathExtension() const noexcept {
    return data->basePathExtension;
}
