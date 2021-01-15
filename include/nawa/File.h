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

#include <string>
#include <memory>
#include <fstream>

namespace nawa {
    /**
     * Container for (especially POST-submitted) files.
     */
    struct File {
        std::string filename; /**< Original file name (submitted by sender) */
        std::string contentType; /**< Content-Type string */
        size_t size; /**< File size in bytes */
        std::shared_ptr<char[]> dataPtr; /**< Pointer to the first byte of the memory area */

        /**
         * Copy the file into a std::string
         * @return std::string containing the whole file
         */
        [[nodiscard]] std::string copyFile() const {
            return std::string(dataPtr.get(), size);
        }

        /**
         * Write the file to disk.
         * @param path File name and path where to write the file.
         * @return true on success, false on failure
         */
        bool writeFile(const std::string &path) const {
            std::ofstream outfile;
            std::ios_base::iostate exceptionMask = outfile.exceptions() | std::ios::failbit;
            outfile.exceptions(exceptionMask);
            try {
                outfile.open(path, std::ofstream::out | std::ofstream::binary);
                outfile.write(dataPtr.get(), size);
                outfile.close();
            }
            catch (std::ios_base::failure &e) {
                return false;
            }
            return true;
        }
    };
}

#endif //NAWA_FILE_H
