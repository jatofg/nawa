//
// Created by tobias on 09/11/18.
//

#ifndef QSF_ENCODING_H
#define QSF_ENCODING_H

#include <string>
#include <unordered_map>

namespace Qsf {
    /**
     * Namespace containing functions for text encoding and decoding
     */
    namespace Encoding {
        /**
         * Escape special HTML characters in the input string. Only utf-8 strings are supported by this function.
         * @param input String with characters that need to be escaped for use in HTML.
         * @param encodeAll If false (by default), only the characters &, ", <, and > will be escaped.
         * If true, all characters that have an HTML entity will be encoded (this is less efficient, prefer to use
         * utf-8 encoding consistently to avoid encoding problems and only escape the required characters).
         * @return String with escaped characters.
         */
        std::string htmlEncode(std::string input, bool encodeAll = false);
        /**
         * Decode a string with HTML entities by replacing them with the corresponding utf-8 characters.
         * Only utf-8 strings are supported by this function.\n
         * Only entities starting with & and ending in ; will be replaced, including the unicode notations.
         * If the semicolon is missing, the entity will not be replaced.
         * @param input Input string containing HTML entities.
         * @return String in which the entities are decoded to utf-8 characters.
         */
        std::string htmlDecode(std::string input);
        // TODO url encoding, base64 encoding, ...
    }
}

#endif //QSF_ENCODING_H
