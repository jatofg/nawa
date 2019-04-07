/*
   base64.cpp and base64.h

   base64 encoding and decoding with C++.

   This is a modified version of:
   https://github.com/ReneNyffenegger/cpp-base64

   Copyright (original version): (C) 2004-2017 René Nyffenegger
   Modifications (C) 2019 Jan Flaig

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

*/

#include "base64.h"
#include <sstream>

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static inline bool is_whitespace(unsigned char c) {
    return (c == '\r' || c == '\n' || c == '\t' || c == ' ');
}

std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, size_t break_after, const std::string& break_seq) {
    std::stringstream ret;
    int i = 0;
    int j = 0;
    size_t break_count = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = static_cast<unsigned char>((char_array_3[0] & 0xfc) >> 2);
            char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
            char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));
            char_array_4[3] = static_cast<unsigned char>(char_array_3[2] & 0x3f);

            for(i = 0; (i <4) ; i++) {
                if(break_after > 0 && break_count == break_after) {
                    ret << break_seq;
                    break_count = 0;
                }
                ret << base64_chars[char_array_4[i]];
                ++break_count;
            }
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = static_cast<unsigned char>(( char_array_3[0] & 0xfc) >> 2);
        char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
        char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));

        for (j = 0; (j < i + 1); j++) {
            if(break_after > 0 && break_count == break_after) {
                ret << break_seq;
                break_count = 0;
            }
            ret << base64_chars[char_array_4[j]];
            ++break_count;
        }

        while((i++ < 3)) {
            if(break_after > 0 && break_count == break_after) {
                ret << break_seq;
                break_count = 0;
            }
            ret << '=';
            ++break_count;
        }

    }

    return ret.str();

}

std::string base64_decode(std::string const& encoded_string) {
    auto encoded_string_u = reinterpret_cast<unsigned const char*>(encoded_string.c_str());
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::stringstream ret;

    while (in_len-- && (is_base64(encoded_string_u[in_]) || is_whitespace(encoded_string_u[in_]))) {
        if(is_whitespace(encoded_string_u[in_])) continue;
        char_array_4[i++] = encoded_string_u[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] = static_cast<unsigned char>(( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4));
            char_array_3[1] = static_cast<unsigned char>(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
            char_array_3[2] = static_cast<unsigned char>(((char_array_4[2] & 0x3) << 6) +   char_array_4[3]);

            for (i = 0; (i < 3); i++)
                ret << char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = static_cast<unsigned char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
        char_array_3[1] = static_cast<unsigned char>(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));

        for (j = 0; (j < i - 1); j++) ret << char_array_3[j];
    }

    return ret.str();
}
