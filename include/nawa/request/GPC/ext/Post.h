/**
 * \file Post.h
 * \brief Post.h
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

#ifndef NAWA_POST_H
#define NAWA_POST_H

#include <memory>
#include <nawa/request/File.h>
#include <nawa/request/GPC/GPC.h>

namespace nawa::request {
    /**
     * Specialized accessor for POST that also allows accessing files (and in future, maybe, the raw POST data).
     */
    class Post : public GPC {
        std::string contentType;
        std::shared_ptr<std::string> rawPost;
        std::unordered_multimap<std::string, File> fileMap;
    public:
        explicit Post(const RequestInitContainer &requestInit);

        ~Post() override = default;

        /**
         * Shortcut to check for the existence of POST values (including files).
         * @return True if POST values are available.
         */
        explicit operator bool() const override;

        /**
         * Get the raw POST data (availability may depend on the raw_access setting in the config).
         * @return Shared pointer to a string containing the raw POST data if available, otherwise the
         * shared_ptr does not contain an object.
         */
        [[nodiscard]] std::shared_ptr<std::string> getRaw() const;

        /**
         * Get the POST content type as submitted by the browser
         * @return String containing the POST content type
         */
        [[nodiscard]] std::string getContentType() const;

        /**
         * Check whether files have been uploaded via POST.
         * @return True if files have been uploaded via POST.
         */
        [[nodiscard]] bool hasFiles() const;

        /**
         * Get a file submitted via POST. If the query contains more than one POST file of the same name,
         * only one of them (usually the first definition) will be returned. For accessing all definitions,
         * please use getFileVector(). Complexity is logarithmic, so if you want to access a value multiple times,
         * saving it in a variable is a good idea.
         * @param postVar Name of the variable.
         * @return Value of the variable. Empty string if not set
         * (use countFiles() for checking whether the variable is set).
         */
        [[nodiscard]] File getFile(const std::string &postVar) const;

        /**
         * Get all POST files with the given name.
         * @param postVar Name of the files.
         * @return Vector of files. Empty if no file with the given name exists.
         */
        [[nodiscard]] std::vector<File> getFileVector(const std::string &postVar) const;

        /**
         * Get the number of submitted POST files with the given name.
         * @param postVar Name of the file.
         * @return Number of occurrences.
         */
        [[nodiscard]] size_t countFiles(const std::string &postVar) const;

        /**
         * Get a reference to the POST file multimap.
         * @return Reference to the multimap.
         */
        [[nodiscard]] std::unordered_multimap<std::string, File> const &getFileMultimap() const;
    };
}

#endif //NAWA_POST_H