/**
 * \file SysException.h
 * \brief Exception class to provide debugging output for severe errors in QSF itself.
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

#ifndef QSF_QSFEXCEPTION_H
#define QSF_QSFEXCEPTION_H

#include <string>
#include <sstream>

namespace soru {

    /**
     * Exception class to provide debugging output for severe errors in QSF itself.
     */
    class SysException: public std::exception {
        std::string message; /**< The exception message will be stored here. */
    public:
        /**
         * Construct a SysException without an additional message.
         * @param file File in which the exception occurred.
         * @param line Line of the source code where the exception occurred.
         */
        SysException(const std::string& file, unsigned int line) {
            std::stringstream ret;
            ret << "QSF: SysException in " << file << " at line " << line;
            message = ret.str();
        }
        /**
         * Construct a SysException with an additional message.
         * @param file File in which the exception occurred.
         * @param line Line of the source code where the exception occurred.
         * @param _message The exception message that will be additionally provided to the (hopefully) developer.
         */
        SysException(const std::string& file, unsigned int line, const std::string& _message) {
            std::stringstream ret;
            ret << "QSF: SysException in " << file << " at line " << line << ": " << _message;
            message = ret.str();
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




#endif //QSF_QSFEXCEPTION_H
