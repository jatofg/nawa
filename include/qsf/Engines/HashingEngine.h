//
// Created by tobias on 23/04/19.
//

#ifndef QSF_HASHINGENGINE_H
#define QSF_HASHINGENGINE_H

#include <string>

namespace soru {
    namespace Engines {

        /**
         * A hashing engine must implement these functions in order to be useful for password hashing. It may store
         * additional information (such as cost and salt) in its object, the user should be able to set them as
         * constructor parameters. It is expected that the salt is automatically generated, giving the user an option
         * to manually specify the salt is optional but recommended.
         */
        class HashingEngine {
        public:
            /**
             * Generate a (salted) hash basing on the input (usually the password to be hashed). Throw a
             * UserException with an error code >=10 in case of failure.
             * @param input The input string (e.g., user password).
             * @return The hash in a standard format, including the salt and all information necessary to check it
             * (e.g., "$2a$10$...").
             */
            virtual std::string generateHash(std::string input) const = 0;
            /**
             * Check whether the input string has been used to produce the hash. This function should not need to throw
             * exceptions.
             * @param input The input string (e.g., password the user entered).
             * @param hash The stored hash containing all necessary information.
             * @return True if the input string matches the hash, false otherwise.
             */
            virtual bool verifyHash(std::string input, std::string hash) const = 0;
        };

    }
}

#endif //QSF_HASHINGENGINE_H
