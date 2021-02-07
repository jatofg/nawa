/**
 * \file Config.cpp
 * \brief Implementation of the Config class.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <nawa/config/Config.h>
#include <nawa/Exception.h>
#include "../../libs/inih/ini.h"

using namespace nawa;
using namespace std;

Config::Config(const string &iniFile) {
    read(iniFile);
}

Config::Config(initializer_list<pair<pair<string, string>, string>> init) {
    values.insert(init.begin(), init.end());
}

Config &Config::operator=(const Config &other) {
    if (this != &other) {
        values = other.values;
    }
    return *this;
}

void Config::read(const string &iniFile) {
    auto valueHandler = [](void *obj, const char *section, const char *name, const char *value) -> int {
        auto _this = (Config *) obj;
        pair<string, string> keyToInsert(section, name);
        pair<pair<string, string>, string> pairToInsert(keyToInsert, value);
        _this->values.insert(pairToInsert);
        return 1;
    };
    if (ini_parse(iniFile.c_str(), valueHandler, this) < 0) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Could not read config file.");
    }
}

void Config::insert(std::initializer_list<std::pair<std::pair<std::string, std::string>, std::string>> init) {
    values.insert(init.begin(), init.end());
}

bool Config::isSet(const pair<string, string> &key) const {
    return (values.count(key) == 1);
}

string Config::operator[](const pair<string, string> &key) const {
    if (values.count(key) == 1) {
        return values.at(key);
    } else {
        return string();
    }
}

// doxygen bug requires std:: here
void Config::set(std::pair<string, string> key, std::string value) {
    values[move(key)] = move(value);
}

void Config::set(string section, string key, string value) {
    set(pair<string, string>(move(section), move(key)), move(value));
}

Config::Config(const Config &other) {
    values = other.values;
}

Config::Config(Config &&other) noexcept: values(move(other.values)) {}

Config &Config::operator=(Config &&other) noexcept {
    if (this != &other) {
        values = move(other.values);
    }
    return *this;
}
