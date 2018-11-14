//
// Created by tobias on 14/11/18.
//

#include "Utils.h"

void
Qsf::regex_replace_callback(std::string &s, const std::regex &rgx, std::function<std::string(std::string)> fmt) {
    std::sregex_token_iterator begin(s.begin(), s.end(), rgx, {-1, 0}), end;
    std::stringstream out;
    for(auto it = begin; it != end; ++it) {
        // does matched really behave like expected?
        if(it->matched) {
            out << fmt(it->str());
        }
        else {
            out << it->str();
        }
    }
    s = out.str();
}
