/**
 * \file Email.cpp
 * \brief Implementation of methods in email-related structs in the Nawa::Types namespace.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#include <nawa/Email.h>
#include <nawa/Crypto.h>
#include <sstream>
#include <regex>
#include <random>
#include <nawa/UserException.h>
#include <fstream>
#include <nawa/Encoding.h>
#include <nawa/Utils.h>

namespace {
    /**
     * Generate a random MIME boundary.
     * @return The boundary string.
     */
    std::string genBoundary() {
        std::stringstream ret;
        ret << "----=_";

        // Use MD5 sum of a random integer
        std::random_device rd;
        ret << nawa::Crypto::md5(std::to_string(rd()));

        return ret.str();
    }

    std::string multipartTypeToString(const nawa::MimeEmail::MimePartList::MultipartType &multipartType) {
        switch(multipartType) {
            case nawa::MimeEmail::MimePartList::MIXED:
                return "mixed";
            case nawa::MimeEmail::MimePartList::DIGEST:
                return "digest";
            case nawa::MimeEmail::MimePartList::ALTERNATIVE:
                return "alternative";
            case nawa::MimeEmail::MimePartList::RELATED:
                return "related";
            case nawa::MimeEmail::MimePartList::REPORT:
                return "report";
            case nawa::MimeEmail::MimePartList::SIGNED:
                return "signed";
            case nawa::MimeEmail::MimePartList::ENCRYPTED:
                return "encrypted";
        }
        return "";
    }

    /**
     * Takes a MimePartList and converts it to a string (recursively) for inclusion in an email message.
     * @param mimePartList The MimePartList object representing the MIME parts to be converted.
     * @return Tuple containing the boundary string (for inclusion in the header) and the MIME parts as a string.
     */
    std::string mergeMimePartList(const nawa::MimeEmail::MimePartList &mimePartList, const std::string& boundary,
            const nawa::ReplacementRules& replacementRules) {
        std::stringstream ret;

        // iterate through the list
        for(auto const &part: mimePartList.mimeParts) {
            ret << "--" << boundary << "\r\n";
            // if the current part is a MIME part with content
            if(part.mimePart) {
                const auto &mimePart = *(part.mimePart);
                ret << "Content-Type: " << mimePart.contentType << "\r\n";
                if(!mimePart.contentDisposition.empty()) {
                    ret << "Content-Disposition: " << mimePart.contentDisposition << "\r\n";
                }
                for(const auto &e: mimePart.partHeaders) {
                    ret << e.first << ": " << e.second << "\r\n";
                }

                // apply the replacement rules (only if necessary) and the selected encoding afterwards
                switch(mimePart.applyEncoding) {
                    case nawa::MimeEmail::MimePart::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n\r\n";
                        ret << nawa::Encoding::base64Encode(
                                (mimePart.allowReplacements && !replacementRules.empty())
                                ? nawa::string_replace(mimePart.data, replacementRules) : mimePart.data,
                                76, "\r\n");
                        break;
                    case nawa::MimeEmail::MimePart::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
                        ret << nawa::Encoding::quotedPrintableEncode(
                                (mimePart.allowReplacements && !replacementRules.empty())
                                ? nawa::string_replace(mimePart.data, replacementRules) : mimePart.data
                                );
                        break;
                    case nawa::MimeEmail::MimePart::NONE:
                        ret << "\r\n" << ((mimePart.allowReplacements && !replacementRules.empty())
                        ? nawa::string_replace(mimePart.data, replacementRules) : mimePart.data);
                        break;
                }

                ret << "\r\n\r\n";

            }
            // if the current part is another list with MIME parts (nested)
            else if(part.mimePartList) {
                ret << "Content-Type: multipart/" << multipartTypeToString(part.mimePartList->multipartType);
                std::string partBoundary = genBoundary();
                ret << "; boundary=\"" << partBoundary << "\"\r\n\r\n";
                ret << mergeMimePartList(*part.mimePartList, partBoundary, replacementRules) << "\r\n\r\n";
            }
        }

        ret << "--" << boundary << "--";
        return ret.str();
    }
}

std::string nawa::EmailAddress::get(bool includeName) const {
    std::stringstream ret;
    if(includeName) {
        ret << name << " ";
    }
    ret << '<' << address << '>';
    return ret.str();
}

bool nawa::EmailAddress::isValid() const {
    std::regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", std::regex::icase);
    return std::regex_match(address, emCheck);
}

std::string nawa::SimpleEmail::getRaw(const ReplacementRules &replacementRules) const {
    std::stringstream ret;

    for(auto const &e: headers) {
        if(quotedPrintableEncode && e.first == "Content-Transfer-Encoding")
            continue;
        ret << e.first << ": " << e.second << "\r\n";
    }

    if(quotedPrintableEncode) {
        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
        ret << Encoding::quotedPrintableEncode(
                replacementRules.empty() ? text : string_replace(text, replacementRules)
                );
    }
    else {
        ret << "\r\n" << (replacementRules.empty() ? text : string_replace(text, replacementRules));
    }

    return ret.str();
}

nawa::MimeEmail::MimePartOrList::MimePartOrList(const nawa::MimeEmail::MimePartOrList &other) {
    operator=(other);
}

nawa::MimeEmail::MimePartOrList::MimePartOrList(const nawa::MimeEmail::MimePart &_mimePart) {
    operator=(_mimePart);
}

nawa::MimeEmail::MimePartOrList::MimePartOrList(const nawa::MimeEmail::MimePartList &_mimePartList) {
    operator=(_mimePartList);
}

nawa::MimeEmail::MimePartOrList &nawa::MimeEmail::MimePartOrList::operator=(const nawa::MimeEmail::MimePartOrList &other) {
    if(&other == this) {
        return *this;
    }
    if(other.mimePart) {
        if(!mimePart) {
            mimePart = std::make_unique<MimePart>();
        }
        if(mimePartList) {
            mimePartList.reset(nullptr);
        }
        *mimePart = *(other.mimePart);
    }
    else if(other.mimePartList) {
        if(!mimePartList) {
            mimePartList = std::make_unique<MimePartList>();
        }
        if(mimePart) {
            mimePart.reset(nullptr);
        }
        *mimePartList = *(other.mimePartList);
    }
    return *this;
}

nawa::MimeEmail::MimePartOrList &nawa::MimeEmail::MimePartOrList::operator=(const nawa::MimeEmail::MimePart &_mimePart) {
    if(!mimePart) {
        mimePart = std::make_unique<MimePart>();
    }
    if(mimePartList) {
        mimePartList.reset(nullptr);
    }
    *mimePart = _mimePart;
    return *this;
}

nawa::MimeEmail::MimePartOrList &
nawa::MimeEmail::MimePartOrList::operator=(const nawa::MimeEmail::MimePartList &_mimePartList) {
    if(!mimePartList) {
        mimePartList = std::make_unique<MimePartList>();
    }
    if(mimePart) {
        mimePart.reset(nullptr);
    }
    *mimePartList = _mimePartList;
    return *this;
}

std::string nawa::MimeEmail::getRaw(const ReplacementRules &replacementRules) const {
    std::stringstream ret;
    for(auto const &e: headers) {
        if(e.first == "MIME-Version" || e.first == "Content-Type") continue;
        ret << e.first << ": " << e.second << "\r\n";
    }
    std::string boundary = genBoundary();
    ret << "MIME-Version: 1.0\r\nContent-Type: multipart/" << multipartTypeToString(mimePartList.multipartType);
    ret << "; boundary=\"" << boundary;
    ret << "\"\r\n\r\nThis is a multi-part message in MIME format\r\n\r\n";
    ret << mergeMimePartList(mimePartList, boundary, replacementRules);

    return ret.str();
}
