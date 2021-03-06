/**
 * \file MimeMultipart.h
 * \brief Parser for MIME multipart, especially in POST form data.
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

#ifndef NAWA_MIMEMULTIPART_H
#define NAWA_MIMEMULTIPART_H

#include <string>
#include <vector>
#include <unordered_map>

namespace nawa {

    /**
     * Class for parsing MIME multipart data, usually transmitted through POST. Currently, only parsing (and not
     * creating) is supported, but it could be extended in future to create MIME for emails as well. The implementation
     * currently does NOT support nested MIME parts.
     */
    struct MimeMultipart {
        struct Part {
            std::string partName;
            std::string fileName;
            std::string contentType;
            std::unordered_map<std::string, std::string> headers;
            std::string content;
        };
        std::string contentType_;
        std::vector<Part> parts_;

        /**
         * Construct an empty MimeMultipart container.
         */
        MimeMultipart() = default;

        /**
         * Construct a MimeMultipart container and parse content. Throws a nawa::Exception in case
         * of a parsing error (see parse() for details).
         * @param contentType Content type of the data in content, including the boundary.
         * @param content A MIME multipart source according to RFC 2046.
         */
        MimeMultipart(const std::string &contentType, std::string content);

        /**
         * Parse content into the MimeMultipart container. Clears the existing content before. Throws a nawa::Exception
         * in case of a parsing error. Error codes:
         * - 1: Could not find boundary in content type.
         * - 2: Malformed MIME payload.
         * @param contentType Content type of the data in content, including the boundary.
         * @param content A MIME multipart source according to RFC 2046.
         */
        void parse(const std::string &contentType, std::string content);

        /**
         * Clear the existing content in the container.
         */
        void clear();
    };
}

#endif //NAWA_MIMEMULTIPART_H
