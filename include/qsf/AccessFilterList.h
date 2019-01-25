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

    // TODO auth filters requesting the credentials on every request do not make sense
    //  --> possibly http auth should better be implemented somewhere else?
    //  (however, forwarded requests cannot be authenticated then)

    /**
     * Defines a filter on the request path. Path filtering will be done first, then extension filtering, then regex
     * filtering. In order to match, all enabled conditions must match (AND connector, use multiple filters for OR).
     * Filters which are left empty will "match everything". A filter with no conditions will match everything.
     */
    struct AccessFilter {
        /**
         * For path filtering: A vector containing all directory names under the web root
         * (e.g., ["dir1", "dir2"] for /dir1/dir2/). If empty, path filtering will not be applied.
         */
        std::vector<std::string> pathFilter;
        /**
         * For extension filtering: the file extension (the part behind the last '.'). If empty, extension filtering
         * will not be applied.
         */
        std::string extensionFilter;
        /**
         * Regex filtering will be applied iff this value is set to true.\n
         * Please note that this filtering is very resource-intensive. Use it only if you cannot achieve your goal by
         * using (possibly a combination of) the other filters. It makes sense to define a path and/or extension filter
         * additionally, so that the regex is only checked if the path and/or extension preconditions are met.
         */
        bool regexFilterEnabled = false;
        /**
         * For regex filtering: A std::regex matching the paths to be filtered (in the form "/dir1/dir2/file.ext").
         * std::regex_match will be used for matching. If regexFilterEnabled is set to false, this value will be
         * ignored. If regexFilterEnabled is set to true and the regex is not assigned, nothing will be matched.
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
     * Defines a filter that will request HTTP Basic Authentication if matching. This filter type is not implemented
     * yet and possibly it also won't be implemented in future.
     */
    struct AuthFilter: public AccessFilter {
        /**
         * The authentication function. It will be called with the provided user name as first parameter and the
         * provided user password as the second parameter. Access will be granted if the authentication function
         * returns true.
         */
        std::function<bool(std::string, std::string)> authFunction;
        /**
         * Use sessions to remember the authenticated user. This will create a session variable "_qsf_staticAuth_user".
         * You can use it in your application to find out which user has authenticated and delete it to log the user out.
         * If disabled, the user will have to authenticate on every single request.
         */
        bool useSessions = true;
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
        enum BasePathExtension {BY_FILENAME, BY_PATH} basePathExtension = BY_FILENAME;
    };

    /**
     * Structure in which all filters that should be applied by the RequestHandler can be included. The filters will
     * be processed from the first element in a vector to the last element, block filters first, then auth filters,
     * then forward filters. If one filter leads to a block/forward/denied access, all following filters will be ignored.
     */
    struct AccessFilterList {
        bool filtersEnabled = false; /**< Is the filter module enabled? If false, no filters will be applied. */
        std::vector<BlockFilter> blockFilters; /**< List of BlockFilter objects to be applied. */
        std::vector<AuthFilter> authFilters; /**< List of AuthFilter objects to be applied. */
        std::vector<ForwardFilter> forwardFilters; /**< List of ForwardFilter objects to be applied. */
    };
}

#endif //QSF_ACCESSRULES_H
