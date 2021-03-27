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
#include <nawa/util/encoding.h>
#include <nawa/util/utils.h>
#include <regex>
#include <sstream>

using namespace nawa;
using namespace std;

namespace {
    string applyPunycodeToDomain(string const& address) {
        auto partsOfAddr = utils::splitString(address, '@', false);
        if (partsOfAddr.size() != 2) {
            return address;
        }
        partsOfAddr[1] = encoding::punycodeEncode(partsOfAddr[1]);
        return partsOfAddr[0] + "@" + partsOfAddr[1];
    }
}// namespace

struct mail::EmailAddress::Data {
    string name;    /**< The name of the sender or recipient. */
    string address; /**< The email address itself. */
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(mail, EmailAddress)

NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(mail, EmailAddress)

NAWA_COPY_CONSTRUCTOR_IMPL_WITH_NS(mail, EmailAddress)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(mail::EmailAddress)

NAWA_MOVE_CONSTRUCTOR_IMPL_WITH_NS(mail, EmailAddress)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(mail::EmailAddress)

mail::EmailAddress::EmailAddress(string address) : EmailAddress() {
    data->address = move(address);
}

mail::EmailAddress::EmailAddress(string name, string address) : EmailAddress() {
    data->name = move(name);
    data->address = move(address);
}

string mail::EmailAddress::get(bool includeName, bool applyPunycode) const {
    stringstream ret;
    if (includeName && !data->name.empty()) {
        ret << data->name << " ";
    }
    string address = data->address;
    if (applyPunycode) {
        address = applyPunycodeToDomain(address);
    }
    ret << '<' << address << '>';
    return ret.str();
}

bool mail::EmailAddress::isValid() const {
    string address = applyPunycodeToDomain(data->address);
    if (address.empty()) {
        return false;
    }
    regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", regex::icase);
    return regex_match(address, emCheck);
}

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(mail::EmailAddress, name, string)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(mail::EmailAddress, address, string)
