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
 * \file encoding.h
 * \brief Namespace containing functions for text encoding and decoding
 */

#ifndef NAWA_ENCODING_H
#define NAWA_ENCODING_H

#include <string>

/**
 * Namespace containing functions for text encoding and decoding
 */
namespace nawa::encoding {
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
    std::string urlEncode(std::string const& input);

    /**
     * Decode a percent-encoded string (byte-wise, in URLs always utf-8).
     * @param input Percent-encoded string.
     * @return Decoded string.
     */
    std::string urlDecode(std::string input);

    /**
     * Check if a string contains only valid base64 characters and could be valid base64.
     * @param input Input string.
     * @param allowWhitespaces If true, the whitespace characters '\\r', '\\n', '\\t', and ' ' will be allowed.
     * @return True if the string contains only valid base64 characters, false otherwise.
     */
    bool isBase64(std::string const& input, bool allowWhitespaces = true);

    /**
     * Encode a string (of bytes) as base64.
     * @param input Input string.
     * @param breakAfter Maximum line length. The breakSequence will be inserted after the given number of base64
     * characters. If breakAfter is 0 (default), no line breaks will be inserted by this function.
     * @param breakSequence The characters to use as a line break. (e.g., "\\n" or "\\r\\n").
     * @return Base64-encoded string.
     */
    std::string base64Encode(std::string const& input, size_t breakAfter = 0, std::string const& breakSequence = "");

    /**
     * Decode a base64-encoded string (of bytes). If the input string does not (only) contain valid base64, the
     * return string may be undefined garbage. Exception: The characters '\\r', '\\n', '\\t', and ' ' (whitespaces)
     * will be ignored. You may use isBase64 to check it with a regex first, if the result matters.
     * @param input Base64 representation of a string.
     * @return The decoded string.
     */
    std::string base64Decode(std::string const& input);

    /**
     * Encode a string using quoted-printable encoding (especially useful for emails).
     * @param input String to be encoded.
     * @param lineEnding Character(s) to end the line. Default is "\\r\\n", as this is the default for emails.
     * @param replaceCrlf Whether to encode the '\\r' and '\\n' characters in the string. Defaults to false.
     * @param qEncoding Use the similar Q-encoding instead of quoted-printable. Line breaks will not be used, no
     * limit on line length will be applied.
     * @return The encoded string.
     */
    std::string quotedPrintableEncode(std::string const& input, std::string const& lineEnding = "\r\n",
                                      bool replaceCrlf = false, bool qEncoding = false);

    /**
     * Decode a quoted-printable encoded string. Please note that only the line endings "\\r\\n" and "\\n" are
     * supported.
     * @param input Encoded string.
     * @param qEncoding Whether the string is Q-encoded.
     * @return Decoded string.
     */
    std::string quotedPrintableDecode(std::string input, bool qEncoding = false);

    /**
     * Convert a UTF8-encoded string to the encoded-word syntax for email headers.
     * @param input The input string to encode.
     * @param base64 Whether to use base64-encoding instead of Q-encoding (defaults to false).
     * @param onlyIfNecessary Only apply q-encoding if special characters, which would be encoded, are present in the
     * input (does not apply to base64 encoding).
     * @return Encoded string.
     */
    std::string makeEncodedWord(std::string const& input, bool base64 = false, bool onlyIfNecessary = true);

    /**
     * Encode a domain in punycode, if it contains non-ASCII characters (otherwise it will be returned as-is). The
     * return value of this function is undefined for invalid domain names.
     * @param input Domain to be encoded.
     * @return Encoded domain.
     */
    std::string punycodeEncode(std::string const& input);

    /**
     * Decode a punycode-encoded domain. If it is not encoded, it will be returned as-is. The return value of this
     * function is undefined for invalid domain names.
     * @param input Encoded domain.
     * @return Decoded domain.
     */
    std::string punycodeDecode(std::string const& input);
}// namespace nawa::encoding

#endif//NAWA_ENCODING_H
