/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SORU_HASHTYPETABLE_H
#define SORU_HASHTYPETABLE_H

#include <memory>
#include <soru/Engines/HashingEngine.h>

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

#endif //SORU_HASHTYPETABLE_H
