/**
 * \file AccessFilter.h
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

#ifndef NAWA_ACCESSFILTER_H
#define NAWA_ACCESSFILTER_H

#include <nawa/internal/macros.h>
#include <regex>
#include <string>
#include <vector>

namespace nawa {
    /**
     * Defines a filter on the request path. Path filtering will be done first, then extension filtering, then regex
     * filtering. In order to match, all enabled conditions must match (AND connector, use multiple filters for OR).
     * Filters which are left empty will "match everything". A filter with no conditions will match everything.
     */
    class AccessFilter {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(AccessFilter);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(AccessFilter);

        NAWA_COPY_CONSTRUCTOR_DEF(AccessFilter);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(AccessFilter);

        NAWA_MOVE_CONSTRUCTOR_DEF(AccessFilter);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(AccessFilter);

        /**
         * Negate the filter, i.e., the filter will match if none of the conditions apply (instead of all). This is
         * particularly useful, for example, for defining a BlockFilter with a set of allowed URLs and send a 404
         * error page for everything else, or to apply authentication to everything except for a path.
         * @return Reference to element.
         */
        bool &invert() noexcept;

        /**
         * Negate the filter, i.e., the filter will match if none of the conditions apply (instead of all). This is
         * particularly useful, for example, for defining a BlockFilter with a set of allowed URLs and send a 404
         * error page for everything else, or to apply authentication to everything except for a path.
         * @return The element.
         */
        [[nodiscard]] bool invert() const noexcept;

        /**
         * For path filtering: A vector containing all paths to match. Each path shall be provided as a vector of
         * strings containing all directory names under the web root
         * (e.g., {"dir1", "dir2"} for /dir1/dir2/). If empty, path filtering will not be applied.
         * @return Reference to element.
         */
        std::vector<std::vector<std::string>> &pathFilter() noexcept;

        /**
         * For path filtering: A vector containing all paths to match. Each path shall be provided as a vector of
         * strings containing all directory names under the web root
         * (e.g., {"dir1", "dir2"} for /dir1/dir2/). If empty, path filtering will not be applied.
         * @return Reference to element.
         */
        [[nodiscard]] std::vector<std::vector<std::string>> const &pathFilter() const noexcept;

        /**
         * Invert the path filtering condition, i.e., the condition applies when the request URI is *outside* of
         * the specified paths.
         * @return Reference to element.
         */
        bool &invertPathFilter() noexcept;

        /**
         * Invert the path filtering condition, i.e., the condition applies when the request URI is *outside* of
         * the specified paths.
         * @return The element.
         */
        [[nodiscard]] bool invertPathFilter() const noexcept;

        /**
         * For extension filtering: A list of file extensions (the part behind the last '.') to match.
         * If empty, extension filtering will not be applied.
         * @return Reference to element.
         */
        std::vector<std::string> &extensionFilter() noexcept;

        /**
         * For extension filtering: A list of file extensions (the part behind the last '.') to match.
         * If empty, extension filtering will not be applied.
         * @return Reference to element.
         */
        [[nodiscard]] std::vector<std::string> const &extensionFilter() const noexcept;

        /**
         * Invert the extension filtering condition, i.e., the condition applies when a file has an extension which
         * is not part of the given list of extensions.
         * @return Reference to element.
         */
        bool &invertExtensionFilter() noexcept;

        /**
         * Invert the extension filtering condition, i.e., the condition applies when a file has an extension which
         * is not part of the given list of extensions.
         * @return The element.
         */
        [[nodiscard]] bool invertExtensionFilter() const noexcept;

        /**
         * Regex filtering will be applied iff this value is set to true.\n
         * Please note that this filtering is very resource-intensive. Use it only if you cannot achieve your goal by
         * using (possibly a combination of) the other filters. It makes sense to define a path and/or extension filter
         * additionally, so that the regex is only checked if the path and/or extension preconditions are met.
         * @return Reference to element.
         */
        bool &regexFilterEnabled() noexcept;

        /**
         * Regex filtering will be applied iff this value is set to true.\n
         * Please note that this filtering is very resource-intensive. Use it only if you cannot achieve your goal by
         * using (possibly a combination of) the other filters. It makes sense to define a path and/or extension filter
         * additionally, so that the regex is only checked if the path and/or extension preconditions are met.
         * @return The element.
         */
        [[nodiscard]] bool regexFilterEnabled() const noexcept;

        /**
         * For regex filtering: A std::regex matching the paths to be filtered (in the form "/dir1/dir2/file.ext").
         * std::regex_match will be used for matching. If regexFilterEnabled is set to false, this value will be
         * ignored. If regexFilterEnabled is set to true and the regex is not assigned, nothing will be matched.
         * @return Reference to element.
         */
        std::regex &regexFilter() noexcept;

        /**
         * For regex filtering: A std::regex matching the paths to be filtered (in the form "/dir1/dir2/file.ext").
         * std::regex_match will be used for matching. If regexFilterEnabled is set to false, this value will be
         * ignored. If regexFilterEnabled is set to true and the regex is not assigned, nothing will be matched.
         * @return Reference to element.
         */
        [[nodiscard]] std::regex const &regexFilter() const noexcept;

        /**
         * The response that will be sent to the client if the request is not forwarded to the app (i.e., the request
         * is blocked, the file to forward is not found, or access has been denied).
         * If empty, a standard NAWA error document will be sent.
         * @return Reference to element.
         */
        std::string &response() noexcept;

        /**
         * The response that will be sent to the client if the request is not forwarded to the app (i.e., the request
         * is blocked, the file to forward is not found, or access has been denied).
         * If empty, a standard NAWA error document will be sent.
         * @return Reference to element.
         */
        [[nodiscard]] std::string const &response() const noexcept;

    };
}

#endif //NAWA_ACCESSFILTER_H
