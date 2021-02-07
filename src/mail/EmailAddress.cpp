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
#include <sstream>
#include <regex>

using namespace nawa;
using namespace std;

string EmailAddress::get(bool includeName) const {
    stringstream ret;
    if (includeName) {
        ret << name << " ";
    }
    ret << '<' << address << '>';
    return ret.str();
}

bool EmailAddress::isValid() const {
    regex emCheck(R"([a-z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-z0-9.-]+)", regex::icase);
    return regex_match(address, emCheck);
}
