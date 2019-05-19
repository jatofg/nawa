/**
 * \file Email.h
 * \brief Types representing email addresses and emails.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SORU_EMAIL_H
#define SORU_EMAIL_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace soru {

    /**
     * Replacement rules for emails are just a string -> string map. All occurrences of the key string should be
     * replaced by the value string.
     */
    typedef std::unordered_map<std::string, std::string> ReplacementRules;

    /**
     * Structure containing the name and email address of a recipient or sender. It contains functions for validation
     * and getting a representation in a standard format that can be used for curl.
     */
    struct EmailAddress {
        std::string name; /**< The name of the sender or recipient. */
        std::string address; /**< The email address itself. */
        /**
         * Construct an empty EmailAddress object.
         */
        EmailAddress() = default;
        /**
         * Construct an EmailAddress object and initialize it with an email address.
         * @param _address Email address.
         */
        explicit EmailAddress(std::string _address) {
            address = std::move(_address);
        }
        /**
         * Construct an EmailAddress object and initialize it with a recipient (or sender) name and email address.
         * @param _name Name.
         * @param _address Email address.
         */
        EmailAddress(std::string _name, std::string _address) {
            name = std::move(_name);
            address = std::move(_address);
        }
        /**
         * Get the email address in a standard representation, i.e., `<john.doe\@example.com>` without name, or
         * `John Doe <john.doe\@example.com>` with name included.
         * @param includeName Include the name in the representation.
         * @return String representation.
         */
        std::string get(bool includeName = true) const;
        /**
         * Perform a very basic, regex-based validity check on the saved email address. This will only check that it
         * contains an \@ symbol and only valid characters before and after it, so if this function returns true,
         * that does not necessarily mean the email address is valid. To really validate an email address, you should
         * send a confirmation email to it.
         * @return True if the email address could be valid, false if it is definitely not valid.
         */
        bool isValid() const;
    };

    /**
     * Base structure for emails. To create an email, use the SimpleEmail or MimeEmail class.
     */
    struct Email {
        /**
         * Map to save the mail headers in (case-sensitive). Headers From and Date are mandatory and must be set
         * automatically by the mail function if not specified in this map. Other fields that should be considered
         * are: To, Subject, Cc, Content-Type (will be set automatically by MimeEmail). This map is designed to
         * be accessed by the mail function in order to complete it, if necessary. The toRaw() method might as well
         * include other headers.
         *
         * The email function is not obliged to (and should not) do any escaping in the headers. The application
         * creating the email is responsible for ensuring their validity.
         */
        std::unordered_map<std::string, std::string> headers;

        // TODO offer encoded-word in headers like subject to make it easier

        /**
         * This method shall generate the raw source of the email (including headers).
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        virtual std::string getRaw(const ReplacementRules &replacementRules) const = 0;
    };

    /**
     * Structure representing a basic email (just headers and payload, excluding the envelope). Please remember to
     * also set the headers in the headers map of the base class Email.
     */
    struct SimpleEmail: public Email {
        /**
         * The text part of the email.
         */
        std::string text;

        /**
         * Whether to apply quoted-printable encoding to the content of the email (recommended). This will also take
         * care of the line lengths and set the header Content-Transfer-Encoding.
         */
        bool quotedPrintableEncode = true;

        /**
         * Get the raw source of the email.
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        std::string getRaw(const ReplacementRules &replacementRules) const override;
    };

    /**
     * Structure representing a MIME email (headers and MIME parts, excluding the envelope).
     */
    struct MimeEmail: public Email {
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
             * The encoding to apply to the MIME part. Choose BASE64 for file attachments, and QUOTED_PRINTABLE
             * for HTML, etc. Use NONE if and only if you made sure that the part contains only valid characters.
             */
            enum ApplyEncoding {
                BASE64,
                QUOTED_PRINTABLE,
                NONE
            } applyEncoding = QUOTED_PRINTABLE;
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
            MimePartOrList(const MimePart& _mimePart);
            /**
             * Construct a MimePartOrList from a MimePartList (also implicitly).
             * @param _mimePartList The MimePartList object this MimePartOrList should refer to.
             */
            MimePartOrList(const MimePartList& _mimePartList);
            /**
             * Copy constructor, as this struct contains smart pointers.
             * @param other Object to copy.
             */
            MimePartOrList(const MimePartOrList& other);
            /**
             * Copy assignment operator, as this struct contains smart pointers.
             * @param other Object to copy from.
             * @return This object.
             */
            MimePartOrList& operator=(const MimePartOrList& other);
            /**
             * Assign a MimePart object to this MimePartOrList.
             * @param _mimePart Object to assign.
             * @return This object.
             */
            MimePartOrList& operator=(const MimePart& _mimePart);
            /**
             * Assign a MimePartList object to this MimePartOrList.
             * @param _mimePartList Object to assign.
             * @return This object.
             */
            MimePartOrList& operator=(const MimePartList& _mimePartList);
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
             * The type of this MIME container (content-type, e.g., `multipart/mixed` for independent parts, or
             * `multipart/alternative` for alternatives like plain text and HTML). See Wikipedia/MIME for details.
             */
            enum MultipartType {
                MIXED,
                DIGEST,
                ALTERNATIVE,
                RELATED,
                REPORT,
                SIGNED,
                ENCRYPTED
            } multipartType = MIXED;
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
        std::string getRaw(const ReplacementRules &replacementRules) const override;
        // TODO add extra functions, e.g., for adding an attachment or easily creating alternative text and html
        //   parts (attachments read from a file, can use util function get_file_contents)
    };

}

#endif //SORU_EMAIL_H
