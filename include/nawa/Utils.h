/**
 * \file Utils.h
 * \brief Contains useful functions that improve the readability and facilitate maintenance of the NAWA code.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#ifndef NAWA_UTILS_H
#define NAWA_UTILS_H

#include <string>
#include <regex>
#include <unordered_map>

namespace nawa {
    /**
     * ASCII (byte-)string based regex_replace variant which takes a callback function as format parameter. This
     * function may be used to generate individual replacements for each match.
     * @param s String in which the replacements should take place.
     * @param rgx std::regex object containing the regular expression to use for replacement.
     * @param fmt Callback function taking a reference to a string vector as a parameter and returning the replacement
     * as a string. The vector contains the full match at index 0 and, if capturing groups are used in the expression,
     * the submatches at the following indexes (capturing group 1 at index 1, ...)
     */
    void regex_replace_callback(std::string &s, const std::regex &rgx,
                                const std::function<std::string(const std::vector<std::string> &)> &fmt);

    /**
     * Apply hex encoding to every byte in a string of bytes.
     * @param in Byte string that should be converted.
     * @return String containing the hexadecimal representation of every byte (thus, twice as long as in).
     */
    std::string hex_dump(const std::string &in);

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

    /**
     * Get the extension (that is, everything after the last '.') out of a filename. Swallows out_of_range exceptions.
     * @param filename The filename.
     * @return String containing the extension, or an empty string if no extension has been found.
     */
    std::string get_file_extension(const std::string &filename);

    /**
     * Get the MIME content type string for a particular file extension. This function checks the extension against a
     * limited, incomplete set of file types (case-insensitive). If no match is found, "application/octet-stream"
     * will be returned.
     * @param extension The file extension (without '.').
     * @return A MIME content type string.
     */
    std::string content_type_by_extension(std::string extension);

    /**
     * Convert a time_t value (UNIX timestamp) to a HTTP header compatible date/time string.
     * @param time UNIX timestamp as a time_t value.
     * @return Time string in the format
     * "<day-name(3)>, <day(2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> GMT".
     */
    std::string make_http_time(time_t time);

    /**
     * Create a time_t value (UNIX timestamp) from a HTTP header date/time string.
     * @param httpTime Time string in the format:
     * "<day-name(3)>, <day(2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> GMT".
     * @return UNIX timestamp value (time_t).
     */
    time_t read_http_time(const std::string &httpTime);

    /**
     * Convert a time_t value (UNIX timestamp) to a SMTP header compatible date/time string.
     * @param time UNIX timestamp as a time_t value.
     * @return Time string in the format
     * "<day-name(3)>, <day(1*2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> <tzoffset(5)>".
     */
    std::string make_smtp_time(time_t time);

    /**
     * Create a time_t value (UNIX timestamp) from a SMTP header date/time string.
     * @param smtpTime Time string in the format:
     * "<day-name(3)>, <day(1*2)> <month(3)> <year(4)> <hour(2)>:<minute(2)>:<second(2)> <tzoffset(5)>".
     * @return UNIX timestamp value (time_t).
     */
    time_t read_smtp_time(const std::string &smtpTime);

    /**
     * Split a string using a character as a delimiter.
     * @param str String to split.
     * @param delimiter Delimiter.
     * @param ignoreEmpty Ignore empty tokens, i.e., do not add them to the vector.
     * @return Vector containing the elements.
     */
    std::vector<std::string> split_string(std::string str, char delimiter, bool ignoreEmpty = false);

    /**
     * Convert a vector representation of a path ({"dir1", "dir2"}) to a string representation ("/dir1/dir2").
     * An empty vector will result in the string representation "/".
     * @param path Vector representation of a path.
     * @return String representation of the given path.
     */
    std::string merge_path(const std::vector<std::string> &path);

    /**
     * Convert a string representation of a path ("/dir1/dir2") to a vector representation ({"dir1", "dir2"}).
     * Query strings (starting with '?') are being ignored and will not be part of the returned vector.
     * @param pathString Sting representation of the path.
     * @return Vector representation of the given path.
     */
    std::vector<std::string> split_path(const std::string &pathString);

    /**
     * Convert line endings in a string between LF (Unix) and CRLF (Windows/Email).
     * @param in Input string with CRLF or LF line endings
     * @param ending The new line ending (e.g., "\\n" or "\\r\\n")
     * @return String with converted line endings.
     */
    std::string convert_line_endings(const std::string &in, const std::string &ending);

    /**
     * Simple function to read a whole file into a string. Will throw a nawa::Exception with error code 1 if the file
     * does not exist.
     * @param path Path and filename.
     * @return String containing the whole file.
     */
    std::string get_file_contents(const std::string &path);

    /**
     * Replace all occurrences of pattern keys by the corresponding pattern values in the input string.
     * @param input String in which the patterns shall be replaced.
     * @param patterns Map of key characters (to be replaced in the input string) and value characters (the replacements).
     * @return String with replacements applied.
     */
    std::string string_replace(std::string input, const std::unordered_map<char, char> &patterns);

    /**
     * Replace all occurrences of pattern keys by the corresponding pattern values in the input string.
     * @param input String in which the patterns shall be replaced.
     * @param patterns Map of key strings (to be replaced in the input string) and value strings (the replacements).
     * @return String with replacements applied.
     */
    std::string string_replace(std::string input, const std::unordered_map<std::string, std::string> &patterns);

    /**
     * Generate a GET key => value multimap out of a query string (the ?k1=v1&k2=v2... part of a URL).
     * @param queryString Query string or URL containing a query string.
     * @return Key => value map containing the GET variables.
     */
    std::unordered_multimap<std::string, std::string> split_query_string(const std::string &queryString);

    /**
     * Parse a block of headers into a map.
     * @param rawHeaders The raw block of headers.
     * @return Map with key => value mapping (keys will be transformed to lowercase).
     */
    std::unordered_map<std::string, std::string> parse_headers(std::string rawHeaders);

    /**
     * Parse cookies sent by the browser.
     * @param rawCookies The content of the "Cookie" header.
     * @return A multimap of the cookies.
     */
     std::unordered_multimap<std::string, std::string> parse_cookies(const std::string &rawCookies);

    /**
     * Convert any iterable map to an unordered_map.
     * @tparam KeyType Key type (automatically deduced).
     * @tparam ValueType Value type (automatically deduced).
     * @tparam MapType Input map type (automatically deduced).
     * @tparam Args Further template arguments of map type (automatically deduced).
     * @param inputMap The input map to convert.
     * @return An unordered_map with the input map's content.
     */
    template<typename KeyType, typename ValueType, template<typename, typename, typename...> class MapType, typename... Args>
    std::unordered_multimap<KeyType, ValueType> to_unordered_multimap(MapType<KeyType, ValueType, Args...> inputMap) {
        std::unordered_multimap<KeyType, ValueType> ret;
        for (auto const &[k, v]: inputMap) {
            ret.insert({k, v});
        }
        return ret;
    }
}

#endif //NAWA_UTILS_H
