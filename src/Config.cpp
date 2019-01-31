/**
 * \file Config.cpp
 * \brief Implementation of the Config class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <qsf/Config.h>
#include <qsf/SysException.h>
#include "../libs/inih/ini.h"

Qsf::Config::Config(std::string iniFile) {
    read(std::move(iniFile));
}

Qsf::Config &Qsf::Config::operator=(const Qsf::Config &other) {
    if(this != &other) {
        values = other.values;
    }
    return *this;
}

void Qsf::Config::read(std::string iniFile) {
    auto valueHandler = [](void* obj, const char* section, const char* name, const char* value) -> int {
        auto _this = (Qsf::Config*) obj;
        std::pair<std::string, std::string> keyToInsert (section, name);
        std::pair<std::pair<std::string, std::string>, std::string> pairToInsert (keyToInsert, value);
        _this->values.insert(pairToInsert);
        return 1;
    };
    if(ini_parse(iniFile.c_str(), valueHandler, this) < 0) {
        throw Qsf::SysException(__FILE__, __LINE__, "Could not read config file.");
    }
}

bool Qsf::Config::isSet(std::pair<std::string, std::string> key) const {
    return (values.count(key) == 1);
}

std::string Qsf::Config::operator[](std::pair<std::string, std::string> key) const {
    if(values.count(key) == 1) {
        return values.at(key);
    }
    else {
        return std::string();
    }
}

void Qsf::Config::set(std::pair<std::string, std::string> key, std::string value) {
    values[std::move(key)] = std::move(value);
}

void Qsf::Config::set(std::string section, std::string key, std::string value) {
    set(std::pair<std::string, std::string> (std::move(section), std::move(key)), std::move(value));
}
