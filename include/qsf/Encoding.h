/**
 * \file Encoding.h
 * \brief Namespace containing functions for text encoding and decoding
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

#ifndef QSF_ENCODING_H
#define QSF_ENCODING_H

#include <string>

namespace Qsf {
    /**
     * Namespace containing functions for text encoding and decoding
     */
    namespace Encoding {
        /**
         * Escape special HTML characters in the input string.
         * @param input String with characters that need to be escaped for use in HTML.
         * @param encodeAll If false (by default), only the characters &, ", <, and > will be escaped.
         * If true, all characters that have an HTML entity will be encoded (less efficient and usually not necessary).
         * Please note that if encodeAll is set to true, the input string must be utf-8 encoded for correct results.
         * @return String with escaped characters.
         */
        std::string htmlEncode(std::string input, bool encodeAll = false);
        /**
         * Decode a string with HTML entities by replacing them with the corresponding utf-8 characters.
         * Only utf-8 (or ascii) encoded strings are supported by this function.\n
         * Only entities starting with & and ending in ; will be replaced, including the unicode notations.
         * If the semicolon is missing, the entity will not be replaced.
         * @param input Input string containing HTML entities.
         * @return String in which the entities are decoded to utf-8 characters.
         */
        std::string htmlDecode(std::string input);
        /**
         * Percent-encode a string for inclusion in a URL. Works with any byte-wise encoding, but only utf-8 makes
         * sense for use in URLs.
         * @param input String with characters that need to be escaped for use in a URL.
         * @return String with escaped characters.
         */
        std::string urlEncode(std::string input);
        /**
         * Decode a percent-encoded string (byte-wise, in URLs always utf-8).
         * @param input Percent-encoded string.
         * @return Decoded string.
         */
        std::string urlDecode(std::string input);
    }
}

#endif //QSF_ENCODING_H
