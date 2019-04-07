/**
 * \file Email.cpp
 * \brief Implementation of methods in email-related structs in the Qsf::Types namespace.
 */

#include <qsf/Types/Email.h>
#include <qsf/Crypto.h>
#include <sstream>
#include <regex>
#include <random>
#include <qsf/UserException.h>
#include <fstream>
#include <qsf/Encoding.h>

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

    std::string multipartTypeToString(const Qsf::Types::MimeEmail::MimePartList::MultipartType &multipartType) {
        switch(multipartType) {
            case Qsf::Types::MimeEmail::MimePartList::MIXED:
                return "mixed";
            case Qsf::Types::MimeEmail::MimePartList::DIGEST:
                return "digest";
            case Qsf::Types::MimeEmail::MimePartList::ALTERNATIVE:
                return "alternative";
            case Qsf::Types::MimeEmail::MimePartList::RELATED:
                return "related";
            case Qsf::Types::MimeEmail::MimePartList::REPORT:
                return "report";
            case Qsf::Types::MimeEmail::MimePartList::SIGNED:
                return "signed";
            case Qsf::Types::MimeEmail::MimePartList::ENCRYPTED:
                return "encrypted";
        }
        return "";
    }

    /**
     * Takes a MimePartList and converts it to a string (recursively) for inclusion in an email message.
     * @param mimePartList The MimePartList object representing the MIME parts to be converted.
     * @return Tuple containing the boundary string (for inclusion in the header) and the MIME parts as a string.
     */
    std::string mergeMimePartList(const Qsf::Types::MimeEmail::MimePartList &mimePartList, const std::string& boundary) {
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

                switch(part.mimePart->applyEncoding) {
                    case Qsf::Types::MimeEmail::MimePart::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n\r\n";
                        ret << Qsf::Encoding::base64Encode(mimePart.data, 76, "\r\n");
                        break;
                    case Qsf::Types::MimeEmail::MimePart::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
                        ret << Qsf::Encoding::quotedPrintableEncode(mimePart.data);
                        break;
                    case Qsf::Types::MimeEmail::MimePart::NONE:
                        ret << "\r\n" << mimePart.data;
                        break;
                }

                ret << "\r\n\r\n";

            }
            // if the current part is another list with MIME parts (nested)
            else if(part.mimePartList) {
                ret << "Content-Type: multipart/" << multipartTypeToString(part.mimePartList->multipartType);
                std::string partBoundary = genBoundary();
                ret << "; boundary=\"" << partBoundary << "\"\r\n\r\n";
                ret << mergeMimePartList(*part.mimePartList, partBoundary) << "\r\n\r\n";
            }
        }

        ret << "--" << boundary << "--";
        return ret.str();
    }
}

bool Qsf::Types::EmailAddress::isValid() {
    std::regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", std::regex::icase);
    return std::regex_match(address, emCheck);
}

std::string Qsf::Types::SimpleEmail::toRaw() {
    std::stringstream ret;
    for(auto const &e: headers) {
        ret << e.first << ": " << e.second << "\r\n";
    }
    ret << "\r\n" << text;
    return ret.str();
}

Qsf::Types::MimeEmail::MimePartOrList::MimePartOrList(const Qsf::Types::MimeEmail::MimePartOrList &other) {
    operator=(other);
}

Qsf::Types::MimeEmail::MimePartOrList &
Qsf::Types::MimeEmail::MimePartOrList::operator=(const Qsf::Types::MimeEmail::MimePartOrList &other) {
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

std::string Qsf::Types::MimeEmail::toRaw() {
    std::stringstream ret;
    headers["MIME-Version"] = "1.0";
    std::string boundary = genBoundary();
    headers["Content-Type"] = "multipart/" + multipartTypeToString(mimeParts.multipartType) + "; boundary=\"" + boundary + "\"";
    for(auto const &e: headers) {
        ret << e.first << ": " << e.second << "\r\n";
    }
    ret << "\r\n" << "This is a multi-part message in MIME format" << "\r\n\r\n";
    ret << mergeMimePartList(mimeParts, boundary);

    return ret.str();
}