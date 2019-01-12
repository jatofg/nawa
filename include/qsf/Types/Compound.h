//
// Created by tobias on 17/10/18.
//

#ifndef QSF_COMPOUND_H
#define QSF_COMPOUND_H

#include <string>

namespace Qsf {
    namespace Types {
        class Compound {
            std::string content;
        public:
            Compound() = default;
            Compound(std::string c) {
                content = std::move(c);
            }
            Compound(long c) {
                content = std::to_string(c);
            }
            Compound(double c) {
                content = std::to_string(c);
            }
            Compound(bool c) {
                content = c ? "1" : "";
            }
            Compound& operator=(const Compound& s) {
                if(this != &s) {
                    content = s.content;
                }
                return *this;
            }
            Compound& operator=(const std::string& s) {
                content = s;
                return *this;
            }
            Compound& operator=(const long& s) {
                content = std::to_string(s);
                return *this;
            }
            Compound& operator=(const double& s) {
                content = std::to_string(s);
                return *this;
            }
            operator std::string() const {
                return content;
            }
            std::string getString() const {
                return content;
            }
        };
    }
}

std::ostream& operator<<(std::ostream& ostream, Qsf::Types::Compound& compound) {
    ostream << compound.getString();
    return ostream;
}


#endif //QSF_COMPOUND_H
