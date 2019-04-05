/**
 * \file Email.h
 * \brief Types representing email addresses and emails.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_EMAIL_H
#define QSF_EMAIL_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Qsf {
    namespace Types {
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
            std::string get(bool includeName = true) {
                return std::string();
            }
            /**
             * Perform a very basic, regex-based validity check on the saved email address. This will only check that it
             * contains an \@ symbol and only valid characters before and after it, so if this function returns true,
             * that does not necessarily mean the email address is valid. To really validate an email address, you should
             * send a confirmation email to it.
             * @return True if the email address could be valid, false if it is definitely not valid.
             */
            bool isValid();
        };

        /**
         * Pure virtual base structure for emails. To create an email, use the SimpleEmail or MimeEmail class.
         */
        struct Email {
            /**
             * Map to save the mail headers in (case-sensitive). Headers From and Date are mandatory and must be set
             * automatically by the mail function if not specified in this map. Other fields that should be considered
             * are: To, Subject, Cc, Content-Type (will be set automatically if MIME is used). This map is designed to
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
             * @return Raw source of the email.
             */
            virtual std::string toRaw() = 0;
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
             * Get the raw source of the email.
             * @return Raw source of the email.
             */
            std::string toRaw() override;
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
                 * Specify how the data string will be interpreted. PAYLOAD (default) means that the data string
                 * contains the payload of this MIME part.
                 * FILENAME means that the data string contains a path to a file which should be read, encoded,
                 * and attached.
                 */
                enum DataType {
                    FILENAME,
                    PAYLOAD
                } dataType = PAYLOAD;
                /**
                 * The data string, containing either the payload of the MIME part or the path to a file containing it,
                 * according to the dataType property.
                 */
                std::string data;
            };

            struct MimePartList;

            /**
             * As a MIME part of an email can be nested, this struct allows you to either create a MIME part
             * containing data, or a list of MIME parts - in the latter case, a MIME part containing another list of
             * MIME parts will be created.
             */
            struct MimePartOrList {
                /**
                 * Make sure default constructor exists.
                 */
                MimePartOrList() = default;
                /**
                 * Copy constructor, as this struct contains smart pointers.
                 * @param other Object to copy.
                 */
                MimePartOrList(const MimePartOrList& other) {
                    if(other.mimePart) {
                        mimePart = std::make_unique<MimePart>();
                        *mimePart = *(other.mimePart);
                    }
                    else if(other.mimePartList) {
                        mimePartList = std::make_unique<MimePartList>();
                        *mimePartList = *(other.mimePartList);
                    }
                }
                /**
                 * Create a MIME part containing data. If this pointer contains a MimePart object, the second pointer
                 * (for another MimePartList object) will be ignored (and not copied together with the object).
                 */
                std::unique_ptr<MimePart> mimePart;
                /**
                 * Create a MIME part containing another MIME container with (possibly) multiple MIME parts.
                 */
                std::unique_ptr<MimePartList> mimePartList;
            };

            struct MimePartList {
                enum MultipartType {
                    MIXED,
                    DIGEST,
                    ALTERNATIVE,
                    RELATED,
                    REPORT,
                    SIGNED,
                    ENCRYPTED
                } multipartType = MIXED;
                std::vector<MimePartOrList> mimeParts;
            };

            // From, To, Cc, Subject, Date, ...
            /**
             * Map to save the mail headers in (case-sensitive). Headers From and Date are mandatory and must be set
             * automatically by the mail function if not specified in this map. Other fields that should be considered
             * are: To, Subject, Cc, Content-Type (will be set automatically if MIME is used).
             */
            std::unordered_map<std::string, std::string> headers;
            /**
             * Vector containing all MIME parts that should be included in this email. It should contain at least one
             * text (or HTML) part.
             */
            std::vector<MimePartList> mimeParts;
            /**
             * Get the raw source of the email.
             * @return Raw source of the email.
             */
            std::string toRaw() override;
        };
    }

}

#endif //QSF_EMAIL_H
