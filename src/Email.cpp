/**
 * \file Email.cpp
 * \brief Implementation of methods in email-related structs in the Qsf::Types namespace.
 */

#include <qsf/Types/Email.h>
#include <qsf/Crypto.h>
#include <sstream>
#include <regex>
#include <random>

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
                ret << "Content-Type: " << part.mimePart->contentType << "\r\n";
                if(!part.mimePart->contentDisposition.empty()) {
                    ret << "Content-Disposition: " << part.mimePart->contentDisposition << "\r\n";
                }
                switch(part.mimePart->applyEncoding) {
                    case Qsf::Types::MimeEmail::MimePart::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n";
                        break;
                    case Qsf::Types::MimeEmail::MimePart::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n";
                        break;
                    case Qsf::Types::MimeEmail::MimePart::NONE:
                        break;
                }

                // continue here: load, encode, and include the payload
                // do not forget the newline (\r\n) at the end

            }
            // if the current part is another list with MIME parts (nested)
            else if(part.mimePartList) {
                ret << "Content-Type: multipart/";
                switch(part.mimePartList->multipartType) {
                    case Qsf::Types::MimeEmail::MimePartList::MIXED:
                        ret << "mixed";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::DIGEST:
                        ret << "digest";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::ALTERNATIVE:
                        ret << "alternative";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::RELATED:
                        ret << "related";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::REPORT:
                        ret << "report";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::SIGNED:
                        ret << "signed";
                        break;
                    case Qsf::Types::MimeEmail::MimePartList::ENCRYPTED:
                        ret << "encrypted";
                        break;
                }
                std::string partBoundary = genBoundary();
                ret << "; boundary=\"" << partBoundary << "\"\r\n\r\n";
                ret << mergeMimePartList(*part.mimePartList, partBoundary) << "\r\n";
            }
        }

        // IMPORTANT: last boundary must have -- at the end!
        // check that the return value ends with a newline (\r\n)!
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

std::string Qsf::Types::MimeEmail::toRaw() {
    std::stringstream ret;
    for(auto const &e: headers) {
        ret << e.first << ": " << e.second << "\r\n";
    }
    ret << "\r\n" << "This is a multi-part message in MIME format" << "\r\n\r\n";
    // set the headers MIME-Version: 1.0 and Content-Type: multipart/... -- boundary in quotes!
    for(auto const &e: mimeParts) {
        // make this a function and think about recursion

        // create start boundary/frontier
        // add part headers
        //Content-Type: text/plain; charset="utf-8"; format="fixed"
        //Content-Transfer-Encoding: quoted-printable
        // \r\n
        // add encoded part OR, IF THIS IS A MIMEPARTLIST, RECURSE
        // !!! make sure the lines are not too long, break them with =

        // create end boundary
    }

    return ret.str();
}
