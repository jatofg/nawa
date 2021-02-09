/**
 * \file Connection.cpp
 * \brief Implementation of the Connection and FlushCallbackContainer classes.
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

#include <fstream>
#include <nawa/Exception.h>
#include <nawa/connection/Connection.h>
#include <nawa/connection/FlushCallbackContainer.h>
#include <nawa/util/Utils.h>
#include <regex>
#include <sys/stat.h>

using namespace nawa;
using namespace std;

namespace {
    const unordered_map<unsigned int, string> httpStatusCodes = {
            {200, "OK"},
            {201, "Created"},
            {202, "Accepted"},
            {203, "Non-Authoritative Information"},
            {204, "No Content"},
            {205, "Reset Content"},
            {206, "Partial Content"},
            {207, "Multi-Status"},
            {208, "Already Reported"},
            {226, "IM Used"},
            {300, "Multiple Choices"},
            {301, "Moved Permanently"},
            {302, "Found"},
            {303, "See Other"},
            {304, "Not Modified"},
            {305, "Use Proxy"},
            {307, "Temporary Redirect"},
            {308, "Permanent Redirect"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {402, "Payment Required"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {406, "Not Acceptable"},
            {407, "Proxy Authentication Required"},
            {408, "Request Timeout"},
            {409, "Conflict"},
            {410, "Gone"},
            {411, "Length Required"},
            {412, "Precondition Failed"},
            {413, "Payload Too Large"},
            {414, "URI Too Long"},
            {415, "Unsupported Media Type"},
            {416, "Range Not Satisfiable"},
            {417, "Expectation Failed"},
            {418, "I'm a teapot"},
            {421, "Misdirected Request"},
            {422, "Unprocessable Entity"},
            {423, "Locked"},
            {424, "Failed Dependency"},
            {426, "Upgrade Required"},
            {428, "Precondition Required"},
            {429, "Too Many Requests"},
            {431, "Request Header Fields Too Large"},
            {451, "Unavailable For Legal Reasons"},
            {500, "Internal Server Error"},
            {501, "Not Implemented"},
            {502, "Bad Gateway"},
            {503, "Service Unavailable"},
            {504, "Gateway Timeout"},
            {505, "HTTP Version Not Supported"},
            {506, "Variant Also Negotiates"},
            {507, "Insufficient Storage"},
            {508, "Loop Detected"},
            {510, "Not Extended"},
            {511, "Network Authentication Required"}
    };

    void clearStream(Connection *base) {
        base->response.str(string());
        base->response.clear();
    }
}

struct Connection::Impl {
    std::string bodyString;
    unsigned int responseStatus = 200;
    std::unordered_map<std::string, std::vector<std::string>> headers;
    std::unordered_map<std::string, Cookie> cookies;
    Cookie cookiePolicy;
    bool isFlushed = false;
    FlushCallbackFunction flushCallback;

    void mergeStream(Connection *base) {
        bodyString += base->response.str();
        clearStream(base);
    }
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Connection)

void Connection::setBody(string content) {
    impl->bodyString = move(content);
    clearStream(this);
}

void
Connection::sendFile(const string &path, const string &contentType, bool forceDownload,
                     const string &downloadFilename, bool checkIfModifiedSince) {

    // open file as binary
    ifstream f(path, ifstream::binary);

    // throw exception if file cannot be opened
    if (!f) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Cannot open file for reading");
    }

    // get time of last modification
    struct stat fileStat;
    time_t lastModified = 0;
    if (stat(path.c_str(), &fileStat) == 0) {
        lastModified = fileStat.st_mtim.tv_sec;
    }

    // check if-modified if requested
    time_t ifModifiedSince = 0;
    try {
        ifModifiedSince = stoul(request.env["if-modified-since"]);
    } catch (invalid_argument const &) {} catch (out_of_range const &) {}
    if (checkIfModifiedSince && ifModifiedSince >= lastModified) {
        setStatus(304);
        setBody(string());
        return;
    }

    // set content-type header
    if (!contentType.empty()) {
        setHeader("content-type", contentType);
    } else {
        // use the function from utils.h to guess the content type
        setHeader("content-type", content_type_by_extension(get_file_extension(path)));
    }

    // set the content-disposition header
    if (forceDownload) {
        if (!downloadFilename.empty()) {
            stringstream hval;
            hval << "attachment; filename=\"" << downloadFilename << '"';
            setHeader("content-disposition", hval.str());
        } else {
            setHeader("content-disposition", "attachment");
        }
    } else if (!downloadFilename.empty()) {
        stringstream hval;
        hval << "inline; filename=\"" << downloadFilename << '"';
        setHeader("content-disposition", hval.str());
    }

    // set the content-length header
    // get file size
    f.seekg(0, ios::end);
    long fs = f.tellg();
    f.seekg(0);
    setHeader("content-length", to_string(fs));

    // set the last-modified header (if possible)
    if (lastModified > 0) {
        setHeader("last-modified", make_http_time(lastModified));
    }

    // resize the bodyString, fill it with \0 chars if needed, make sure char fs [(fs+1)th] is \0, and insert file contents
    impl->bodyString.resize(static_cast<unsigned long>(fs) + 1, '\0');
    impl->bodyString[fs] = '\0';
    f.read(&impl->bodyString[0], fs);

    // also clear the stream so that it doesn't mess with our file
    clearStream(this);
}

void Connection::setHeader(string key, string value) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    impl->headers[key] = {move(value)};
}

void Connection::addHeader(string key, string value) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    impl->headers[key].push_back(move(value));
}

void Connection::unsetHeader(string key) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    impl->headers.erase(key);
}

unordered_multimap<string, string> Connection::getHeaders(bool includeCookies) const {
    unordered_multimap<string, string> ret;
    for (auto const &[key, values]: impl->headers) {
        for (auto const &value: values) {
            ret.insert({key, value});
        }
    }

    // include cookies if desired
    if (includeCookies)
        for (auto const &e: impl->cookies) {
            stringstream headerVal;
            headerVal << e.first << "=" << e.second.getContent();
            // Domain option
            optional<string> domain = e.second.getDomain() ? e.second.getDomain() : impl->cookiePolicy.getDomain();
            if (domain && !domain->empty()) {
                headerVal << "; Domain=" << *domain;
            }
            // Path option
            optional<string> path = e.second.getPath() ? e.second.getPath() : impl->cookiePolicy.getPath();
            if (path && !path->empty()) {
                headerVal << "; Path=" << *path;
            }
            // Expires option
            optional<time_t> expiry = e.second.getExpires() ? e.second.getExpires() : impl->cookiePolicy.getExpires();
            if (expiry) {
                headerVal << "; Expires=" << make_http_time(*expiry);
            }
            // Max-Age option
            optional<unsigned long> maxAge = e.second.getMaxAge() ? e.second.getMaxAge()
                                                                  : impl->cookiePolicy.getMaxAge();
            if (maxAge) {
                headerVal << "; Max-Age=" << *maxAge;
            }
            // Secure option
            if (e.second.getSecure() || impl->cookiePolicy.getSecure()) {
                headerVal << "; Secure";
            }
            // HttpOnly option
            if (e.second.getHttpOnly() || impl->cookiePolicy.getHttpOnly()) {
                headerVal << "; HttpOnly";
            }
            // SameSite option
            Cookie::SameSite sameSite = (e.second.getSameSite() != Cookie::SameSite::OFF) ? e.second.getSameSite()
                                                                                          : impl->cookiePolicy.getSameSite();
            if (sameSite == Cookie::SameSite::LAX) {
                headerVal << "; SameSite=lax";
            } else if (sameSite == Cookie::SameSite::STRICT) {
                headerVal << "; SameSite=strict";
            }
            ret.insert({"set-cookie", headerVal.str()});
        }

    return ret;
}

string Connection::getBody() {
    impl->mergeStream(this);
    return impl->bodyString;
}

Connection::Connection(const ConnectionInitContainer &connectionInit)
        : request(connectionInit.requestInit),
          config(connectionInit.config),
          session(*this) {
    impl = make_unique<Impl>();
    impl->flushCallback = connectionInit.flushCallback;

    impl->headers["content-type"] = {"text/html; charset=utf-8"};
    // autostart of session must happen here (as config is not yet accessible in Session constructor)
    // check if autostart is enabled in config and if yes, directly call ::start
    if (config[{"session", "autostart"}] == "on") {
        session.start();
    }
}

void Connection::setCookie(const string &key, Cookie cookie) {
    // check key and value using regex, according to ietf rfc 6265
    regex matchKey(R"([A-Za-z0-9!#$%&'*+\-.^_`|~]*)");
    regex matchContent(R"([A-Za-z0-9!#$%&'()*+\-.\/:<=>?@[\]^_`{|}~]*)");
    if (!regex_match(key, matchKey) || !regex_match(cookie.getContent(), matchContent)) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Invalid characters in key or value");
    }
    impl->cookies[key] = move(cookie);
}

void Connection::setCookie(const string &key, string cookieContent) {
    setCookie(key, Cookie(move(cookieContent)));
}

void Connection::unsetCookie(const string &key) {
    impl->cookies.erase(key);
}

void Connection::flushResponse() {
    // use callback to flush response
    impl->flushCallback(FlushCallbackContainer{.status=impl->responseStatus, .headers=getHeaders(true),
            .body=getBody(), .flushedBefore=impl->isFlushed});
    // response has been flushed now
    impl->isFlushed = true;
    // also, empty the Connection object, so that content will not be sent more than once
    setBody("");
}

void Connection::setStatus(unsigned int status) {
    impl->responseStatus = status;
}

void Connection::setCookiePolicy(Cookie policy) {
    impl->cookiePolicy = move(policy);
}

unsigned int Connection::getStatus() const {
    return impl->responseStatus;
}

std::string FlushCallbackContainer::getStatusString() const {
    stringstream hval;
    hval << status;
    if (httpStatusCodes.count(status) == 1) {
        hval << " " << httpStatusCodes.at(status);
    }
    return hval.str();
}

std::string FlushCallbackContainer::getFullHttp() const {
    stringstream raw;
    // include headers and cookies, but only when flushing for the first time
    if (!flushedBefore) {
        // Add headers, incl. cookies, to the raw HTTP source
        for (auto const &e: headers) {
            raw << e.first << ": " << e.second << "\r\n";
        }
        raw << "\r\n";
    }
    raw << body;
    return raw.str();
}
