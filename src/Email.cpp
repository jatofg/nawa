/**
 * \file Email.cpp
 * \brief Implementation of methods in email-related structs in the Nawa::Types namespace.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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
#include <nawa/Encoding.h>
#include <nawa/Utils.h>

using namespace nawa;
using namespace std;

namespace {
    /**
     * Generate a random MIME boundary.
     * @return The boundary string.
     */
    string genBoundary() {
        stringstream ret;
        ret << "----=_";

        // Use MD5 sum of a random integer
        random_device rd;
        ret << Crypto::md5(to_string(rd()));

        return ret.str();
    }

    string multipartTypeToString(const MimeEmail::MimePartList::MultipartType &multipartType) {
        switch (multipartType) {
            case MimeEmail::MimePartList::MultipartType::MIXED:
                return "mixed";
            case MimeEmail::MimePartList::MultipartType::DIGEST:
                return "digest";
            case MimeEmail::MimePartList::MultipartType::ALTERNATIVE:
                return "alternative";
            case MimeEmail::MimePartList::MultipartType::RELATED:
                return "related";
            case MimeEmail::MimePartList::MultipartType::REPORT:
                return "report";
            case MimeEmail::MimePartList::MultipartType::SIGNED:
                return "signed";
            case MimeEmail::MimePartList::MultipartType::ENCRYPTED:
                return "encrypted";
        }
        return "";
    }

    /**
     * Takes a MimePartList and converts it to a string (recursively) for inclusion in an email message.
     * @param mimePartList The MimePartList object representing the MIME parts to be converted.
     * @return Tuple containing the boundary string (for inclusion in the header) and the MIME parts as a string.
     */
    string mergeMimePartList(const MimeEmail::MimePartList &mimePartList, const string &boundary,
                                  const shared_ptr<ReplacementRules> &replacementRules) {
        stringstream ret;

        // iterate through the list
        for (auto const &part: mimePartList.mimeParts) {
            ret << "--" << boundary << "\r\n";
            // if the current part is a MIME part with content
            if (part.mimePart) {
                const auto &mimePart = *(part.mimePart);
                ret << "Content-Type: " << mimePart.contentType << "\r\n";
                if (!mimePart.contentDisposition.empty()) {
                    ret << "Content-Disposition: " << mimePart.contentDisposition << "\r\n";
                }
                for (const auto &e: mimePart.partHeaders) {
                    ret << e.first << ": " << e.second << "\r\n";
                }

                // apply the replacement rules (only if necessary) and the selected encoding afterwards
                switch (mimePart.applyEncoding) {
                    case MimeEmail::MimePart::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n\r\n";
                        ret << Encoding::base64Encode(
                                (mimePart.allowReplacements && replacementRules)
                                ? string_replace(mimePart.data, *replacementRules) : mimePart.data,
                                76, "\r\n");
                        break;
                    case MimeEmail::MimePart::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
                        ret << Encoding::quotedPrintableEncode(
                                (mimePart.allowReplacements && replacementRules)
                                ? string_replace(mimePart.data, *replacementRules) : mimePart.data
                        );
                        break;
                    case MimeEmail::MimePart::NONE:
                        ret << "\r\n" << ((mimePart.allowReplacements && replacementRules)
                                          ? string_replace(mimePart.data, *replacementRules) : mimePart.data);
                        break;
                }

                ret << "\r\n\r\n";

            }
                // if the current part is another list with MIME parts (nested)
            else if (part.mimePartList) {
                ret << "Content-Type: multipart/" << multipartTypeToString(part.mimePartList->multipartType);
                string partBoundary = genBoundary();
                ret << "; boundary=\"" << partBoundary << "\"\r\n\r\n";
                ret << mergeMimePartList(*part.mimePartList, partBoundary, replacementRules) << "\r\n\r\n";
            }
        }

        ret << "--" << boundary << "--";
        return ret.str();
    }
}

string EmailAddress::get(bool includeName) const {
    stringstream ret;
    if (includeName) {
        ret << name << " ";
    }
    ret << '<' << address << '>';
    return ret.str();
}

bool EmailAddress::isValid() const {
    regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", regex::icase);
    return regex_match(address, emCheck);
}

string SimpleEmail::getRaw(const shared_ptr<ReplacementRules> &replacementRules) const {
    stringstream ret;

    for (auto const &e: headers) {
        if (e.first == "MIME-Version" || (quotedPrintableEncode && e.first == "Content-Transfer-Encoding"))
            continue;
        ret << e.first << ": " << e.second << "\r\n";
    }

    ret << "MIME-Version: 1.0\r\n";
    if (quotedPrintableEncode) {
        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
        ret << Encoding::quotedPrintableEncode(
                replacementRules ? string_replace(text, *replacementRules) : text
        );
    } else {
        ret << "\r\n" << (replacementRules ? string_replace(text, *replacementRules) : text);
    }

    return ret.str();
}

MimeEmail::MimePartOrList::MimePartOrList(const MimeEmail::MimePartOrList &other) {
    operator=(other);
}

MimeEmail::MimePartOrList::MimePartOrList(const MimeEmail::MimePart &_mimePart) {
    operator=(_mimePart);
}

MimeEmail::MimePartOrList::MimePartOrList(const MimeEmail::MimePartList &_mimePartList) {
    operator=(_mimePartList);
}

MimeEmail::MimePartOrList &
MimeEmail::MimePartOrList::operator=(const MimeEmail::MimePartOrList &other) {
    if (&other == this) {
        return *this;
    }
    if (other.mimePart) {
        if (!mimePart) {
            mimePart = make_unique<MimePart>();
        }
        if (mimePartList) {
            mimePartList.reset(nullptr);
        }
        *mimePart = *(other.mimePart);
    } else if (other.mimePartList) {
        if (!mimePartList) {
            mimePartList = make_unique<MimePartList>();
        }
        if (mimePart) {
            mimePart.reset(nullptr);
        }
        *mimePartList = *(other.mimePartList);
    }
    return *this;
}

MimeEmail::MimePartOrList &
MimeEmail::MimePartOrList::operator=(const MimeEmail::MimePart &_mimePart) {
    if (!mimePart) {
        mimePart = make_unique<MimePart>();
    }
    if (mimePartList) {
        mimePartList.reset(nullptr);
    }
    *mimePart = _mimePart;
    return *this;
}

MimeEmail::MimePartOrList &
MimeEmail::MimePartOrList::operator=(const MimeEmail::MimePartList &_mimePartList) {
    if (!mimePartList) {
        mimePartList = make_unique<MimePartList>();
    }
    if (mimePart) {
        mimePart.reset(nullptr);
    }
    *mimePartList = _mimePartList;
    return *this;
}

string MimeEmail::getRaw(const shared_ptr<ReplacementRules> &replacementRules) const {
    stringstream ret;
    for (auto const &e: headers) {
        if (e.first == "MIME-Version" || e.first == "Content-Type") continue;
        ret << e.first << ": " << e.second << "\r\n";
    }
    string boundary = genBoundary();
    ret << "MIME-Version: 1.0\r\nContent-Type: multipart/" << multipartTypeToString(mimePartList.multipartType);
    ret << "; boundary=\"" << boundary;
    ret << "\"\r\n\r\nThis is a multi-part message in MIME format\r\n\r\n";
    ret << mergeMimePartList(mimePartList, boundary, replacementRules);

    return ret.str();
}
