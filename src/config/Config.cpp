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

#include "../../libs/inih/ini.h"
#include <boost/functional/hash.hpp>
#include <nawa/Exception.h>
#include <nawa/config/Config.h>
#include <nawa/internal/macros.h>
#include <unordered_map>

using namespace nawa;
using namespace std;

// implementation
struct Config::Impl {
    std::unordered_map<std::pair<std::string, std::string>, std::string, boost::hash<std::pair<std::string, std::string>>> values;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Config)
NAWA_COPY_CONSTRUCTOR_IMPL(Config)
NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Config)
NAWA_MOVE_CONSTRUCTOR_IMPL(Config)
NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Config)
NAWA_DEFAULT_CONSTRUCTOR_IMPL(Config)

Config::Config(initializer_list<pair<pair<string, string>, string>> init) : Config() {
    impl->values.insert(init.begin(), init.end());
}

Config::Config(const string &iniFile) : Config() {
    read(iniFile);
}

void Config::read(const string &iniFile) {
    auto valueHandler = [](void *obj, const char *section, const char *name, const char *value) -> int {
        auto _this = (Config *) obj;
        pair<string, string> keyToInsert(section, name);
        pair<pair<string, string>, string> pairToInsert(keyToInsert, value);
        _this->impl->values.insert(pairToInsert);
        return 1;
    };
    if (ini_parse(iniFile.c_str(), valueHandler, this) < 0) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Could not read config file.");
    }
}

void Config::insert(std::initializer_list<std::pair<std::pair<std::string, std::string>, std::string>> init) {
    impl->values.insert(init.begin(), init.end());
}

bool Config::isSet(const pair<string, string> &key) const {
    return (impl->values.count(key) == 1);
}

string Config::operator[](const pair<string, string> &key) const {
    if (impl->values.count(key) == 1) {
        return impl->values.at(key);
    } else {
        return string();
    }
}

// doxygen bug requires std:: here
void Config::set(std::pair<string, string> key, std::string value) {
    impl->values[move(key)] = move(value);
}

void Config::set(string section, string key, string value) {
    set(pair<string, string>(move(section), move(key)), move(value));
}
