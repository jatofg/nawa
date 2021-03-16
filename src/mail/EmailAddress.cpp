/**
 * \file EmailAddress.cpp
 * \brief Implementation of the EmailAddress class.
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

#include <nawa/mail/EmailAddress.h>
#include <regex>
#include <sstream>

using namespace nawa;
using namespace std;

struct EmailAddress::Data {
    string name;    /**< The name of the sender or recipient. */
    string address; /**< The email address itself. */
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(EmailAddress)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(EmailAddress)

NAWA_COPY_CONSTRUCTOR_IMPL(EmailAddress)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(EmailAddress)

NAWA_MOVE_CONSTRUCTOR_IMPL(EmailAddress)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(EmailAddress)

EmailAddress::EmailAddress(string address) : EmailAddress() {
    data->address = move(address);
}

EmailAddress::EmailAddress(string name, string address) : EmailAddress() {
    data->name = move(name);
    data->address = move(address);
}

string EmailAddress::get(bool includeName) const {
    stringstream ret;
    if (includeName) {
        ret << data->name << " ";
    }
    ret << '<' << data->address << '>';
    return ret.str();
}

bool EmailAddress::isValid() const {
    regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", regex::icase);
    return regex_match(data->address, emCheck);
}

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(EmailAddress, name, string)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(EmailAddress, address, string)
