//
// Created by tobias on 03/12/18.
//

#ifndef QSF_CRYPTO_H
#define QSF_CRYPTO_H

#include <string>

namespace Qsf {
    namespace Crypto {
        // TODO more functions
        /**
         * Get SHA1 hash of a string.
         * @param input String to hash.
         * @param hex If true (default), get hash in hexadecimal representation (40 chars). Otherwise, the hash will
         * be binary (20 bytes).
         * @return SHA1 hash of input.
         */
        std::string sha1(const std::string& input, bool hex = true);
    };
}

#endif //QSF_CRYPTO_H
