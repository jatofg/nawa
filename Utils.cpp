//
// Created by tobias on 14/11/18.
//

#include "Utils.h"

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
