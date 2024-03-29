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
 * \file ForwardFilter.h
 * \brief Structure defining a forwarding filter.
 */

#ifndef NAWA_FORWARDFILTER_H
#define NAWA_FORWARDFILTER_H

#include <nawa/filter/AccessFilter/AccessFilter.h>

namespace nawa {
    /**
     * Filter to bypass the NAWA app for certain requests and directly forward files to the client. This is especially
     * useful for media files, e.g. images, as well as CSS stylesheets.
     */
    class ForwardFilter : public AccessFilter {
        NAWA_PRIVATE_DATA()

    public:
        /**
         * How a file will be looked up below the base path:
         * - FILENAME: Only the file name will be added to the base path (i.e., a file "/dir1/dir2/file.ext" will be
         *   looked up in basePath."/file.ext").
         * - PATH: The request path will be added to the base path (e.g., the file mentioned above would be looked up
         *   in basepath."/dir1/dir2/file.ext").
         */
        enum class BasePathExtension {
            BY_FILENAME,
            BY_PATH
        };

        NAWA_DEFAULT_DESTRUCTOR_OVERRIDE_DEF(ForwardFilter);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(ForwardFilter);

        NAWA_COPY_CONSTRUCTOR_DEF(ForwardFilter);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(ForwardFilter);

        NAWA_MOVE_CONSTRUCTOR_DEF(ForwardFilter);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(ForwardFilter);

        /**
         * The path under which the file will be looked up (should be an absolute OS path to the files
         * starting with a '/' and *not* ending with a '/', e.g., "/var/www/website1").
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(ForwardFilter, basePath, std::string);

        /**
         * How the file will be looked up (see explanation of enum BasePathExtension, default: only file name).
         * @return Reference to element.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(ForwardFilter, basePathExtension, BasePathExtension);
    };
}// namespace nawa

#endif//NAWA_FORWARDFILTER_H
