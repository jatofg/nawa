//
// Created by tobias on 30/10/18.
//

#include <algorithm>
#include "Response.h"

void Qsf::Response::setBody(std::string content) {
    body = std::move(content);
    clearStream();
}

void Qsf::Response::setHeader(std::string key, std::string value) {
    // convert to lowercase
    // TODO check for correctness, maybe using regex (or waste of cpu time?)
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    headers[key] = std::move(value);
}

void Qsf::Response::unsetHeader(std::string key) {
    // convert to lowercase
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    headers.erase(key);
}

std::string Qsf::Response::getRaw() {
    mergeStream();
    std::stringstream raw;

    // include headers
    for(auto const &e: headers) {
        raw << e.first << ": " << e.second << "\r\n";
    }
    // include cookies
    for(auto const &e: cookies) {
        // TODO checking and escaping?
        // TODO respect cookie options
        raw << "Cookie: " << e.first << "=" << e.second.content << "\r\n";
    }
    raw << "\r\n";

    raw << body;
    return raw.str();
}

void Qsf::Response::mergeStream() {
    body += bodyStream.str();
    clearStream();
}

void Qsf::Response::clearStream() {
    bodyStream.str(std::string());
    bodyStream.clear();
}

Qsf::Response& Qsf::Response::operator<<(std::ostream &(*f)(std::ostream &)) {
    bodyStream << f;
    return *this;
}

Qsf::Response::Response() {
    headers["content-type"] = "text/html; charset=utf-8";
}

Qsf::Response::Response(Request& request, int cookieMode) {
    switch(cookieMode) {
        case QSF_COOKIES_SESSION:
            // TODO set session cookie here - this may not be what we want?
            // TODO session cookie name in config
            if(request.cookie.count("Session") > 0) {
                setCookie("Session", Cookie(request.cookie["Session"]));
            }
            break;
        case QSF_COOKIES_ALL:
            //for()
            // TODO iterators in Request
            for(auto const& c: request.cookie) {
                // TODO cookie options (should be set automatically?)?
                setCookie(c.first, Cookie(c.second));
            }
            break;
        default:
            break;
    }
}

void Qsf::Response::setCookie(std::string key, Qsf::Cookie cookie) {
    cookies[key] = std::move(cookie);
}

void Qsf::Response::unsetCookie(std::string key) {
    cookies.erase(key);
}
