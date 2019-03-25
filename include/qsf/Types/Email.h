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
         * Structure representing an email (just headers and payload, excluding the envelope).
         */
        struct Email {
            // From, To, Cc, Subject, Date, ...
            /**
             * Map to save the mail headers in (case-sensitive).
             */
            std::unordered_map<std::string, std::string> headers;
            /**
             * The content of the email.
             */
            std::string payload;
            // TODO MIME, attachments, ...
            // smtp server etc. -> get from config? set here manually?
            // establish smtp persistent connection and allow to send multiple mails via this instance?
            //      (unlikely this would work with libcurl, but maybe an idea for the future)
        };
    }

}

#endif //QSF_EMAIL_H
