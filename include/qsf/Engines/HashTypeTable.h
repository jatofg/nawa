//
// Created by tobias on 23/04/19.
//

#ifndef QSF_HASHTYPETABLE_H
#define QSF_HASHTYPETABLE_H

#include <memory>
#include <qsf/Engines/HashingEngine.h>

namespace soru {
    namespace Engines {

        /**
         * A HashTypeTable should be used to determine the HashingEngine that can be used to verify a certain hash. It
         * may take constructor parameters.
         */
        class HashTypeTable {
        public:
            /**
             * Determine a HashingEngine that can be used to verify the given hash (usually the one that generated it).
             * @param hash Hash to be verified (in the standard format, e.g., "$2a$10$...").
             * @return Shared pointer to an instance of a matching HashingEngine. If no matching HashingEngine could be
             * determined by this function, the returned shared_ptr should be empty.
             */
            virtual std::shared_ptr<HashingEngine> getEngine(std::string hash) const = 0;
        };

    }
}

#endif //QSF_HASHTYPETABLE_H
