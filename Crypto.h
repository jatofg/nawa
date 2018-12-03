//
// Created by tobias on 03/12/18.
//

#ifndef QSF_CRYPTO_H
#define QSF_CRYPTO_H

#include <string>

namespace Qsf {
    namespace Crypto {
        /**
         * Get SHA-1 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation (40 chars). Otherwise, the hash will
         * be binary (20 bytes).
         * @return SHA-1 hash of input.
         */
        std::string sha1(const std::string& input, bool hex = true);
        /**
         * Get SHA-224 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
         * be binary (28 bytes).
         * @return Hash of input.
         */
        std::string sha224(const std::string &input, bool hex = true);
        /**
         * Get SHA-256 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
         * be binary (32 bytes).
         * @return Hash of input.
         */
        std::string sha256(const std::string &input, bool hex = true);
        /**
         * Get SHA-384 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
         * be binary (48 bytes).
         * @return Hash of input.
         */
        std::string sha384(const std::string &input, bool hex = true);
        /**
         * Get SHA-512 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation. Otherwise, the hash will
         * be binary (64 bytes).
         * @return Hash of input.
         */
        std::string sha512(const std::string &input, bool hex = true);
        /**
         * Get MD5 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation (32 chars). Otherwise, the hash will
         * be binary (16 bytes).
         * @return Hash of input.
         */
        std::string md5(const std::string &input, bool hex = true);
    };
}

#endif //QSF_CRYPTO_H
