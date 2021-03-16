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
#include <nawa/util/utils.h>

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

bool nawa::AccessFilter::matches(vector<string> const& requestPath) const {
    if (!data->pathFilter.empty()) {
        // one of the paths in the path filter must match for the path filter to match
        bool pathFilterMatches = false;
        for (auto const& filter : data->pathFilter) {
            // path condition is set but does not match -> the whole filter does not match
            // all elements of the filter path must be in the request path
            if (requestPath.size() < filter.size()) {
                continue;
            }
            pathFilterMatches = true;
            for (size_t i = 0; i < filter.size(); ++i) {
                if (filter.at(i) != requestPath.at(i)) {
                    pathFilterMatches = false;
                    break;
                }
            }
            if (pathFilterMatches) {
                break;
            }
        }
        if ((!pathFilterMatches && !data->invertPathFilter) || (pathFilterMatches && data->invertPathFilter)) {
            return false;
        }
        // path condition matches -> continue to the next filter condition
    }

    if (!data->extensionFilter.empty()) {
        auto fileExtension = get_file_extension(requestPath.back());
        bool extensionFilterMatches = false;
        for (auto const& e : data->extensionFilter) {
            if (fileExtension == e) {
                extensionFilterMatches = true;
                break;
            }
        }
        if ((!extensionFilterMatches && !data->invertExtensionFilter) ||
            (extensionFilterMatches && data->invertExtensionFilter)) {
            return false;
        }
        // extension condition matches -> continue to the next filter condition
    }

    if (data->regexFilterEnabled) {
        // merge request path to string
        stringstream pathStr;
        for (auto const& e : requestPath) {
            pathStr << '/' << e;
        }
        if (!regex_match(pathStr.str(), data->regexFilter))
            return false;
    }

    // all conditions match or no condition has been set -> the filter matches
    return true;
}
