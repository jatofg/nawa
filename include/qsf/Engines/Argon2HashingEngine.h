//
// Created by tobias on 30/04/19.
//

#ifndef QSF_ARGON2HASHINGENGINE_H
#define QSF_ARGON2HASHINGENGINE_H

#include <qsf/Engines/HashingEngine.h>

namespace Qsf {
    namespace Engines {

        class Argon2HashingEngine: public HashingEngine {
            enum Algorithm {
                ARGON2I,
                ARGON2D,
                ARGON2ID
            } algorithm; /**< The Argon2 flavor to use. */
            uint32_t timeCost; /**< Number of iterations. */
            uint32_t memoryCost; /**< Memory usage in kiB. */
            uint32_t parallelism; /**< Number of threads used. */
            std::string salt; /**< User-defined salt. */
            std::string secret; /**< Input for keyed hashing. */
        public:
            /**
             * Create a new Argon2 hash generator and set the parameters.
             * @param algorithm The Argon2 flavor to use.
             * @param timeCost Number of iterations.
             * @param memoryCost Memory usage in kiB.
             * @param parallelism Number of threads used.
             * @param salt User-defined salt. Please leave this empty, a good salt will be generated automatically.
             * @param secret An optional secret if you want to use keyed hashing. In in use, you cannot use
             * Crypto::verifyHash to verify the hash, of course, you have to use this engine directly then.
             */
            explicit Argon2HashingEngine(Algorithm algorithm = ARGON2ID, uint32_t timeCost = 2,
                    uint32_t memoryCost = (1<<16), uint32_t parallelism = 1, std::string salt = "",
                    std::string secret = "");
            /**
             * Generate a hash of the input string with the given salt or, if empty, with a random one, and taking into
             * account the properties.
             * @param input The input string to hash.
             * @return An Argon2 hash in standard format (starting with $argon2id$, for example, and in hex format).
             * The hash itself is 32 bytes (64 hash characters) long.
             */
            std::string generateHash(std::string input) const override;
            /**
             * Verify the given Argon2 hash, hashed with the specified properties.
             * @param input The input string.
             * @param hash The hash to verify.
             * @return True if it matches, false otherwise.
             */
            bool verifyHash(std::string input, std::string hash) const override;
        };

    }
}

#endif //QSF_ARGON2HASHINGENGINE_H
