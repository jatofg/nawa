/**
 * \file Exception.h
 * \brief Exception class that can be used by apps to catch errors resulting from nawa function calls.
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

#ifndef NAWA_EXCEPTION_H
#define NAWA_EXCEPTION_H

#include <string>
#include <sstream>

namespace nawa {

    /**
     * Exception class that can be used by apps to catch errors resulting from NAWA function calls.
     */
    class Exception : public std::exception {
        int errorCode; /**< The error code so that the app can distinguish different exceptions from a function. */
        std::string message; /**< The exception message will be stored here. */
        std::string debugMessage; /**< The full debug message will be stored here. */
    public:
        /**
         * Construct a UserException with an additional message.
         * @param inFunction Function in which the exception occurred.
         * @param errorCode An integral error code identifying the error that caused this exception.
         * @param message Additional message that will also be a part of what().
         */
        Exception(const std::string &inFunction, int errorCode, std::string message) : errorCode(errorCode),
                                                                                       message(std::move(message)) {
            std::stringstream mstream;
            mstream << "[NAWA Exception #" << errorCode << " in " << inFunction << "] " << message;
            debugMessage = mstream.str();
        }

        /**
         * Construct a UserException without an additional message.
         * @param inFunction Function in which the exception occurred.
         * @param errorCode An integral error code identifying the error that caused this exception.
         */
        Exception(const std::string &inFunction, int errorCode) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "[NAWA Exception #" << errorCode << " in " << inFunction << "] No message provided.";
            message = mstream.str();
        }

        /**
         * Get the integral error code that identifies the error that caused this exception.
         * @return The error code.
         */
        [[nodiscard]] virtual int getErrorCode() const noexcept {
            return errorCode;
        }

        /**
         * Get the provided message as a string.
         * @return The provided message.
         */
        [[nodiscard]] virtual std::string getMessage() const noexcept {
            return message;
        }

        /**
         * Get the full constructed debug message as a string.
         * @return
         */
        [[nodiscard]] virtual std::string getDebugMessage() const noexcept {
            return debugMessage;
        }

        /**
         * Get the full constructed debug message. This function would be called and the message displayed when an
         * uncaught Exception leads to termination. Use getMessage to get the message without debug information.
         * @return The full exception message.
         */
        const char *what() const noexcept override {
            return debugMessage.c_str();
        }
    };
}

#endif //NAWA_EXCEPTION_H
