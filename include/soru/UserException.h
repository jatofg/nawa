/**
 * \file UserException.h
 * \brief Exception class that can be used by apps to catch errors resulting from soru function calls.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SORU_USEREXCEPTION_H
#define SORU_USEREXCEPTION_H

#include <string>
#include <sstream>

namespace soru {

    /**
     * Exception class that can be used by apps to catch errors resulting from QSF function calls.
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
            mstream << "QSF: UserException #" << errorCode << " in " << inFunction << ": " << _message;
            message = mstream.str();
        }
        /**
         * Construct a UserException without an additional message.
         * @param inFunction Function in which the exception occurred.
         * @param errorCode An integral error code identifying the error that caused this exception.
         */
        UserException(const std::string& inFunction, int errorCode) : errorCode(errorCode) {
            std::stringstream mstream;
            mstream << "QSF: UserException #" << errorCode << " in " << inFunction;
            message = mstream.str();
        }
        /**
         * Get the integral error code that identifies the error that caused this exception.
         * @return The error code.
         */
        virtual const int getErrorCode() const noexcept {
            return errorCode;
        }
        /**
         * Get the full constructed exception message.
         * @return The full exception message.
         */
        virtual const char* what() const noexcept {
            return message.c_str();
        }
    };
}

#endif //SORU_USEREXCEPTION_H
