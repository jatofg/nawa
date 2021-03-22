/**
 * \file fwdecl.h
 * \brief Forward declarations.
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

#ifndef NAWA_FWDECL_H
#define NAWA_FWDECL_H

namespace nawa {
    // config
    class Config;
    class Connection;
    class ConnectionInitContainer;
    class Cookie;

    // filter
    class AccessFilterList;
    class AccessFilter;
    class AuthFilter;
    class BlockFilter;
    class ForwardFilter;

    // hashing
    namespace hashing {
        class HashingEngine;
        class Argon2HashingEngine;
        class BcryptHashingEngine;
        class HashTypeTable;
        class DefaultHashTypeTable;
    }// namespace hashing

    // logging
    class Log;

    // mail
    namespace mail {
        class EmailAddress;
        class SmtpMailer;
        class Email;
        class SimpleEmail;
        class MimeEmail;
    }// namespace mail

    // request
    class Request;
    class RequestInitContainer;
    class File;
    namespace request {
        class Env;
        class GPC;
        class Post;
    }// namespace request

    // RequestHandler
    class HandleRequestFunctionWrapper;
    class RequestHandler;

    // session
    class Session;

    // util
    class MimeMultipart;

    class RequestHandler;

    class AppInit;
    class Exception;
}// namespace nawa

#endif//NAWA_FWDECL_H
