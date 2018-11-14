//
// Created by tobias on 14/11/18.
//

#ifndef QSF_UTILS_H
#define QSF_UTILS_H

#include <string>
#include <regex>

namespace Qsf {
    void regex_replace_callback(std::string& s, const std::regex& rgx, std::function<std::string(std::string)> fmt);
}

#endif //QSF_UTILS_H
