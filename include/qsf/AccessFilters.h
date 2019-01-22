/**
 * \file AccessRules.h
 * \brief Options to check the path and invoke certain actions before forwarding the request to the app.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_ACCESSRULES_H
#define QSF_ACCESSRULES_H

#include <string>
#include <vector>
#include <regex>
#include <functional>

namespace Qsf {

    /**
     * Defines a filter on the request path.
     */
    struct AccessFilter {
        /**
         * Type of the filter:
         * - EXTENSION: match all files with a certain file extension
         * - PATH: match everything under a certain request path (directory)
         * - REGEX: matches the request path with a given regex
         */
        enum FilterType {EXTENSION, PATH, REGEX} filterType = EXTENSION;
        /**
         * For EXTENSION filtering: the file extension (the part behind the last '.')
         */
        std::string extensionFilter;
        /**
         * For PATH filtering: A vector containing all directory names under the web root
         * (e.g., ["dir1", "dir2"] for /dir1/dir2/).
         */
        std::vector<std::string> pathFilter;
        /**
         * For REGEX filtering: A std::regex matching the paths to be filtered (in the form "/dir1/dir2/file.ext").
         * Please note that this filtering is very resource-intensive and should only be used if the other filters
         * are not an option and the filtering goal cannot be achieved by combining another kind of filter with some
         * other application logic (that does not use regex). std::regex_match will be used for matching.
         */
        std::regex regexFilter;
        /**
         * The response that will be sent to the client if the request is not forwarded to the app (i.e., the request
         * is blocked, the file to forward is not found, or access has been denied).
         * If empty, a standard QSF error document will be sent.
         */
        std::string response;
    };

    /**
     * Defines a request blocking filter.
     */
    struct BlockFilter: public AccessFilter {
        /**
         * The HTTP status that will be sent to the client if the request is blocked.
         */
        unsigned int status = 403;
    };

    /**
     * Defines a filter that will request HTTP Basic Authentication if matching.
     */
    struct AuthFilter: public AccessFilter {
        /**
         * The authentication function. It will be called with the provided user name as first parameter and the
         * provided user password as the second parameter. Access will be granted if the authentication function
         * returns true.
         */
        std::function<bool(std::string, std::string)> authFunction;
    };

    /**
     * Filter to bypass the QSF app for certain requests and directly forward files to the client. This is especially
     * useful for media files, e.g. images, as well as CSS stylesheets.
     */
    struct ForwardFilter: public AccessFilter {
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
        enum BasePathExtension {BY_FILENAME, BY_PATH} basePathExtension = BY_PATH;
    };

    /**
     * Structure in which all filters that should be applied by the RequestHandler can be included. The filters will
     * be processed from the first element in a vector to the last element, block filters first, then auth filters,
     * then forward filters. If one filter leads to a block/forward/denied access, all following filters will be ignored.
     */
    struct AccessFilters {
        bool filtersEnabled = false; /**< Is the filter module enabled? If false, no filters will be applied. */
        std::vector<BlockFilter> blockFilters; /**< List of BlockFilter objects to be applied. */
        std::vector<AuthFilter> authFilters; /**< List of AuthFilter objects to be applied. */
        std::vector<ForwardFilter> forwardFilters; /**< List of ForwardFilter objects to be applied. */
    };
}

#endif //QSF_ACCESSRULES_H
