/**
 * \file File.h
 * \brief Container for (especially POST-submitted) files.
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

#ifndef NAWA_FILE_H
#define NAWA_FILE_H

#include <fstream>
#include <memory>
#include <nawa/internal/macros.h>
#include <string>

namespace nawa {
    /**
     * Container for (especially POST-submitted) files.
     */
    class File {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(File);

        /**
         * Construct a File from data.
         * @param dataPtr Shared pointer to the file data.
         * @param size Size of the file.
         * @deprecated Files should be constructed from std::string objects instead.
         */
        File(std::shared_ptr<char[]> dataPtr, size_t size);

        /**
         * Construct a file from data stored in a string.
         * @param data File data.
         */
        explicit File(const std::string &data);

        /**
         * Copy the File object which contains a reference to a file. Please note that this does *not* deep-copy the
         * file in memory, just the reference!
         * @param other File object to copy (the refence) from.
         */
        NAWA_COPY_CONSTRUCTOR_DEF(File);

        /**
         * Copy-assign from another File object. Please note that this does *not* deep-copy the file in memory, just
         * the reference!
         * @param other File object to copy (the refence) from.
         * @return This File object.
         */
        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(File);

        NAWA_MOVE_CONSTRUCTOR_DEF(File);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(File);

        /**
         * Original file name (submitted by sender).
         * @return Reference to file name.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(File, filename, std::string);

        /**
         * Content-Type string.
         * @return Reference to content-type string.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(File, contentType, std::string);

        /**
         * Get file size in bytes.
         * @return File size.
         */
        [[nodiscard]] size_t size() const noexcept;

        /**
         * Copy the file into a std::string
         * @return std::string containing the whole file
         */
        [[nodiscard]] std::string toString() const;

        /**
         * Write the file to disk. Throws a nawa::Excption with error code 1 on failure.
         * @param path File name and path where to write the file.
         */
        void writeToDisk(const std::string &path) const;
    };
}

#endif //NAWA_FILE_H
