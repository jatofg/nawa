//
// Created by tobias on 26/11/18.
//

#include "Config.h"
#include "inih/ini.h"
#include "SysException.h"

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

