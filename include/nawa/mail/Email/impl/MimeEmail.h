/**
 * \file MimeEmail.h
 * \brief Structure representing a MIME email.
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

#ifndef NAWA_MIMEEMAIL_H
#define NAWA_MIMEEMAIL_H

#include <nawa/mail/Email/Email.h>
#include <string>
#include <unordered_map>

namespace nawa {
    /**
     * Structure representing a MIME email (headers and MIME parts, excluding the envelope).
     */
    class MimeEmail : public Email {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_OVERRIDE_DEF(MimeEmail);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(MimeEmail);

        NAWA_COPY_CONSTRUCTOR_DEF(MimeEmail);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(MimeEmail);

        NAWA_MOVE_CONSTRUCTOR_DEF(MimeEmail);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(MimeEmail);

        class MimePartOrList;

        /**
         * Structure representing a MIME part of a MIME email.
         */
        class MimePart {
            NAWA_PRIVATE_DATA()

        public:
            /**
             * Encodings which can be applied to the MIME part. Choose BASE64 for file attachments, and QUOTED_PRINTABLE
             * for HTML, etc. Use NONE if and only if you made sure that the part contains only valid characters.
             */
            enum class ApplyEncoding {
                BASE64,
                QUOTED_PRINTABLE,
                NONE
            }; /**< The encoding to apply to the MIME part, see nawa::MimeEmail::MimePart::ApplyEncoding. */

            NAWA_DEFAULT_DESTRUCTOR_DEF(MimePart);

            NAWA_DEFAULT_CONSTRUCTOR_DEF(MimePart);

            NAWA_COPY_CONSTRUCTOR_DEF(MimePart);

            NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(MimePart);

            NAWA_MOVE_CONSTRUCTOR_DEF(MimePart);

            NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(MimePart);

            /**
             * The Content-Type of this part (e.g., "text/plain; charset=utf-8").
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePart, contentType, std::string);

            /**
             * The Content-Disposition part-header (e.g., "inline", or "attachment; filename=test.zip"). Optional,
             * if this value is empty, the header will not be set.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePart, contentDisposition, std::string);

            /**
             * The encoding to apply to the MIME part, see nawa::MimeEmail::MimePart::ApplyEncoding.
             * Default: quoted-printable.
             */
            NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(MimePart, applyEncoding, ApplyEncoding);

            /**
             * Additional headers for this MIME part (such as Content-ID) can be added here.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePart, partHeaders, HeadersMap);

            /**
             * Whether to allow replacements using ReplacementRules in the data of this MIME part. The replacements
             * will be applied before the encoding. Default: false.
             */
            NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(MimePart, allowReplacements, bool);

            /**
             * The data string, containing the body of the MIME part.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePart, partData, std::string);

            friend MimePartOrList;
        };

        class MimePartList;

        /**
         * This struct can save either a MimePart or a MimePartList object and acts as a basis for the MimePartList.
         * You don't have to deal with this object directly, you can just use the MimePartList::mimePart.emplace_back()
         * function to add a MimePart or another MimePartList object to the list.
         */
        class MimePartOrList {
            NAWA_PRIVATE_DATA()

        public:
            NAWA_DEFAULT_DESTRUCTOR_DEF(MimePartOrList);

            NAWA_DEFAULT_CONSTRUCTOR_DEF(MimePartOrList);

            NAWA_COPY_CONSTRUCTOR_DEF(MimePartOrList);

            NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(MimePartOrList);

            NAWA_MOVE_CONSTRUCTOR_DEF(MimePartOrList);

            NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(MimePartOrList);

            /**
             * Construct a MimePartOrList from a MimePart (also implicitly).
             * @param _mimePart The MimePart object this MimePartOrList should refer to.
             */
            MimePartOrList(MimePart const& _mimePart);

            /**
             * Construct a MimePartOrList from a MimePartList (also implicitly).
             * @param _mimePartList The MimePartList object this MimePartOrList should refer to.
             */
            MimePartOrList(MimePartList const& _mimePartList);

            /**
             * Assign a MimePart object to this MimePartOrList.
             * @param otherMimePart Object to assign.
             * @return This object.
             */
            MimePartOrList& operator=(MimePart const& otherMimePart);

            /**
             * Assign a MimePartList object to this MimePartOrList.
             * @param otherMimePartList Object to assign.
             * @return This object.
             */
            MimePartOrList& operator=(MimePartList const& otherMimePartList);

            /**
             * Create a MIME part containing data. If this pointer contains a MimePart object, the second pointer
             * (for another MimePartList object) will be ignored (and not copied together with the object). Please
             * use the assignment operator for assigning a new object, that function will make sure that the old
             * object will be destroyed properly.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePartOrList, mimePart, std::unique_ptr<MimePart>);

            /**
             * Create a MIME part containing another MIME container with (possibly) multiple MIME parts. Please
             * use the assignment operator for assigning a new object, that function will make sure that the old
             * object will be destroyed properly.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePartOrList, mimePartList, std::unique_ptr<MimePartList>);
        };

        /**
         * A list containing MIME parts, which can also be lists of MIME parts themselves (nested).
         */
        class MimePartList {
            NAWA_PRIVATE_DATA()

        public:
            /**
             * Possible types of MIME containers (content-type, e.g., `multipart/mixed` for independent parts, or
             * `multipart/alternative` for alternatives like plain text and HTML). See Wikipedia/MIME for details.
             */
            enum class MultipartType {
                MIXED,
                DIGEST,
                ALTERNATIVE,
                RELATED,
                REPORT,
                SIGNED,
                ENCRYPTED
            };

            NAWA_DEFAULT_DESTRUCTOR_DEF(MimePartList);

            NAWA_DEFAULT_CONSTRUCTOR_DEF(MimePartList);

            NAWA_COPY_CONSTRUCTOR_DEF(MimePartList);

            NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(MimePartList);

            NAWA_MOVE_CONSTRUCTOR_DEF(MimePartList);

            NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(MimePartList);

            /**
             * The type of this MIME container, see nawa::MimeEmail::MimePartList::MultipartType. Default: MIXED/
             */
            NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(MimePartList, multipartType, MultipartType);

            /**
             * The list of MIME parts. The MimePartOrList type allows nesting, it can contain either a "final"
             * MIME part with payload, or another nested list of MIME parts.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimePartList, mimeParts, std::vector<MimePartOrList>);

            friend MimePartOrList;
        };

        /**
         * List containing all MIME parts that should be included in this email. It should contain at least one
         * text (or HTML) part.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimeEmail, mimePartList, MimePartList);

        /**
         * Get the raw source of the email.
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        [[nodiscard]] std::string getRaw(std::shared_ptr<ReplacementRules> const& replacementRules) const override;
    };
}// namespace nawa

#endif//NAWA_MIMEEMAIL_H
