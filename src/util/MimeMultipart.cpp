/**
 * \file MimeMultipart.cpp
 * \brief Implementation of the MimeMultipart class.
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

#include <boost/algorithm/string.hpp>
#include <nawa/Exception.h>
#include <nawa/util/MimeMultipart.h>
#include <nawa/util/Utils.h>
#include <regex>

using namespace nawa;
using namespace std;

MimeMultipart::MimeMultipart(const string &contentType, string content) {
    parse(contentType, move(content));
}

void MimeMultipart::parse(const string &contentType, string content) {
    regex findBoundary(R"X(boundary="?([A-Za-z0-9'()+_,\-.\/:=? ]+)"?)X");
    smatch boundaryMatch;
    if (!regex_search(contentType, boundaryMatch, findBoundary) || boundaryMatch.size() != 2) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Could not find boundary in content type.");
    }
    string boundary = "--";
    boundary += boundaryMatch[1];
    size_t boundaryLen = boundary.length();

    regex matchPartAndFileName(R"X((;| )name="?([^"]+)"?(; ?filename="?([^"]+)"?)?)X");
    auto extractPartAndFileName = [&](const string &contentDisposition) -> pair<string, string> {
        smatch sm;
        if (!regex_search(contentDisposition, sm, matchPartAndFileName) || sm.size() < 3) {
            return make_pair(string(), string());
        }
        return make_pair(sm[2], sm.size() >= 5 ? sm[4] : string());
    };

    // parse content
    while (!content.empty()) {

        // check for boundary
        if (content.length() < boundaryLen + 2 || content.substr(0, boundaryLen) != boundary) {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }
        content = content.substr(boundaryLen);

        // if followed by --, this is the end
        if (content.substr(0, 2) == "--") {
            break;
        }

        // newline must follow, and content must still have at least 2 (\r\n) + 1 (hdrs) + 2 (\r\n) + boundaryLen + 2 chars
        if (content.length() < boundaryLen + 6 || content.substr(0, 2) != "\r\n") {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }
        content = content.substr(2);

        // find next boundary
        size_t nextBoundaryPos = content.find(boundary);
        if (nextBoundaryPos == string::npos) {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }

        // headers section goes until the next \r\n\r\n or, alternatively, the next boundary
        size_t headersEndPos = content.find("\r\n\r\n");
        if (headersEndPos > nextBoundaryPos) {
            headersEndPos = nextBoundaryPos;
        }
        if (headersEndPos < 4) {
            throw Exception(__PRETTY_FUNCTION__, 2, "Malformed MIME payload.");
        }

        Part currentPart;
        currentPart.headers = parse_headers(content.substr(0, headersEndPos));
        currentPart.contentType = currentPart.headers.count("content-type") ? currentPart.headers.at("content-type")
                                                                            : "";
        if (currentPart.headers.count("content-disposition")) {
            tie(currentPart.partName, currentPart.fileName) = extractPartAndFileName(
                    currentPart.headers.at("content-disposition"));
        }

        // is there a part content in between? (headersEndPos + "\r\n\r\n" + content + "\r\n")
        if (nextBoundaryPos > headersEndPos + 7) {
            currentPart.content = content.substr(headersEndPos + 4, nextBoundaryPos - 2 - (headersEndPos + 4));
        }

        parts_.push_back(currentPart);
        content = content.substr(nextBoundaryPos);

    }

    contentType_ = contentType.substr(0, contentType.find_first_of(';'));
}

void MimeMultipart::clear() {
    contentType_.clear();
    parts_.clear();
}
