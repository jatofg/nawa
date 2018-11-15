//
// Created by tobias on 14/11/18.
//

#include "Utils.h"

void
Qsf::regex_replace_callback(std::string &s, const std::regex &rgx, std::function<std::string(std::string)> fmt) {
    std::sregex_token_iterator begin(s.begin(), s.end(), rgx, {-1, 0}), end;
    std::stringstream out;
    // prefixes and matches (should) alternate
    bool match = false;
    for(auto it = begin; it != end; ++it, match = !match) {
        if(match) {
            out << fmt(it->str());
        }
        else {
            out << it->str();
        }
    }
    s = out.str();
}
