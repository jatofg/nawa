/**
 * \file RequestInitContainer.h
 * \brief Container used by request handlers to initiate the nawa::Request object.
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

#ifndef NAWA_REQUESTINITCONTAINER_H
#define NAWA_REQUESTINITCONTAINER_H

#include <nawa/request/File.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace nawa {
    /**
     * Internal container filled by the RequestHandler with prerequisites for creating Connection and Request objects.
     */
    struct RequestInitContainer {
        /**
         * Environment variables, see \ref environmentmanual
         */
        std::unordered_map<std::string, std::string> environment;
        std::unordered_multimap<std::string, std::string> getVars; /**< The HTTP GET vars. */
        /**
         * The HTTP POST vars, only if it is in standard format (content type `multipart/form-data` or
         * `application/x-www-form-urlencoded`). Files are excluded and handled separately by fileVectorCallback.
         */
        std::unordered_multimap<std::string, std::string> postVars;
        std::unordered_multimap<std::string, std::string> cookieVars; /**< The HTTP COOKIE vars. */
        std::string postContentType;                                  /**< The HTTP POST content type. */
        std::unordered_multimap<std::string, File> postFiles;         /**< Files submitted via POST. */
        /**
         * A shared_ptr to a string which contains the raw POST data. Raw data does not have to be available
         * if the config option {"post", "raw_access"} is set to "never", or when it's set to "nonstandard" and the
         * POST content type is neither `multipart/form-data` nor `application/x-www-form-urlencoded`. In this case,
         * the shared_ptr should not contain an object.
         */
        std::shared_ptr<std::string> rawPost;
    };
}// namespace nawa

#endif//NAWA_REQUESTINITCONTAINER_H
