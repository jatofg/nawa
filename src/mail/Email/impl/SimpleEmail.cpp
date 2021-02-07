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
#include <nawa/util/Encoding.h>
#include <nawa/util/Utils.h>
#include <sstream>

using namespace nawa;
using namespace std;

string SimpleEmail::getRaw(const shared_ptr<ReplacementRules> &replacementRules) const {
    stringstream ret;

    for (auto const &e: headers) {
        if (e.first == "MIME-Version" || (quotedPrintableEncode && e.first == "Content-Transfer-Encoding"))
            continue;
        ret << e.first << ": " << e.second << "\r\n";
    }

    ret << "MIME-Version: 1.0\r\n";
    if (quotedPrintableEncode) {
        ret << "Content-Transfer-Encoding: quoted-printable\r\n\r\n";
        ret << Encoding::quotedPrintableEncode(
                replacementRules ? string_replace(text, *replacementRules) : text
        );
    } else {
        ret << "\r\n" << (replacementRules ? string_replace(text, *replacementRules) : text);
    }

    return ret.str();
}