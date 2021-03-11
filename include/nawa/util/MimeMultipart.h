/**
 * \file MimeMultipart.h
 * \brief Parser for MIME multipart, especially in POST form data.
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

#ifndef NAWA_MIMEMULTIPART_H
#define NAWA_MIMEMULTIPART_H

#include <nawa/internal/macros.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace nawa {

    /**
     * Class for parsing MIME multipart data, usually transmitted through POST. Currently, only parsing (and not
     * creating) is supported, but it could be extended in future to create MIME for emails as well. The implementation
     * currently does NOT support nested MIME parts.
     */
    class MimeMultipart {
        NAWA_PRIVATE_DATA()

    public:
        using HeadersMap = std::unordered_map<std::string, std::string>;

        class Part {
            NAWA_PRIVATE_DATA()

        public:
            NAWA_DEFAULT_DESTRUCTOR_DEF(Part);

            NAWA_DEFAULT_CONSTRUCTOR_DEF(Part);

            NAWA_COPY_CONSTRUCTOR_DEF(Part);

            NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Part);

            NAWA_MOVE_CONSTRUCTOR_DEF(Part);

            NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Part);

            /**
             * Name of the MIME part.
             * @return Name of the MIME part.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(Part, partName, std::string);

            /**
             * The file name, if available (if this MIME part is a file). Otherwise empty.
             * @return The file name.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(Part, filename, std::string);

            /**
             * The content type of this MIME part.
             * @return The content type.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(Part, contentType, std::string);

            /**
             * Access a map containing this MIME part's headers.
             * @return Reference to the map of headers.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(Part, headers, HeadersMap);

            /**
             * Access the content of this MIME part.
             * @return Reference to the content of this MIME part.
             */
            NAWA_COMPLEX_DATA_ACCESSORS_DEF(Part, content, std::string);

            friend nawa::MimeMultipart;
        };

        NAWA_DEFAULT_DESTRUCTOR_DEF(MimeMultipart);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(MimeMultipart);

        NAWA_COPY_CONSTRUCTOR_DEF(MimeMultipart);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(MimeMultipart);

        NAWA_MOVE_CONSTRUCTOR_DEF(MimeMultipart);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(MimeMultipart);

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
         * Access the MIME parts generated by the parser (in future, this class may be able to generate MIME using
         * these parts, therefore, it is already possible to set them).
         * @return Reference to the generated parts.
         */
        NAWA_COMPLEX_DATA_ACCESSORS_DEF(MimeMultipart, parts, std::vector<Part>);

        /**
         * Clear the existing content in the container.
         */
        void clear();
    };
}

#endif //NAWA_MIMEMULTIPART_H
