/**
 * \file Email.h
 * \brief Base structure for emails.
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

#ifndef NAWA_EMAIL_H
#define NAWA_EMAIL_H

#include <memory>
#include <nawa/internal/macros.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace nawa::mail {

    /**
     * Replacement rules for emails are just a string -> string map. All occurrences of the key string should be
     * replaced by the value string.
     */
    using ReplacementRules = std::unordered_map<std::string, std::string>;

    /**
     * Base structure for emails. To create an email, use the SimpleEmail or MimeEmail class.
     */
    class Email {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Email);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(Email);

        NAWA_COPY_CONSTRUCTOR_DEF(Email);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Email);

        NAWA_MOVE_CONSTRUCTOR_DEF(Email);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Email);

        using HeadersMap = std::unordered_map<std::string, std::string>;

        /**
         * Map to save the mail headers in (case-sensitive). Headers From and Date are mandatory and must be set
         * automatically by the mail function if not specified in this map. Other fields that should be considered
         * are: To, Subject, Cc, Content-Type (will be set automatically by MimeEmail). This map is designed to
         * be accessed by the mail function in order to complete it, if necessary. The toRaw() method might as well
         * include other headers.
         *
         * The email function is not obliged to (and should not) do any escaping in the headers. The application
         * creating the email is responsible for ensuring their validity.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(Email, headers, HeadersMap);

        /**
         * This method shall generate the raw source of the email (including headers).
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        [[nodiscard]] virtual std::string getRaw(std::shared_ptr<ReplacementRules> const& replacementRules) const = 0;
    };
}// namespace nawa::mail

#endif//NAWA_EMAIL_H
