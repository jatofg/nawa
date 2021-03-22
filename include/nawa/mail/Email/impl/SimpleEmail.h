/**
 * \file SimpleEmail.h
 * \brief Structure representing a basic email.
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

#ifndef NAWA_SIMPLEEMAIL_H
#define NAWA_SIMPLEEMAIL_H

#include <memory>
#include <nawa/mail/Email/Email.h>
#include <string>

namespace nawa::mail {
    /**
     * Structure representing a basic email (just headers and payload, excluding the envelope). Please remember to
     * also set the headers in the headers map of the base class Email.
     */
    class SimpleEmail : public Email {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_OVERRIDE_DEF(SimpleEmail);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(SimpleEmail);

        NAWA_COPY_CONSTRUCTOR_DEF(SimpleEmail);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(SimpleEmail);

        NAWA_MOVE_CONSTRUCTOR_DEF(SimpleEmail);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(SimpleEmail);

        /**
         * The text part of the email.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(SimpleEmail, text, std::string);

        /**
         * Whether to apply quoted-printable encoding to the content of the email (recommended). This will also take
         * care of the line lengths and set the header Content-Transfer-Encoding. Default: true.
         */
        NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(SimpleEmail, quotedPrintableEncode, bool);

        /**
         * Get the raw source of the email.
         * @param replacementRules Replacements that shall be applied in all suitable (body) parts of the email.
         * @return Raw source of the email.
         */
        [[nodiscard]] std::string getRaw(std::shared_ptr<ReplacementRules> const& replacementRules) const override;
    };
}// namespace nawa::mail

#endif//NAWA_SIMPLEEMAIL_H
