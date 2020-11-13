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

MimeMultipart::MimeMultipart(string contentType, string content) {
    parse(move(contentType), move(content));
}

void MimeMultipart::parse(string contentType, string content) {
    regex findBoundary(R"X(boundary="?([A-Za-z0-9'()+_,\-.\/:=? ]+)"?)X");
    smatch boundaryMatch;
    if (!regex_search(contentType, boundaryMatch, findBoundary) || boundaryMatch.size() != 2) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Could not find boundary in content type.");
    }
    string boundary = "--";
    boundary += boundaryMatch[1];

    // parse content
    while (!content.empty()) {

        // check for boundary
        if (content.substr(0, boundary.length()) != boundary) {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }
        content = content.substr(boundary.length());
        string nextChars = content.substr(0, 2);

        // if followed by --, this is the end
        if (nextChars == "--") {
            break;
        }
        // \r\n should follow
        if (nextChars != "\r\n") {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }
        content = content.substr(2);

        // now, we expect headers

        // find next boundary
        size_t nextBoundaryPos = content.find_first_of(boundary);
    }

    contentType_ = move(contentType);
}

void MimeMultipart::clear() {

}
