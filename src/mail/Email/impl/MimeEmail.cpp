/**
 * \file MimeEmail.cpp
 * \brief Implementation of the MimeEmail class.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <nawa/mail/Email/impl/MimeEmail.h>
#include <nawa/util/crypto.h>
#include <nawa/util/encoding.h>
#include <nawa/util/utils.h>
#include <random>
#include <sstream>

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
        ret << crypto::md5(to_string(rd()));

        return ret.str();
    }

    string multipartTypeToString(MimeEmail::MimePartList::MultipartType const& multipartType) {
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
    string mergeMimePartList(MimeEmail::MimePartList const& mimePartList, string const& boundary,
                             shared_ptr<ReplacementRules> const& replacementRules) {
        stringstream ret;

        // iterate through the list
        for (auto const& part : mimePartList.mimeParts()) {
            ret << "--" << boundary << "\r\n";
            // if the current part is a MIME part with content
            if (part.mimePart()) {
                auto const& mimePart = *(part.mimePart());
                ret << "Content-Type: " << mimePart.contentType() << "\r\n";
                if (!mimePart.contentDisposition().empty()) {
                    ret << "Content-Disposition: " << mimePart.contentDisposition() << "\r\n";
                }
                for (auto const& e : mimePart.partHeaders()) {
                    ret << e.first << ": " << e.second << "\r\n";
                }

                // apply the replacement rules (only if necessary) and the selected encoding afterwards
                switch (mimePart.applyEncoding()) {
                    case MimeEmail::MimePart::ApplyEncoding::BASE64:
                        ret << "Content-Transfer-Encoding: base64\r\n\r\n";
                        ret << encoding::base64Encode(
                                (mimePart.allowReplacements() && replacementRules)
                                        ? string_replace(mimePart.partData(), *replacementRules)
                                        : mimePart.partData(),
                                76, "\r\n");
                        break;
                    case MimeEmail::MimePart::ApplyEncoding::QUOTED_PRINTABLE:
                        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
                        ret << encoding::quotedPrintableEncode(
                                (mimePart.allowReplacements() && replacementRules)
                                        ? string_replace(mimePart.partData(), *replacementRules)
                                        : mimePart.partData());
                        break;
                    case MimeEmail::MimePart::ApplyEncoding::NONE:
                        ret << "\r\n"
                            << ((mimePart.allowReplacements() && replacementRules)
                                        ? string_replace(mimePart.partData(), *replacementRules)
                                        : mimePart.partData());
                        break;
                }

                ret << "\r\n\r\n";

            }
            // if the current part is another list with MIME parts (nested)
            else if (part.mimePartList()) {
                ret << "Content-Type: multipart/" << multipartTypeToString(part.mimePartList()->multipartType());
                string partBoundary = genBoundary();
                ret << "; boundary=\"" << partBoundary << "\"\r\n\r\n";
                ret << mergeMimePartList(*part.mimePartList(), partBoundary, replacementRules) << "\r\n\r\n";
            }
        }

        ret << "--" << boundary << "--";
        return ret.str();
    }
}// namespace

struct MimeEmail::Data {
    MimePartList mimePartList;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(MimeEmail)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(MimeEmail)

NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL(MimeEmail, Email)

NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(MimeEmail, Email)

NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL(MimeEmail, Email)

NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(MimeEmail, Email)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail, mimePartList, MimeEmail::MimePartList)

struct MimeEmail::MimePart::Data {
    std::string contentType;
    std::string contentDisposition;
    ApplyEncoding applyEncoding = ApplyEncoding::QUOTED_PRINTABLE;
    HeadersMap partHeaders;
    bool allowReplacements = false;
    std::string partData;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_COPY_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_MOVE_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePart)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, contentType, string)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, contentDisposition, string)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, applyEncoding, MimeEmail::MimePart::ApplyEncoding)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, partHeaders, MimeEmail::HeadersMap)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, allowReplacements, bool)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePart, partData, string)

struct MimeEmail::MimePartList::Data {
    MultipartType multipartType = MultipartType::MIXED;
    std::vector<MimePartOrList> mimeParts;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_COPY_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_MOVE_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePartList)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(MimeEmail::MimePartList, multipartType, MimeEmail::MimePartList::MultipartType)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePartList, mimeParts, vector<MimeEmail::MimePartOrList>)

struct MimeEmail::MimePartOrList::Data {
    unique_ptr<MimePart> mimePart;
    unique_ptr<MimePartList> mimePartList;

    Data() = default;

    Data(Data const& other) {
        operator=(other);
    }

    Data& operator=(MimePart::Data const& otherMimePartData) {
        if (!mimePart) {
            mimePart = make_unique<MimePart>();
        }
        if (mimePartList) {
            mimePartList.reset(nullptr);
        }
        *mimePart->data = otherMimePartData;
        return *this;
    }

    Data& operator=(MimePartList::Data const& otherMimePartListData) {
        if (!mimePartList) {
            mimePartList = make_unique<MimePartList>();
        }
        if (mimePart) {
            mimePart.reset(nullptr);
        }
        *mimePartList->data = otherMimePartListData;
        return *this;
    }

    Data& operator=(Data const& other) {
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
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_COPY_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_MOVE_CONSTRUCTOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(MimeEmail, MimePartOrList)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePartOrList, mimePart, unique_ptr<MimeEmail::MimePart>)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(MimeEmail::MimePartOrList, mimePartList, unique_ptr<MimeEmail::MimePartList>)

MimeEmail::MimePartOrList::MimePartOrList(MimeEmail::MimePart const& otherMimePart) : MimePartOrList() {
    *data = *otherMimePart.data;
}

MimeEmail::MimePartOrList::MimePartOrList(MimeEmail::MimePartList const& otherMimePartList) : MimePartOrList() {
    *data = *otherMimePartList.data;
}

MimeEmail::MimePartOrList&
MimeEmail::MimePartOrList::operator=(MimeEmail::MimePart const& otherMimePart) {
    *data = *otherMimePart.data;
    return *this;
}

MimeEmail::MimePartOrList&
MimeEmail::MimePartOrList::operator=(MimeEmail::MimePartList const& otherMimePartList) {
    *data = *otherMimePartList.data;
    return *this;
}

string MimeEmail::getRaw(shared_ptr<ReplacementRules> const& replacementRules) const {
    stringstream ret;
    for (auto const& e : headers()) {
        if (e.first == "MIME-Version" || e.first == "Content-Type")
            continue;
        ret << e.first << ": " << e.second << "\r\n";
    }
    string boundary = genBoundary();
    ret << "MIME-Version: 1.0\r\nContent-Type: multipart/" << multipartTypeToString(data->mimePartList.multipartType());
    ret << "; boundary=\"" << boundary;
    ret << "\"\r\n\r\nThis is a multi-part message in MIME format\r\n\r\n";
    ret << mergeMimePartList(data->mimePartList, boundary, replacementRules);

    return ret.str();
}
