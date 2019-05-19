/**
 * \file Log.h
 * \brief Simple class for (not (yet) thread-safe) logging to stderr or to any other output stream
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_LOG_H
#define QSF_LOG_H

#include <iostream>
#include <fstream>

namespace soru {
    // TODO also 'improve' logging (change it to the correct format) in fastcgi++
    /**
     * Simple class for (not (yet) thread-safe) logging to stderr or to any other output stream
     */
    class Log {
        std::ostream* out; /**< Stream to send the logging output to. */
        std::ofstream logFile; /**< Log file handle in case a file is used and managed by this class. */
        // TODO synchronization with a mutex? (does not make sense right now, as there is no global object)
        std::string hostname;
        __pid_t pid = 0;
    public:
        /**
         * Construct a Log object that writes to std::cerr by default.
         */
        Log();
        /**
         * Construct a Log object that writes to the specified output stream by default.
         * @param os Pointer to the output stream. Make sure that this stream will be available during the full lifetime
         * of the Session object.
         */
        explicit Log(std::ostream* os);
        /**
         * Construct a Log object that appends to the specified log file. This function will invoke setOutfile() and
         * may therefore throw exceptions.
         * @param filename Path to the log file.
         */
        explicit Log(std::string filename);
        // TODO implement copying if required
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;
        virtual ~Log();
        /**
         * Change the output stream to the specified one.
         * @param os Pointer to the output stream. Make sure that this stream will be available during the full lifetime
         * of the Session object.
         */
        void setStream(std::ostream* os);
        // TODO does std::ofstream file opening really throw exceptions or is there sth that has to be changed?
        /**
         * Change the output to append to the specified log file. Will throw a UserException with error code 1 if
         * the requested file cannot be opened for writing.
         * @param filename Path to the log file.
         */
        void setOutfile(std::string filename);
        /**
         * Write a message to the log.
         * @param msg The message.
         */
        void write(std::string msg);
        /**
         * Write a message to the log.
         * @param msg The message.
         */
        void operator()(std::string msg);
    };
}

#endif //QSF_LOG_H
