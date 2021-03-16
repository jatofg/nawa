/**
 * \file AccessFilterList.h
 * \brief Options to check the path and invoke certain actions before forwarding the request to the app.
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

#ifndef NAWA_ACCESSRULES_H
#define NAWA_ACCESSRULES_H

#include <nawa/filter/AccessFilter/ext/AuthFilter.h>
#include <nawa/filter/AccessFilter/ext/BlockFilter.h>
#include <nawa/filter/AccessFilter/ext/ForwardFilter.h>
#include <nawa/internal/macros.h>
#include <vector>

namespace nawa {
    /**
     * Structure in which all filters that should be applied by the RequestHandler can be included. The filters will
     * be processed from the first element in a vector to the last element, block filters first, then auth filters,
     * then forward filters. If one filter leads to a block/forward/denied access, all following filters will be ignored.
     * Filters can only be applied statically on app initialization as part of the AppInit struct (for thread-safety).
     */
    class AccessFilterList {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(AccessFilterList);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(AccessFilterList);

        NAWA_COPY_CONSTRUCTOR_DEF(AccessFilterList);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(AccessFilterList);

        NAWA_MOVE_CONSTRUCTOR_DEF(AccessFilterList);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(AccessFilterList);

        /**
         * Is the filter module enabled? If false, no filters will be applied.
         * @return Reference to element.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(AccessFilterList, filtersEnabled, bool);

        /**
         * List of BlockFilter objects to be applied.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(AccessFilterList, blockFilters, std::vector<BlockFilter>);

        /**
         * List of AuthFilter objects to be applied.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(AccessFilterList, authFilters, std::vector<AuthFilter>);

        /**
         * List of ForwardFilter objects to be applied.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(AccessFilterList, forwardFilters, std::vector<ForwardFilter>);
    };
}// namespace nawa

#endif//NAWA_ACCESSRULES_H
