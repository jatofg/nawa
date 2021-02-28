/**
 * \file FlushCallbackContainer.h
 * \brief Internal container passed to flush callback functions.
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

#ifndef NAWA_FLUSHCALLBACKCONTAINER_H
#define NAWA_FLUSHCALLBACKCONTAINER_H

#include <functional>
#include <string>
#include <unordered_map>

namespace nawa {
    /**
     * Internal container passed to the FlushCallbackFunction.
     */
    struct FlushCallbackContainer {
        unsigned int status; /**< The HTTP response status as an unsigned integer. */
        std::unordered_multimap<std::string, std::string> headers; /**< The multimap of response headers. */
        std::string body; /**< The response body. */
        bool flushedBefore; /**< True if the response has been flushed before, false otherwise. */

        /**
         * Get a textual representation of the HTTP status (such as "200 OK"). Implemented in Connection.cpp.
         * @return Textual representation of the HTTP status.
         */
        std::string getStatusString() const;

        /**
         * Generate a full raw HTTP source, including headers when flushing for the first time
         * (but without HTTP status). Implemented in Connection.cpp.
         * @return The HTTP source.
         */
        std::string getFullHttp() const;
    };

    using FlushCallbackFunction = std::function<void(FlushCallbackContainer)>;
}

#endif //NAWA_FLUSHCALLBACKCONTAINER_H
