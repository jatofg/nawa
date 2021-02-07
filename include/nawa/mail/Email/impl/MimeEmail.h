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
    struct MimeEmail : public Email {
        /**
         * Structure representing a MIME part of a MIME email.
         */
        struct MimePart {
            /**
             * The Content-Type of this part (e.g., "text/plain; charset=utf-8").
             */
            std::string contentType;
            /**
             * The Content-Disposition part-header (e.g., "inline", or "attachment; filename=test.zip"). Optional,
             * if this value is empty, the header will not be set.
             */
            std::string contentDisposition;
            /**
             * Encodings which can be applied to the MIME part. Choose BASE64 for file attachments, and QUOTED_PRINTABLE
             * for HTML, etc. Use NONE if and only if you made sure that the part contains only valid characters.
             */
            enum ApplyEncoding {
                BASE64,
                QUOTED_PRINTABLE,
                NONE
            } applyEncoding = QUOTED_PRINTABLE; /**< The encoding to apply to the MIME part, see nawa::MimeEmail::MimePart::ApplyEncoding. */
            /**
             * Additional headers for this MIME part (such as Content-ID) can be added here.
             */
            std::unordered_map<std::string, std::string> partHeaders;
            /**
             * Whether to allow replacements using ReplacementRules in the data of this MIME part. The replacements
             * will be applied before the encoding.
             */
            bool allowReplacements = false;
            /**
             * The data string, containing the body of the MIME part.
             */
            std::string data;
        };

        struct MimePartList;

        /**
         * This struct can save either a MimePart or a MimePartList object and acts as a basis for the MimePartList.
         * You don't have to deal with this object directly, you can just use the MimePartList::mimePart.emplace_back()
         * function to add a MimePart or another MimePartList object to the list.
         */
        struct MimePartOrList {
            /**
             * Make sure default constructor exists.
             */
            MimePartOrList() = default;

            /**
             * Construct a MimePartOrList from a MimePart (also implicitly).
             * @param _mimePart The MimePart object this MimePartOrList should refer to.
             */
            MimePartOrList(const MimePart &_mimePart);

            /**
             * Construct a MimePartOrList from a MimePartList (also implicitly).
             * @param _mimePartList The MimePartList object this MimePartOrList should refer to.
             */
            MimePartOrList(const MimePartList &_mimePartList);

            /**
             * Copy constructor, as this struct contains smart pointers.
             * @param other Object to copy.
             */
            MimePartOrList(const MimePartOrList &other);

            /**
             * Copy assignment operator, as this struct contains smart pointers.
             * @param other Object to copy from.
             * @return This object.
             */
            MimePartOrList &operator=(const MimePartOrList &other);

            /**
             * Assign a MimePart object to this MimePartOrList.
             * @param _mimePart Object to assign.
             * @return This object.
             */
            MimePartOrList &operator=(const MimePart &_mimePart);

            /**
             * Assign a MimePartList object to this MimePartOrList.
             * @param _mimePartList Object to assign.
             * @return This object.
             */
            MimePartOrList &operator=(const MimePartList &_mimePartList);

            /**
             * Create a MIME part containing data. If this pointer contains a MimePart object, the second pointer
             * (for another MimePartList object) will be ignored (and not copied together with the object). Please
             * use the assignment operator for assigning a new object, that function will make sure that the old
             * object will be destroyed properly.
             */
            std::unique_ptr<MimePart> mimePart;
            /**
             * Create a MIME part containing another MIME container with (possibly) multiple MIME parts. Please
             * use the assignment operator for assigning a new object, that function will make sure that the old
             * object will be destroyed properly.
             */
            std::unique_ptr<MimePartList> mimePartList;
        };

        /**
         * A list containing MIME parts, which can also be lists of MIME parts themselves (nested).
         */
        struct MimePartList {
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
            } multipartType = MultipartType::MIXED; /**< The type of this MIME container, see nawa::MimeEmail::MimePartList::MultipartType. */
            /**
             * The list of MIME parts. The MimePartOrList type allows nesting, it can contain either a "final"
             * MIME part with payload, or another nested list of MIME parts.
             */
            std::vector<MimePartOrList> mimeParts;
        };

        /**
         * List containing all MIME parts that should be included in this email. It should contain at least one
         * text (or HTML) part.
         */
        MimePartList mimePartList;

        /**
         * Get the raw source of the email.
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        std::string getRaw(const std::shared_ptr<ReplacementRules> &replacementRules) const override;
    };
}

#endif //NAWA_MIMEEMAIL_H
