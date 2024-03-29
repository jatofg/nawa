/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file DefaultHashTypeTable.cpp
 * \brief Implementation of the DefaultHashTypeTable class.
 */

#include <nawa/hashing/HashTypeTable/impl/DefaultHashTypeTable.h>
#include <nawa/hashing/HashingEngine/impl/Argon2HashingEngine.h>
#include <nawa/hashing/HashingEngine/impl/BcryptHashingEngine.h>

using namespace nawa;
using namespace std;

std::shared_ptr<hashing::HashingEngine> hashing::DefaultHashTypeTable::getEngine(std::string hash) const {
    auto hid = hash.substr(0, 4);
    if (hid == "$2a$" || hid == "$2b$" || hid == "$2x$" || hid == "$2y$") {
        return shared_ptr<hashing::HashingEngine>(new hashing::BcryptHashingEngine());
    } else if (hash.substr(0, 10) == "$argon2id$" || hash.substr(0, 9) == "$argon2i$" ||
               hash.substr(0, 9) == "$argon2d$") {
        return shared_ptr<hashing::HashingEngine>(new hashing::Argon2HashingEngine());
    }
    return {};
}
