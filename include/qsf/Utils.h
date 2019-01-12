//
// Created by tobias on 14/11/18.
//

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
}

#endif //QSF_UTILS_H
