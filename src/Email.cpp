/**
 * \file Email.cpp
 * \brief Implementation of methods in email-related structs in the Qsf::Types namespace.
 */

#include <qsf/Email.h>
#include <qsf/Crypto.h>
#include <sstream>
#include <regex>
#include <random>
#include <qsf/UserException.h>
#include <fstream>
#include <qsf/Encoding.h>
#include <qsf/Utils.h>

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
        ret << Qsf::Crypto::md5(std::to_string(rd()));

        return ret.str();
    }

    std::string multipartTypeToString(const Qsf::MimeEmail::MimePartList::MultipartType &multipartType) {
        switch(multipartType) {
            case Qsf::MimeEmail::MimePartList::MIXED:
                return "mixed";
            case Qsf::MimeEmail::MimePartList::DIGEST:
                return "digest";
            case Qsf::MimeEmail::MimePartList::ALTERNATIVE:
                return "alternative";
            case Qsf::MimeEmail::MimePartList::RELATED:
                return "related";
            case Qsf::MimeEmail::MimePartList::REPORT:
                return "report";
            case Qsf::MimeEmail::MimePartList::SIGNED:
                return "signed";
            case Qsf::MimeEmail::MimePartList::ENCRYPTED:
                return "encrypted";
        }
        return "";
    }

    /**
     * Takes a MimePartList and converts it to a string (recursively) for inclusion in an email message.
     * @param mimePartList The MimePartList object representing the MIME parts to be converted.
     * @return Tuple containing the boundary string (for inclusion in the header) and the MIME parts as a string.
     */
    std::string mergeMimePartList(const Qsf::MimeEmail::MimePartList &mimePartList, const std::string& boundary,
            const Qsf::ReplacementRules& replacementRules) {
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
                    case Qsf::MimeEmail::MimePart::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n\r\n";
                        ret << Qsf::Encoding::base64Encode(
                                (mimePart.allowReplacements && !replacementRules.empty())
                                ? Qsf::string_replace(mimePart.data, replacementRules) : mimePart.data,
                                76, "\r\n");
                        break;
                    case Qsf::MimeEmail::MimePart::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
                        ret << Qsf::Encoding::quotedPrintableEncode(
                                (mimePart.allowReplacements && !replacementRules.empty())
                                ? Qsf::string_replace(mimePart.data, replacementRules) : mimePart.data
                                );
                        break;
                    case Qsf::MimeEmail::MimePart::NONE:
                        ret << "\r\n" << ((mimePart.allowReplacements && !replacementRules.empty())
                        ? Qsf::string_replace(mimePart.data, replacementRules) : mimePart.data);
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

bool Qsf::EmailAddress::isValid() {
    std::regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", std::regex::icase);
    return std::regex_match(address, emCheck);
}

std::string Qsf::SimpleEmail::getRaw(const ReplacementRules &replacementRules) const {
    std::stringstream ret;
    for(auto const &e: headers) {
        ret << e.first << ": " << e.second << "\r\n";
    }
    ret << "\r\n" << (replacementRules.empty() ? text : string_replace(text, replacementRules));
    return ret.str();
}

Qsf::MimeEmail::MimePartOrList::MimePartOrList(const Qsf::MimeEmail::MimePartOrList &other) {
    operator=(other);
}

Qsf::MimeEmail::MimePartOrList::MimePartOrList(const Qsf::MimeEmail::MimePart &_mimePart) {
    operator=(_mimePart);
}

Qsf::MimeEmail::MimePartOrList::MimePartOrList(const Qsf::MimeEmail::MimePartList &_mimePartList) {
    operator=(_mimePartList);
}

Qsf::MimeEmail::MimePartOrList &Qsf::MimeEmail::MimePartOrList::operator=(const Qsf::MimeEmail::MimePartOrList &other) {
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

Qsf::MimeEmail::MimePartOrList &Qsf::MimeEmail::MimePartOrList::operator=(const Qsf::MimeEmail::MimePart &_mimePart) {
    if(!mimePart) {
        mimePart = std::make_unique<MimePart>();
    }
    if(mimePartList) {
        mimePartList.reset(nullptr);
    }
    *mimePart = _mimePart;
    return *this;
}

Qsf::MimeEmail::MimePartOrList &
Qsf::MimeEmail::MimePartOrList::operator=(const Qsf::MimeEmail::MimePartList &_mimePartList) {
    if(!mimePartList) {
        mimePartList = std::make_unique<MimePartList>();
    }
    if(mimePart) {
        mimePart.reset(nullptr);
    }
    *mimePartList = _mimePartList;
    return *this;
}

std::string Qsf::MimeEmail::getRaw(const ReplacementRules &replacementRules) const {
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
