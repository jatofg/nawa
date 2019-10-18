/**
 * \file DefaultHashTypeTable.cpp
 * \brief Implementation of the DefaultHashTypeTable class.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#include <soru/Engines/DefaultHashTypeTable.h>
#include <soru/Engines/BcryptHashingEngine.h>
#include <soru/Engines/Argon2HashingEngine.h>

std::shared_ptr<soru::Engines::HashingEngine> soru::Engines::DefaultHashTypeTable::getEngine(std::string hash) const {
    auto hid = hash.substr(0, 4);
    if(hid == "$2a$" || hid == "$2b$" || hid == "$2x$" || hid == "$2y$") {
        return std::shared_ptr<soru::Engines::HashingEngine>(new soru::Engines::BcryptHashingEngine());
    }
    else if(hash.substr(0, 10) == "$argon2id$" || hash.substr(0, 9) == "$argon2i$" || hash.substr(0, 9) == "$argon2d$") {
        return std::shared_ptr<soru::Engines::HashingEngine>(new soru::Engines::Argon2HashingEngine());
    }
    return std::shared_ptr<soru::Engines::HashingEngine>();
}
