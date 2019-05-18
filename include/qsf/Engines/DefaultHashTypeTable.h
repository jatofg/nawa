//
// Created by tobias on 24/04/19.
//

#ifndef QSF_DEFAULTHASHTYPETABLE_H
#define QSF_DEFAULTHASHTYPETABLE_H

#include <qsf/Engines/HashTypeTable.h>

namespace Qsf {
    namespace Engines {

        class DefaultHashTypeTable: public HashTypeTable {
            /**
             * Determine a HashingEngine that can be used to verify the given hash using a list of the hashing engines
             * that are included in QSF (currently bcrypt and argon2).
             * @param hash Hash to be verified (in the standard format, e.g., "$2a$10$...").
             * @return Shared pointer to an instance of a matching HashingEngine. If no matching HashingEngine could be
             * determined by this function, the returned shared_ptr will be empty.
             */
            std::shared_ptr<HashingEngine> getEngine(std::string hash) const override;
        };

    }
}

#endif //QSF_DEFAULTHASHTYPETABLE_H
