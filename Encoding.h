//
// Created by tobias on 09/11/18.
//

#ifndef QSF_ENCODING_H
#define QSF_ENCODING_H

#include <string>
#include <unordered_map>

namespace Qsf {
    /**
     * Namespace containing functions for text encoding and decoding
     */
    namespace Encoding {
        std::string htmlEncode(std::string input, bool encodeAll = false);
        std::string htmlDecode(std::string input);
        // TODO url encoding, base64 encoding, ...
    }
}

#endif //QSF_ENCODING_H
