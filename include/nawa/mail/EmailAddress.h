/**
 * \file EmailAddress.h
 * \brief Structure representing an email address.
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

#ifndef NAWA_EMAILADDRESS_H
#define NAWA_EMAILADDRESS_H

#include <nawa/internal/macros.h>
#include <string>

namespace nawa {
    /**
     * Structure containing the name and email address of a recipient or sender. It contains functions for validation
     * and getting a representation in a standard format that can be used for curl.
     */
    class EmailAddress {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(EmailAddress);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(EmailAddress);

        NAWA_COPY_CONSTRUCTOR_DEF(EmailAddress);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(EmailAddress);

        NAWA_MOVE_CONSTRUCTOR_DEF(EmailAddress);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(EmailAddress);

        /**
         * Construct an EmailAddress object and initialize it with an email address.
         * @param address Email address.
         */
        explicit EmailAddress(std::string address);

        /**
         * Construct an EmailAddress object and initialize it with a recipient (or sender) name and email address.
         * @param name Name.
         * @param address Email address.
         */
        EmailAddress(std::string name, std::string address);

        /**
         * The name which is part of the email address.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(EmailAddress, name, std::string);

        /**
         * The email address.
         * @return Reference to element.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(EmailAddress, address, std::string);

        /**
         * Get the email address in a standard representation, i.e., `<john.doe\@example.com>` without name, or
         * `John Doe <john.doe\@example.com>` with name included.
         * @param includeName Include the name in the representation.
         * @return String representation.
         */
        [[nodiscard]] std::string get(bool includeName = true) const;

        /**
         * Perform a very basic, regex-based validity check on the saved email address. This will only check that it
         * contains an \@ symbol and only valid characters before and after it, so if this function returns true,
         * that does not necessarily mean the email address is valid. To really validate an email address, you should
         * send a confirmation email to it.
         * @return True if the email address could be valid, false if it is definitely not valid.
         */
        [[nodiscard]] bool isValid() const;
    };
}

#endif //NAWA_EMAILADDRESS_H
