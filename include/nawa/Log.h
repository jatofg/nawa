/**
 * \file Log.h
 * \brief Simple class for (not (yet) thread-safe) logging to stderr or to any other output stream
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

#ifndef NAWA_LOG_H
#define NAWA_LOG_H

#include <iostream>
#include <fstream>

namespace nawa {
    /**
     * Simple class for thread-safe logging to stderr or to any other output stream. This class uses the same ostream
     * for logging in all instances and synchronizes the output. Every instance can have its own app name.
     * By default, the logger will write to std::cout.
     */
    class Log {
        std::string appname; /**< Name of the current app, appears in brackets in the log. */
    public:
        /**
         * Construct a Log object with the default app name nawa (can be changed later).
         */
        Log() noexcept;

        /**
         * Construct a Log object with a custom app name (can be changed later).
         * @param appname The app name, appears in brackets in the log.
         */
        explicit Log(std::string appname) noexcept;

        Log(const Log &other) noexcept;

        Log &operator=(const Log &other) noexcept;

        virtual ~Log();

        /**
         * Change the output stream to the specified one. This change will be permanent until every active Log
         * object has been destructed. If the output stream is locked, this function will have no effect, and throw
         * no exception (make sure to check isLocked() first).
         * 
         * This function is not thread-safe as long as the stream is unlocked. Please lock the stream before using 
         * the logger in a multi-threaded environment.
         * @param os Pointer to the output stream. Make sure that this stream will be available until NAWA terminates.
         */
        static void setStream(std::ostream *os) noexcept;

        /**
         * Change the output to append to the specified log file. Will throw a nawa::Exception with error code 1 if
         * the requested file cannot be opened for writing. This change will be permanent until every active Log
         * object has been destructed. If the output stream is locked, this function will have no effect, and throw
         * no exception (make sure to check isLocked() first).
         * 
         * This function is not thread-safe as long as the stream is unlocked. Please lock the stream before using 
         * the logger in a multi-threaded environment.
         * @param filename Path to the log file.
         */
        static void setOutfile(const std::string &filename);

        /**
         * Lock the output stream. It will not be possible to change the output stream of the logger anymore as long as
         * there is at least one active Log object. If the output stream is already locked, this will have no effect.
         * This operation is not reversible. Please run this function before multiple threads might construct Log
         * objects, as setting the output stream/file is not thread-safe.
         */
        static void lockStream() noexcept;

        /**
         * Check whether the output stream is locked.
         * @return True if locked, false if not.
         */
        static bool isLocked() noexcept;

        /**
         * Set the app name to use in debugging output. Defaults to nawa, but can be changed in order to use this
         * logging class inside of NAWA apps.
         * @param appname The app name, which will be include in the log in brackets.
         */
        void setAppname(std::string appname) noexcept;

        /**
         * Write a message to the log.
         * @param msg The message.
         */
        void write(const std::string &msg);

        /**
         * Write a message to the log.
         * @param msg The message.
         */
        void operator()(const std::string &msg);
    };
}

#endif //NAWA_LOG_H
