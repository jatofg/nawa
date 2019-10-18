/**
 * \file HashTypeTable.h
 * \brief Abstract base class for hashing engines.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#ifndef NAWA_HASHTYPETABLE_H
#define NAWA_HASHTYPETABLE_H

#include <memory>
#include <nawa/Engines/HashingEngine.h>

namespace nawa {
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

#endif //NAWA_HASHTYPETABLE_H
