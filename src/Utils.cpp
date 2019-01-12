//
// Created by tobias on 14/11/18.
//

#include <iomanip>
#include "qsf/Utils.h"

void
Qsf::regex_replace_callback(std::string &s, const std::regex &rgx, std::function<std::string(const std::vector<std::string>&)> fmt) {
    // how many submatches do we have to deal with?
    int marks = rgx.mark_count();
    // we want to iterate through all submatches (to collect them in a vector passed to fmt())
    std::vector<int> submatchList;
    for(int i = -1; i <= marks; ++i) {
        submatchList.push_back(i);
    }

    std::sregex_token_iterator begin(s.begin(), s.end(), rgx, submatchList), end;
    std::stringstream out;

    // prefixes and submatches (should) alternate
    int submatch = -1;
    std::vector<std::string> submatchVector;
    for(auto it = begin; it != end; ++it) {
        if(submatch == -1) {
            out << it->str();
            ++submatch;
        }
        else {
            submatchVector.push_back(it->str());
            if(submatch < marks) {
                ++submatch;
            }
            else {
                out << fmt(submatchVector);
                submatchVector.clear();
                submatch = -1;
            }
        }
    }
    s = out.str();
}

std::string Qsf::hex_dump(const std::string &in) {
    std::stringstream rets;
    rets << std::hex << std::setfill('0');
    for(char c: in) {
        rets << std::setw(2) << (int)(unsigned char)c;
    }
    return rets.str();
}

std::string Qsf::to_lowercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string Qsf::to_uppercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}
