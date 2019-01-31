/**
 * \file Connection.cpp
 * \brief Implementation of the Connection class.
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

#include <algorithm>
#include <iomanip>
#include <locale>
#include <regex>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <qsf/Utils.h>
#include <qsf/Connection.h>

namespace {
    const std::unordered_map<unsigned int, std::string> httpStatusCodes = {
            {200, "OK"}, {201, "Created"}, {202, "Accepted"}, {203, "Non-Authoritative Information"},
            {204, "No Content"}, {205, "Reset Content"}, {206, "Partial Content"}, {207, "Multi-Status"},
            {208, "Already Reported"}, {226, "IM Used"},
            {300, "Multiple Choices"}, {301, "Moved Permanently"}, {302, "Found"}, {303, "See Other"},
            {304, "Not Modified"}, {305, "Use Proxy"}, {307, "Temporary Redirect"}, {308, "Permanent Redirect"},
            {400, "Bad Request"}, {401, "Unauthorized"}, {402, "Payment Required"}, {403, "Forbidden"},
            {404, "Not Found"}, {405, "Method Not Allowed"}, {406, "Not Acceptable"},
            {407, "Proxy Authentication Required"}, {408, "Request Timeout"}, {409, "Conflict"}, {410, "Gone"},
            {411, "Length Required"}, {412, "Precondition Failed"}, {413, "Payload Too Large"}, {414, "URI Too Long"},
            {415, "Unsupported Media Type"}, {416, "Range Not Satisfiable"}, {417, "Expectation Failed"},
            {418, "I'm a teapot"}, {421, "Misdirected Request"}, {422, "Unprocessable Entity"}, {423, "Locked"},
            {424, "Failed Dependency"}, {426, "Upgrade Required"}, {428, "Precondition Required"},
            {429, "Too Many Requests"}, {431, "Request Header Fields Too Large"}, {451, "Unavailable For Legal Reasons"},
            {500, "Internal Server Error"}, {501, "Not Implemented"}, {502, "Bad Gateway"}, {503, "Service Unavailable"},
            {504, "Gateway Timeout"}, {505, "HTTP Version Not Supported"}, {506, "Variant Also Negotiates"},
            {507, "Insufficient Storage"}, {508, "Loop Detected"}, {510, "Not Extended"},
            {511, "Network Authentication Required"}
    };
}

void Qsf::Connection::setBody(std::string content) {
    bodyString = std::move(content);
    clearStream();
}

void
Qsf::Connection::sendFile(std::string path, std::string contentType, bool forceDownload, std::string downloadFilename,
        bool checkIfModifiedSince) {

    // open file as binary
    std::ifstream f(path, std::ifstream::binary);

    // throw exception if file cannot be opened
    if(!f) {
        throw Qsf::UserException("Qsf::Connection::sendFile", 1, "Cannot open file for reading");
    }

    // get time of last modification
    struct stat fileStat;
    time_t lastModified = 0;
    if(stat(path.c_str(), &fileStat) == 0) {
        lastModified = fileStat.st_mtim.tv_sec;
    }

    // check if-modified if requested
    // TODO make ifModifiedSince available without converting it back and forth (universal? or not before v2?)
    // TODO implement other cache control headers, such as pragma, expires, ...
    if(checkIfModifiedSince && std::stoul(request.env["ifModifiedSince"]) >= lastModified) {
        setStatus(304);
        setBody(std::string());
        return;
    }

    // set content-type header
    if(!contentType.empty()) {
        setHeader("content-type", contentType);
    }
    else {
        // use the function from utils.h to guess the content type
        setHeader("content-type", Qsf::content_type_by_extension(Qsf::get_file_extension(path)));
    }

    // set the content-disposition header
    if(forceDownload) {
        if(!downloadFilename.empty()) {
            std::stringstream hval;
            hval << "attachment; filename=\"" << downloadFilename << '"';
            setHeader("content-disposition", hval.str());
        }
        else {
            setHeader("content-disposition", "attachment");
        }
    }
    else if(!downloadFilename.empty()) {
        std::stringstream hval;
        hval << "inline; filename=\"" << downloadFilename << '"';
        setHeader("content-disposition", hval.str());
    }

    // set the content-length header
    // get file size
    f.seekg(0, std::ios::end);
    long fs = f.tellg();
    f.seekg(0);
    setHeader("content-length", std::to_string(fs));

    // set the last-modified header (if possible)
    if(lastModified > 0) {
        setHeader("last-modified", Qsf::make_http_time(lastModified));
    }

    // resize the bodyString, fill it with \0 chars if needed, make sure char fs [(fs+1)th] is \0, and insert file contents
    bodyString.resize(static_cast<unsigned long>(fs) + 1, '\0');
    bodyString[fs] = '\0';
    f.read(&bodyString[0], fs);

    // also clear the stream so that it doesn't mess with our file
    clearStream();
}

void Qsf::Connection::setHeader(std::string key, std::string value) {
    // convert to lowercase
    // TODO check for correctness, maybe using regex (or waste of cpu time?)
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    headers[key] = std::move(value);
}

void Qsf::Connection::unsetHeader(std::string key) {
    // convert to lowercase
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    headers.erase(key);
}

std::string Qsf::Connection::getRaw() {
    mergeStream();
    std::stringstream raw;

    // include headers and cookies, but only when flushing for the first time
    if(!isFlushed) {
        // Add headers to the raw HTTP source
        for(auto const &e: headers) {
            raw << e.first << ": " << e.second << "\r\n";
        }
        // include cookies
        for(auto const &e: cookies) {
            raw << "Set-Cookie: " << e.first << "=" << e.second.content;
            // Domain option
            const std::string& domain = (!e.second.domain.empty()) ? e.second.domain : cookiePolicy.domain;
            if(!domain.empty()) {
                raw << "; Domain=" << domain;
            }
            // Path option
            const std::string& path = (!e.second.path.empty()) ? e.second.path : cookiePolicy.path;
            if(!path.empty()) {
                raw << "; Path=" << path;
            }
            // Expires option
            time_t expiry = (e.second.expires > 0) ? e.second.expires : cookiePolicy.expires;
            if(expiry > 0) {
                // reset locale to C for correct Expires string and reset it afterwards
                // (in case the app modified the locale)
                std::locale tmp;
                std::locale::global(std::locale::classic());
                raw << "; Expires=" << Qsf::make_http_time(expiry);
                std::locale::global(tmp);
            }
            // Max-Age option
            unsigned long maxAge = (e.second.maxAge > 0) ? e.second.maxAge : cookiePolicy.maxAge;
            if(maxAge > 0) {
                raw << "; Max-Age=" << maxAge;
            }
            // Secure option
            if(e.second.secure || cookiePolicy.secure) {
                raw << "; Secure";
            }
            // HttpOnly option
            if(e.second.httpOnly || cookiePolicy.httpOnly) {
                raw << "; HttpOnly";
            }
            // SameSite option
            uint sameSite = (e.second.sameSite > cookiePolicy.sameSite) ? e.second.sameSite : cookiePolicy.sameSite;
            if(sameSite == 1) {
                raw << "; SameSite=lax";
            }
            else if(sameSite > 1) {
                raw << "; SameSite=strict";
            }
            raw << "\r\n";
        }
        raw << "\r\n";
    }

    // And of course, add the body
    raw << bodyString;
    return raw.str();
}

void Qsf::Connection::mergeStream() {
    bodyString += response.str();
    clearStream();
}

void Qsf::Connection::clearStream() {
    response.str(std::string());
    response.clear();
}

Qsf::Connection::Connection(Request& request, Config& config) : request(request), config(config), session(*this) {
    headers["content-type"] = "text/html; charset=utf-8";
    // autostart of session must happen here (as config is not yet accessible in Session constructor)
    // check if autostart is enabled in config and if yes, directly call ::start
    if(config[{"session", "autostart"}] == "on") {
        session.start();
    }
}

void Qsf::Connection::setCookie(std::string key, Qsf::Cookie cookie) {
    // check key and value using regex, according to ietf rfc 6265
    std::regex matchKey(R"([A-Za-z0-9!#$%&'*+\-.^_`|~]*)");
    std::regex matchContent(R"([A-Za-z0-9!#$%&'()*+\-.\/:<=>?@[\]^_`{|}~]*)");
    if(!std::regex_match(key, matchKey) || !std::regex_match(cookie.content, matchContent)) {
        throw UserException("Qsf::Connection::setCookie", 1, "Invalid characters in key or value");
    }
    cookies[key] = std::move(cookie);
}

void Qsf::Connection::unsetCookie(std::string key) {
    cookies.erase(key);
}

void Qsf::Connection::flushResponse() {
    // access RequestHandler through Request::Env, which declares Connection as a friend
    request.env.requestHandler.flush(*this);
    // now that headers and cookies have been sent to the client, make sure they are not included anymore
    isFlushed = true;
    // also, empty the Connection object, so that content will not be sent more than once
    setBody("");
}

void Qsf::Connection::setStatus(unsigned int status) {
    std::stringstream hval;
    hval << status;
    if(httpStatusCodes.count(status) == 1) {
        hval << " " << httpStatusCodes.at(status);
    }
    headers["status"] = hval.str();
}

void Qsf::Connection::setCookiePolicy(Cookie policy) {
    cookiePolicy = std::move(policy);
}
