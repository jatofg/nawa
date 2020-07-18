/**
 * \file DefaultHashTypeTable.h
 * \brief Default implementation of a HashTypeTable containing the hashing engines included in nawa.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#ifndef NAWA_DEFAULTHASHTYPETABLE_H
#define NAWA_DEFAULTHASHTYPETABLE_H

#include <nawa/Engines/HashTypeTable.h>

namespace nawa {
    namespace Engines {

        class DefaultHashTypeTable: public HashTypeTable {
            /**
             * Determine a HashingEngine that can be used to verify the given hash using a list of the hashing engines
             * that are included in nawa (currently bcrypt and argon2).
             * @param hash Hash to be verified (in the standard format, e.g., "$2a$10$...").
             * @return Shared pointer to an instance of a matching HashingEngine. If no matching HashingEngine could be
             * determined by this function, the returned shared_ptr will be empty.
             */
            std::shared_ptr<HashingEngine> getEngine(std::string hash) const override;
        };

    }
}

#endif //NAWA_DEFAULTHASHTYPETABLE_H
