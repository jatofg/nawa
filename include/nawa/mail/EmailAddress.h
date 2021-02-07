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

#include <string>

namespace nawa {
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
