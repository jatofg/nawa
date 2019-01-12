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
        /**
         * Create a (hopefully) secure password hash using the bcrypt hash algorithm. \n
         * This function returns one-way hashes with pseudo-random salts. Use passwordVerify to validate a password.\n
         * May throw a UserException on failure. Error codes: 1 (empty input password), 2 (unknown bcrypt failure),
         * 3 (invalid cost factor).\n
         * Argon2 hashing might be added later, probably in a separate function.
         * @param password The password to hash.
         * @param cost The cost factor determines how much effort is needed in order to generate the salt.
         * A higher cost factor increases security of the hash, but it will also slow down the hash generation.\n
         * Clearly, the cost factor must be >0.
         * @return Hash of the password (60 characters).
         */
        std::string passwordHash(const std::string& password, int cost = 12);
        /**
         * Validate a password hashed using passwordHash with the same algorithm (currently always bcrypt).
         * This function (more precisely, the underlying libbcrypt function) is designed in a way that timing attacks
         * should not be possible, given that the compiler did no bad optimizations. Only in case the input password is
         * empty, this function will immediately return false.\n
         * May throw a UserException on failure. Error codes: 1 (empty input hash), 2 (unknown bcrypt failure).
         * @param password Password (user input) to be verified.
         * @param hash Hash (e.g., from a database) to verify the user password against.
         * @return True if the password matches, false otherwise.
         */
        bool passwordVerify(const std::string& password, const std::string& hash);
    };
}

#endif //QSF_CRYPTO_H
