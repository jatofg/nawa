/**
 * \file Utils.h
 * \brief Contains useful functions that improve the readability and facilitate maintenance of the QSF code.
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

#ifndef QSF_UTILS_H
#define QSF_UTILS_H

#include <string>
#include <regex>

namespace Qsf {
    /**
     * ASCII (byte-)string based regex_replace variant which takes a callback function as format parameter. This
     * function may be used to generate individual replacements for each match.
     * @param s String in which the replacements should take place.
     * @param rgx std::regex object containing the regular expression to use for replacement.
     * @param fmt Callback function taking a reference to a string vector as a parameter and returning the replacement
     * as a string. The vector contains the full match at index 0 and, if capturing groups are used in the expression,
     * the submatches at the following indexes (capturing group 1 at index 1, ...)
     */
    void regex_replace_callback(std::string& s, const std::regex& rgx, std::function<std::string(const std::vector<std::string>&)> fmt);
    /**
     * Apply hex encoding to every byte in a string of bytes.
     * @param in Byte string that should be converted.
     * @return String containing the hexadecimal representation of every byte (thus, twice as long as in).
     */
    std::string hex_dump(const std::string& in);
    /**
     * Convert every ASCII letter in the given string to lowercase.
     * @param s String to convert.
     * @return Lowercase string.
     */
    std::string to_lowercase(std::string s);
    /**
     * Convert every ASCII letter in the given string to uppercase.
     * @param s String to convert.
     * @return Uppercase string.
     */
    std::string to_uppercase(std::string s);
    /**
     * Generate a very basic HTML error page for a given HTTP status.
     * @param httpStatus The HTTP status. Currently supported: 400, 401, 403, 404, 405, 406, 408, 409, 410, 415, 418,
     * 429, 451, 500, 501, 503.
     * @return An HTML document containing a simple error page. If the given status code is unknown, it will be
     * titled "Unknown error".
     */
    std::string generate_error_page(unsigned int httpStatus);
}

#endif //QSF_UTILS_H
