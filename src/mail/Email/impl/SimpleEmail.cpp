/**
 * \file SimpleEmail.cpp
 * \brief Implementation of the SimpleEmail class.
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

#include <nawa/mail/Email/impl/SimpleEmail.h>
#include <nawa/util/encoding.h>
#include <nawa/util/utils.h>
#include <sstream>

using namespace nawa;
using namespace std;

struct mail::SimpleEmail::Data {
    std::string text;
    bool quotedPrintableEncode = true;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(mail, SimpleEmail)

NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(mail, SimpleEmail)

NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL_WITH_NS(mail, SimpleEmail, Email)

NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(mail::SimpleEmail, Email)

NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL_WITH_NS(mail, SimpleEmail, Email)

NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(mail::SimpleEmail, Email)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(mail::SimpleEmail, text, string)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(mail::SimpleEmail, quotedPrintableEncode, bool)

string mail::SimpleEmail::getRaw(shared_ptr<ReplacementRules> const& replacementRules) const {
    stringstream ret;

    for (auto const& e : headers()) {
        if (e.first == "MIME-Version" || (data->quotedPrintableEncode && e.first == "Content-Transfer-Encoding"))
            continue;
        ret << e.first << ": " << e.second << "\r\n";
    }

    ret << "MIME-Version: 1.0\r\n";
    if (data->quotedPrintableEncode) {
        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
        ret << encoding::quotedPrintableEncode(
                replacementRules ? string_replace(data->text, *replacementRules) : data->text);
    } else {
        ret << "\r\n"
            << (replacementRules ? string_replace(data->text, *replacementRules) : data->text);
    }

    return ret.str();
}
