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

namespace nawa {
    /**
     * Structure representing a basic email (just headers and payload, excluding the envelope). Please remember to
     * also set the headers in the headers map of the base class Email.
     */
    struct SimpleEmail : public Email {
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
        std::string getRaw(const std::shared_ptr<ReplacementRules> &replacementRules) const override;
    };
}

#endif //NAWA_SIMPLEEMAIL_H
