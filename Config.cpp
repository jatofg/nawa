//
// Created by tobias on 26/11/18.
//

#include "Config.h"
#include "inih/ini.h"
#include "SysException.h"

namespace {
    int valueHandler(void* obj, const char* section, const char* name, const char* value) {
        auto m = (std::unordered_map<std::pair<std::string, std::string>, std::string, boost::hash<std::pair<std::string, std::string>>>*) obj;
        std::pair<std::string, std::string> keyToInsert (section, name);
        std::pair<std::pair<std::string, std::string>, std::string> pairToInsert (keyToInsert, value);
        m->insert(pairToInsert);
    }
}

Qsf::Config::Config(std::string iniFile) {
    read(std::move(iniFile));
}

void Qsf::Config::read(std::string iniFile) {
    if(ini_parse(iniFile.c_str(), valueHandler, &values) < 0) {
        throw Qsf::SysException(__FILE__, __LINE__, "Could not read config file.");
    }
}

bool Qsf::Config::isSet(std::pair<std::string, std::string> key) const {
    return (values.count(key) == 1);
}

std::string Qsf::Config::operator[](std::pair<std::string, std::string> key) const {
    if(values.count(key) == 0) {
        return values.at(key);
    }
    else {
        return std::string();
    }
}
