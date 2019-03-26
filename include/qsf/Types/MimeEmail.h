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
#include <regex>

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
            bool isValid() {
                std::regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", std::regex::icase);
                return std::regex_match(address, emCheck);
            }
        };

        /**
         * Structure representing a basic email (just headers and payload, excluding the envelope).
         */
        struct SimpleEmail {
            /**
             * Map to save the mail headers in (case-sensitive). Headers From and Date are mandatory and must be set
             * automatically by the mail function if not specified in this map. Other fields that should be considered
             * are: To, Subject, Cc, Content-Type (will be set automatically if MIME is used).
             */
            std::unordered_map<std::string, std::string> headers;
            /**
             * The text part of the email.
             */
            std::string text;
        };

        /**
         * Structure representing an MIME email (headers and MIME parts, excluding the envelope).
         */
        struct MimeEmail {
            /**
             * Structure representing a MIME part of a MIME email.
             */
            struct MimePart {
                /**
                 * The Content-Type of this part.
                 */
                std::string contentType;
                /**
                 * The Content-Disposition part-header (e.g., "inline", or "attachment; filename=test.zip").
                 */
                std::string contentDisposition;
                /**
                 * Specify how the data string will be interpreted. PAYLOAD (default) means that the data string
                 * contains the payload of this MIME part (encoding will be applied by the mail handling function).
                 * FILENAME means that the data string contains a path to a file which should be read, encoded,
                 * and attached by the mail handling function.
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
            std::vector<MimePart> mimeParts;
            // TODO MIME, attachments, ...
            // smtp server etc. -> get from config? set here manually?
            // establish smtp persistent connection and allow to send multiple mails via this instance?
            //      (unlikely this would work with libcurl, but maybe an idea for the future)
        };
    }

}

#endif //QSF_EMAIL_H
