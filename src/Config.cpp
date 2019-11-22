/**
 * \file Config.cpp
 * \brief Implementation of the Config class.
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

#include <nawa/Config.h>
#include <nawa/SysException.h>
#include "../libs/inih/ini.h"

nawa::Config::Config(std::string iniFile) {
    read(std::move(iniFile));
}

nawa::Config &nawa::Config::operator=(const nawa::Config &other) {
    if(this != &other) {
        values = other.values;
    }
    return *this;
}

void nawa::Config::read(const std::string &iniFile) {
    auto valueHandler = [](void* obj, const char* section, const char* name, const char* value) -> int {
        auto _this = (nawa::Config*) obj;
        std::pair<std::string, std::string> keyToInsert (section, name);
        std::pair<std::pair<std::string, std::string>, std::string> pairToInsert (keyToInsert, value);
        _this->values.insert(pairToInsert);
        return 1;
    };
    if(ini_parse(iniFile.c_str(), valueHandler, this) < 0) {
        throw nawa::SysException(__FILE__, __LINE__, "Could not read config file.");
    }
}

bool nawa::Config::isSet(const std::pair<std::string, std::string> &key) const {
    return (values.count(key) == 1);
}

std::string nawa::Config::operator[](const std::pair<std::string, std::string>& key) const {
    if(values.count(key) == 1) {
        return values.at(key);
    }
    else {
        return std::string();
    }
}

void nawa::Config::set(std::pair<std::string, std::string> key, std::string value) {
    values[std::move(key)] = std::move(value);
}

void nawa::Config::set(std::string section, std::string key, std::string value) {
    set(std::pair<std::string, std::string> (std::move(section), std::move(key)), std::move(value));
}
