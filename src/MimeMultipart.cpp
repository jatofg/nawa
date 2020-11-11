/**
 * \file MimeMultipart.cpp
 * \brief Implementation of the MimeMultipart class.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
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

#include <nawa/MimeMultipart.h>
#include <regex>
#include <nawa/Exception.h>

using namespace nawa;
using namespace std;

MimeMultipart::MimeMultipart(string contentType, const string &content) {
    parse(move(contentType), content);
}

void MimeMultipart::parse(string contentType, const string &content) {
    regex findBoundary(R"X(boundary="?([A-Za-z0-9'()+_,\-.\/:=? ]+)"?)X");
    smatch boundaryMatch;
    if (!regex_search(contentType, boundaryMatch, findBoundary) || boundaryMatch.size() != 2) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Could not find boundary in content type.");
    }
    string boundary = boundaryMatch[1];
    // now parse the content...

    contentType_ = move(contentType);
}

void MimeMultipart::clear() {

}
