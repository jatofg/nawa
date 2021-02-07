/**
 * \file SessionData.h
 * \brief Structure for storing session data.
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

#ifndef NAWA_SESSIONDATA_H
#define NAWA_SESSIONDATA_H

#include <any>
#include <mutex>
#include <string>
#include <unordered_map>

namespace nawa {
    /**
     * SessionData objects contain all data of one session.
     */
    struct SessionData {
        std::mutex dLock; /**< Lock for data. */
        std::mutex eLock; /**< Lock for expires.  */
        std::unordered_map<std::string, std::any> data; /**< Map containing all values of this session. */
        time_t expires; /**< Time when this session expires. */
        const std::string sourceIP; /**< IP address of the session initiator, for optional IP checking. */
        /**
         * Construct an empty SessionData object without a source IP.
         */
        SessionData() : expires(0) {}

        /**
         * Construct an empty SessionData object with a source IP.
         * @param sIP IP address of the session initiator.
         */
        explicit SessionData(std::string sIP) : expires(0), sourceIP(std::move(sIP)) {}
    };
}

#endif //NAWA_SESSIONDATA_H
