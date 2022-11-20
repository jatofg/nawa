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
 * \file utils.h
 * \brief Contains useful functions that improve the readability and facilitate maintenance of the NAWA code.
 */

#ifndef NAWA_UTILS_H
#define NAWA_UTILS_H

#include <regex>
#include <string>
#include <unordered_map>

namespace nawa::utils {
    /**
     * ASCII (byte-)string based regex_replace variant which takes a callback function as format parameter. This
     * function may be used to generate individual replacements for each match.
     * @param s String in which the replacements should take place.
     * @param rgx std::regex object containing the regular expression to use for replacement.
     * @param fmt Callback function taking a reference to a string vector as a parameter and returning the replacement
     * as a string. The vector contains the full match at index 0 and, if capturing groups are used in the expression,
     * the submatches at the following indexes (capturing group 1 at index 1, ...)
     */
    void regexReplaceCallback(std::string& s, std::regex const& rgx,
                              std::function<std::string(std::vector<std::string> const&)> const& fmt);

    /**
     * Apply hex encoding to every byte in a string of bytes.
     * @param in Byte string that should be converted.
     * @return String containing the hexadecimal representation of every byte (thus, twice as long as in).
     */
    std::string hexDump(std::string const& in);

    /**
     * Convert every ASCII letter in the given string to lowercase.
     * @param s String to convert.
     * @return Lowercase string.
     */
    std::string toLowercase(std::string s);

    /**
     * Convert every ASCII letter in the given string to uppercase.
     * @param s String to convert.
     * @return Uppercase string.
     */
    std::string toUppercase(std::string s);

    /**
     * Generate a very basic HTML error page for a given HTTP status.
     * @param httpStatus The HTTP status. Currently supported: 400, 401, 403, 404, 405, 406, 408, 409, 410, 415, 418,
     * 429, 451, 500, 501, 503.
     * @return An HTML document containing a simple error page. If the given status code is unknown, it will be
     * titled "Unknown error".
     */
    std::string generateErrorPage(unsigned int httpStatus);

    /**
     * Get the extension (that is, everything after the last '.') out of a filename. Swallows out_of_range exceptions.
     * @param filename The filename.
     * @return String containing the extension, or an empty string if no extension has been found.
     */
    std::string getFileExtension(std::string const& filename);

    /**
     * Get the MIME content type string for a particular file extension. This function checks the extension against a
     * limited, incomplete set of file types (case-insensitive). If no match is found, "application/octet-stream"
     * will be returned.
     * @param extension The file extension (without '.').
     * @return A MIME content type string.
     */
    std::string contentTypeByExtension(std::string extension);

    /**
     * Convert a time_t value (UNIX timestamp) to a HTTP header compatible date/time string. May throw an Exception
     * with error code 1 if interpretation of the UNIX timestamp fails.
     * @param time UNIX timestamp as a time_t value.
     * @return Time string in the format
     * "<day-name(3)>, <day(2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> GMT".
     */
    std::string makeHttpTime(time_t time);

    /**
     * Create a time_t value (UNIX timestamp) from a HTTP header date/time string. May throw an Exception with
     * error code 1 if parsing fails.
     * @param httpTime Time string in the format:
     * "<day-name(3)>, <day(2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> GMT".
     * @return UNIX timestamp value (time_t).
     */
    time_t readHttpTime(std::string const& httpTime);

    /**
     * Convert a time_t value (UNIX timestamp) to a SMTP header compatible date/time string. May throw an Exception
     * with error code 1 if interpretation of the UNIX timestamp fails.
     * @param time UNIX timestamp as a time_t value.
     * @return Time string in the format
     * "<day-name(3)>, <day(1*2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> <tzoffset(5)>".
     */
    std::string makeSmtpTime(time_t time);

    /**
     * Create a time_t value (UNIX timestamp) from a SMTP header date/time string. May throw an Exception with
     * error code 1 if parsing fails.
     * @param smtpTime Time string in the format:
     * "<day-name(3)>, <day(1*2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> <tzoffset(5)>".
     * @return UNIX timestamp value (time_t).
     */
    time_t readSmtpTime(std::string const& smtpTime);

    /**
     * Split a string using a character as a delimiter.
     * @param str String to split.
     * @param delimiter Delimiter.
     * @param ignoreEmpty Ignore empty tokens, i.e., do not add them to the vector.
     * @return Vector containing the elements.
     */
    std::vector<std::string> splitString(std::string str, char delimiter, bool ignoreEmpty = false);

    /**
     * Convert a vector representation of a path ({"dir1", "dir2"}) to a string representation ("/dir1/dir2").
     * An empty vector will result in the string representation "/".
     * @param path Vector representation of a path.
     * @return String representation of the given path.
     */
    std::string mergePath(std::vector<std::string> const& path);

    /**
     * Convert a string representation of a path ("/dir1/dir2") to a vector representation ({"dir1", "dir2"}).
     * Query strings (starting with '?') are being ignored and will not be part of the returned vector.
     * @param pathString Sting representation of the path.
     * @return Vector representation of the given path.
     */
    std::vector<std::string> splitPath(std::string const& pathString);

    /**
     * Convert line endings in a string between LF (Unix) and CRLF (Windows/Email).
     * @param in Input string with CRLF or LF line endings
     * @param ending The new line ending (e.g., "\\n" or "\\r\\n")
     * @return String with converted line endings.
     */
    std::string convertLineEndings(std::string const& in, std::string const& ending);

    /**
     * Simple function to read a whole file into a string. Will throw a nawa::Exception with error code 1 if the file
     * does not exist.
     * @param path Path and filename.
     * @return String containing the whole file.
     */
    std::string getFileContents(std::string const& path);

    /**
     * Replace all occurrences of pattern keys by the corresponding pattern values in the input string.
     * @param input String in which the patterns shall be replaced.
     * @param patterns Map of key characters (to be replaced in the input string) and value characters (the replacements).
     * @return String with replacements applied.
     */
    std::string stringReplace(std::string input, std::unordered_map<char, char> const& patterns);

    /**
     * Replace all occurrences of pattern keys by the corresponding pattern values in the input string.
     * @param input String in which the patterns shall be replaced.
     * @param patterns Map of key strings (to be replaced in the input string) and value strings (the replacements).
     * @return String with replacements applied.
     */
    std::string stringReplace(std::string input, std::unordered_map<std::string, std::string> const& patterns);

    /**
     * Generate a GET key => value multimap out of a query string (the ?k1=v1&k2=v2... part of a URL).
     * @param queryString Query string or URL containing a query string.
     * @return Key => value map containing the GET variables.
     */
    std::unordered_multimap<std::string, std::string> splitQueryString(std::string const& queryString);

    /**
     * Parse a block of headers into a map.
     * @param rawHeaders The raw block of headers.
     * @return Map with key => value mapping (keys will be transformed to lowercase).
     */
    std::unordered_map<std::string, std::string> parseHeaders(std::string rawHeaders);

    /**
     * Parse cookies sent by the browser.
     * @param rawCookies The content of the "Cookie" header.
     * @return A multimap of the cookies.
     */
    std::unordered_multimap<std::string, std::string> parseCookies(std::string const& rawCookies);

    /**
     * Convert any iterable map to an unordered_multimap.
     * @tparam KeyType Key type (automatically deduced).
     * @tparam ValueType Value type (automatically deduced).
     * @tparam MapType Input map type (automatically deduced).
     * @tparam Args Further template arguments of map type (automatically deduced).
     * @param inputMap The input map to convert.
     * @return An unordered_multimap with the input map's content.
     */
    template<typename KeyType, typename ValueType, template<typename, typename, typename...> class MapType, typename... Args>
    std::unordered_multimap<KeyType, ValueType> toUnorderedMultimap(MapType<KeyType, ValueType, Args...> inputMap) {
        std::unordered_multimap<KeyType, ValueType> ret;
        for (auto const& [k, v] : inputMap) {
            ret.insert({k, v});
        }
        return ret;
    }
}// namespace nawa::utils

#endif//NAWA_UTILS_H
