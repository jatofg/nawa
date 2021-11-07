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
#include <nawa/connection/ConnectionInitContainer.h>
#include <nawa/filter/AccessFilterList.h>
#include <nawa/oss.h>
#include <nawa/util/encoding.h>
#include <nawa/util/utils.h>
#include <regex>
#include <sstream>
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
            {511, "Network Authentication Required"}};
}

struct Connection::Data {
    string bodyString;
    unsigned int responseStatus = 200;
    unordered_map<string, vector<string>> headers;
    unordered_map<string, Cookie> cookies;
    Cookie cookiePolicy;
    bool isFlushed = false;
    FlushCallbackFunction flushCallback;

    Request request;
    Session session;
    Config config;
    stringstream responseStream;

    void clearStream() {
        responseStream.str(string());
        responseStream.clear();
    }

    void mergeStream() {
        bodyString += responseStream.str();
        clearStream();
    }

    Data(Connection* base, ConnectionInitContainer const& connectionInit) : request(connectionInit.requestInit),
                                                                            config(connectionInit.config),
                                                                            session(*base) {}
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(Connection)

void Connection::setResponseBody(string content) {
    data->bodyString = move(content);
    data->clearStream();
}

void Connection::sendFile(string const& path, string const& contentType, bool forceDownload,
                          string const& downloadFilename, bool checkIfModifiedSince) {

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
        lastModified = oss::getLastModifiedTimeOfFile(fileStat);
    }

    // check if-modified if requested
    time_t ifModifiedSince = 0;
    try {
        ifModifiedSince = stol(data->request.env()["if-modified-since"]);
    } catch (invalid_argument const&) {
    } catch (out_of_range const&) {}
    if (checkIfModifiedSince && ifModifiedSince >= lastModified) {
        setStatus(304);
        setResponseBody(string());
        return;
    }

    // set content-type header
    if (!contentType.empty()) {
        setHeader("content-type", contentType);
    } else {
        // use the function from utils.h to guess the content type
        setHeader("content-type", utils::contentTypeByExtension(utils::getFileExtension(path)));
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
        setHeader("last-modified", utils::makeHttpTime(lastModified));
    }

    // resize the bodyString, fill it with \0 chars if needed, make sure char fs [(fs+1)th] is \0, and insert file contents
    data->bodyString.resize(static_cast<unsigned long>(fs) + 1, '\0');
    data->bodyString[fs] = '\0';
    f.read(&data->bodyString[0], fs);

    // also clear the stream so that it doesn't mess with our file
    data->clearStream();
}

void Connection::setHeader(string key, string value) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    data->headers[key] = {move(value)};
}

void Connection::addHeader(string key, string value) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    data->headers[key].push_back(move(value));
}

void Connection::unsetHeader(string key) {
    // convert to lowercase
    transform(key.begin(), key.end(), key.begin(), ::tolower);
    data->headers.erase(key);
}

unordered_multimap<string, string> Connection::getHeaders(bool includeCookies) const {
    unordered_multimap<string, string> ret;
    for (auto const& [key, values] : data->headers) {
        for (auto const& value : values) {
            ret.insert({key, value});
        }
    }

    // include cookies if desired
    if (includeCookies)
        for (auto const& e : data->cookies) {
            stringstream headerVal;
            headerVal << e.first << "=" << e.second.content();
            // Domain option
            optional<string> domain = e.second.domain() ? e.second.domain() : data->cookiePolicy.domain();
            if (domain && !domain->empty()) {
                headerVal << "; Domain=" << *domain;
            }
            // Path option
            optional<string> path = e.second.path() ? e.second.path() : data->cookiePolicy.path();
            if (path && !path->empty()) {
                headerVal << "; Path=" << *path;
            }
            // Expires option
            optional<time_t> expiry = e.second.expires() ? e.second.expires() : data->cookiePolicy.expires();
            if (expiry) {
                headerVal << "; Expires=" << utils::makeHttpTime(*expiry);
            }
            // Max-Age option
            optional<unsigned long> maxAge = e.second.maxAge() ? e.second.maxAge()
                                                               : data->cookiePolicy.maxAge();
            if (maxAge) {
                headerVal << "; Max-Age=" << *maxAge;
            }
            // Secure option
            if (e.second.secure() || data->cookiePolicy.secure()) {
                headerVal << "; Secure";
            }
            // HttpOnly option
            if (e.second.httpOnly() || data->cookiePolicy.httpOnly()) {
                headerVal << "; HttpOnly";
            }
            // SameSite option
            Cookie::SameSite sameSite = (e.second.sameSite() != Cookie::SameSite::OFF) ? e.second.sameSite()
                                                                                       : data->cookiePolicy.sameSite();
            if (sameSite == Cookie::SameSite::LAX) {
                headerVal << "; SameSite=lax";
            } else if (sameSite == Cookie::SameSite::STRICT) {
                headerVal << "; SameSite=strict";
            }
            ret.insert({"set-cookie", headerVal.str()});
        }

    return ret;
}

string Connection::getResponseBody() {
    data->mergeStream();
    return data->bodyString;
}

Connection::Connection(ConnectionInitContainer const& connectionInit) {
    data = make_unique<Data>(this, connectionInit);
    data->flushCallback = connectionInit.flushCallback;

    data->headers["content-type"] = {"text/html; charset=utf-8"};
    // autostart of session must happen here (as config is not yet accessible in Session constructor)
    // check if autostart is enabled in config and if yes, directly call ::start
    if (data->config[{"session", "autostart"}] == "on") {
        data->session.start();
    }
}

void Connection::setCookie(string const& key, Cookie cookie) {
    // check key and value using regex, according to ietf rfc 6265
    regex matchKey(R"([A-Za-z0-9!#$%&'*+\-.^_`|~]*)");
    regex matchContent(R"([A-Za-z0-9!#$%&'()*+\-.\/:<=>?@[\]^_`{|}~]*)");
    if (!regex_match(key, matchKey) || !regex_match(cookie.content(), matchContent)) {
        throw Exception(__PRETTY_FUNCTION__, 1, "Invalid characters in key or value");
    }
    data->cookies[key] = move(cookie);
}

void Connection::setCookie(string const& key, string cookieContent) {
    setCookie(key, Cookie(move(cookieContent)));
}

void Connection::unsetCookie(string const& key) {
    data->cookies.erase(key);
}

void Connection::flushResponse() {
    // use callback to flush response
    data->flushCallback(FlushCallbackContainer{
            .status = data->responseStatus,
            .headers = getHeaders(true),
            .body = getResponseBody(),
            .flushedBefore = data->isFlushed});
    // response has been flushed now
    data->isFlushed = true;
    // also, empty the Connection object, so that content will not be sent more than once
    setResponseBody("");
}

void Connection::setStatus(unsigned int status) {
    data->responseStatus = status;
}

void Connection::setCookiePolicy(Cookie policy) {
    data->cookiePolicy = move(policy);
}

unsigned int Connection::getStatus() const {
    return data->responseStatus;
}

Request const& Connection::request() const noexcept {
    return data->request;
}

Session& Connection::session() noexcept {
    return data->session;
}

Session const& Connection::session() const noexcept {
    return data->session;
}

Config& Connection::config() noexcept {
    return data->config;
}

Config const& Connection::config() const noexcept {
    return data->config;
}

ostream& Connection::responseStream() noexcept {
    return data->responseStream;
}

bool Connection::applyFilters(AccessFilterList const& accessFilters) {
    // if filters are disabled, do not even check
    if (!accessFilters.filtersEnabled())
        return false;

    auto requestPath = data->request.env().getRequestPath();

    // check block filters
    for (auto const& flt : accessFilters.blockFilters()) {
        // if the filter does not apply (or does in case of an inverted filter), go to the next
        bool matches = flt.matches(requestPath);
        if ((!matches && !flt.invert()) || (matches && flt.invert())) {
            continue;
        }

        // filter matches -> apply block
        setStatus(flt.status());
        if (!flt.response().empty()) {
            setResponseBody(flt.response());
        } else {
            setResponseBody(utils::generateErrorPage(flt.status()));
        }
        // the request has been blocked, so no more filters have to be applied
        // returning true means: the request has been filtered
        return true;
    }

    // the ID is used to identify the exact filter for session cookie creation
    int authFilterID = -1;
    for (auto const& flt : accessFilters.authFilters()) {
        ++authFilterID;

        bool matches = flt.matches(requestPath);
        if ((!matches && !flt.invert()) || (matches && flt.invert())) {
            continue;
        }

        bool isAuthenticated = false;
        string sessionVarKey;

        // check session variable for this filter, if session usage is on
        if (flt.useSessions()) {
            data->session.start();
            sessionVarKey = "_nawa_authfilter" + to_string(authFilterID);
            if (data->session.isSet(sessionVarKey)) {
                isAuthenticated = true;
            }
        }

        // if this did not work, request authentication or invoke auth function if credentials have already been sent
        if (!isAuthenticated) {
            // case 1: no authorization header sent by client -> send a 401 without body
            if (data->request.env()["authorization"].empty()) {
                setStatus(401);
                stringstream hval;
                hval << "Basic";
                if (!flt.authName().empty()) {
                    hval << " realm=\"" << flt.authName() << '"';
                }
                setHeader("www-authenticate", hval.str());

                // that's it, the response must be sent to the client directly so it can authenticate
                return true;
            }
            // case 2: credentials already sent
            else {
                // split the authorization string, only the last part should contain base64
                auto authResponse = utils::splitString(data->request.env()["authorization"], ' ', true);
                // here, we should have a vector with size 2 and [0]=="Basic", otherwise sth is wrong
                if (authResponse.size() == 2 || authResponse.at(0) == "Basic") {
                    auto credentials = utils::splitString(encoding::base64Decode(authResponse.at(1)), ':', true);
                    // credentials must also have 2 elements, a username and a password,
                    // and the auth function must be callable
                    if (credentials.size() == 2 && flt.authFunction()) {
                        // now we can actually check the credentials with our function (if it is set)
                        if (flt.authFunction()(credentials.at(0), credentials.at(1))) {
                            isAuthenticated = true;
                            // now, if sessions are used, set the session variable to the username
                            if (flt.useSessions()) {
                                data->session.set(sessionVarKey, any(credentials.at(0)));
                            }
                        }
                    }
                }
            }
        }

        // now, if the user is still not authenticated, send a 403 Forbidden
        if (!isAuthenticated) {
            setStatus(403);
            if (!flt.response().empty()) {
                setResponseBody(flt.response());
            } else {
                setResponseBody(utils::generateErrorPage(403));
            }

            // request blocked
            return true;
        }

        // if the user is authenticated, we can continue to process forward filters
        break;
    }

    // check forward filters
    for (auto const& flt : accessFilters.forwardFilters()) {
        bool matches = flt.matches(requestPath);
        if ((!matches && !flt.invert()) || (matches && flt.invert())) {
            continue;
        }

        stringstream filePath;
        filePath << flt.basePath();
        if (flt.basePathExtension() == ForwardFilter::BasePathExtension::BY_PATH) {
            for (auto const& e : requestPath) {
                filePath << '/' << e;
            }
        } else {
            filePath << '/' << requestPath.back();
        }

        // send file if it exists, catch the "file does not exist" nawa::Exception and send 404 document if not
        auto filePathStr = filePath.str();
        try {
            sendFile(filePathStr, "", false, "", true);
        } catch (Exception&) {
            // file does not exist, send 404
            setStatus(404);
            if (!flt.response().empty()) {
                setResponseBody(flt.response());
            } else {
                setResponseBody(utils::generateErrorPage(404));
            }
        }

        // return true as the request has been filtered
        return true;
    }

    // if no filters were triggered (and therefore returned true), return false so that the request can be handled by
    // the app
    return false;
}

string FlushCallbackContainer::getStatusString() const {
    stringstream hval;
    hval << status;
    if (httpStatusCodes.count(status) == 1) {
        hval << " " << httpStatusCodes.at(status);
    }
    return hval.str();
}

string FlushCallbackContainer::getFullHttp() const {
    stringstream raw;
    // include headers and cookies, but only when flushing for the first time
    if (!flushedBefore) {
        // Add headers, incl. cookies, to the raw HTTP source
        for (auto const& e : headers) {
            raw << e.first << ": " << e.second << "\r\n";
        }
        raw << "\r\n";
    }
    raw << body;
    return raw.str();
}
