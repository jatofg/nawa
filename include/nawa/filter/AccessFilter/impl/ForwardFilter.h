/**
 * \file ForwardFilter.h
 * \brief Structure defining a forwarding filter.
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

#ifndef NAWA_FORWARDFILTER_H
#define NAWA_FORWARDFILTER_H

#include <nawa/filter/AccessFilter/AccessFilter.h>

namespace nawa {
    /**
     * Filter to bypass the NAWA app for certain requests and directly forward files to the client. This is especially
     * useful for media files, e.g. images, as well as CSS stylesheets.
     */
    struct ForwardFilter : public AccessFilter {
        /**
         * The path under which the file will be looked up (should be an absolute OS path to the files
         * starting with a '/' and *not* ending with a '/', e.g., "/var/www/website1").
         */
        std::string basePath;
        /**
         * How should the file be looked up under the base path:
         * - FILENAME: Only the file name will be added to the base path (i.e., a file "/dir1/dir2/file.ext" will be
         * looked up in basePath."/file.ext".
         * - PATH: The request path will be added to the base path (e.g., the file mentioned above would be looked up
         * in basepath."/dir1/dir2/file.ext".
         */
        enum BasePathExtension {
            BY_FILENAME, BY_PATH
        } basePathExtension = BY_FILENAME;
    };
}

#endif //NAWA_FORWARDFILTER_H
