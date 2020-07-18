/**
 * \file UserException.h
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

#ifndef NAWA_USEREXCEPTION_H
#define NAWA_USEREXCEPTION_H

#include <string>
#include <sstream>

namespace nawa {

    /**
     * Exception class that can be used by apps to catch errors resulting from NAWA function calls.
     */
    class UserException: public std::exception {
        int errorCode; /**< The error code so that the app can distinguish different exceptions from a function. */
        std::string message; /**< The exception message will be stored here. */
    public:
        /**
         * Construct a UserException with an additional message.
         * @param inFunction Function in which the exception occurred.
         * @param errorCode An integral error code identifying the error that caused this exception.
         * @param _message Additional message that will also be a part of what().
         */
        UserException(const std::string& inFunction, int errorCode, const std::string& _message) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "NAWA: UserException #" << errorCode << " in " << inFunction << ": " << _message;
            message = mstream.str();
        }
        /**
         * Construct a UserException without an additional message.
         * @param inFunction Function in which the exception occurred.
         * @param errorCode An integral error code identifying the error that caused this exception.
         */
        UserException(const std::string& inFunction, int errorCode) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "NAWA: UserException #" << errorCode << " in " << inFunction;
            message = mstream.str();
        }
        /**
         * Get the integral error code that identifies the error that caused this exception.
         * @return The error code.
         */
        virtual int getErrorCode() const noexcept {
            return errorCode;
        }
        /**
         * Get the full constructed exception message.
         * @return The full exception message.
         */
        const char* what() const noexcept override {
            return message.c_str();
        }
    };
}

#endif //NAWA_USEREXCEPTION_H
