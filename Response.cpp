//
// Created by tobias on 30/10/18.
//

#include <algorithm>
#include "Response.h"

void Qsf::Response::setBody(std::string content) {
    bodyString = std::move(content);
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

    // include headers and cookies, but only when flushing for the first time
    if(!isFlushed) {
        // import cookies from request depending on the cookieMode
        switch(cookieMode) {
            case QSF_COOKIES_SESSION:
                // TODO set session cookie here - this may not be what we want?
                // TODO session cookie name in config
                if(request.cookie.count("SESSION") > 0) {
                    setCookie("SESSION", Cookie(request.cookie["SESSION"]));
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
        // Add headers to the raw HTTP source
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
    }

    // And of course, add the body
    raw << bodyString;
    return raw.str();
}

void Qsf::Response::mergeStream() {
    bodyString += body.str();
    clearStream();
}

void Qsf::Response::clearStream() {
    body.str(std::string());
    body.clear();
}

Qsf::Response& Qsf::Response::operator<<(std::string s) {
    body << s;
    return *this;
}

Qsf::Response& Qsf::Response::operator<<(std::ostream &(*f)(std::ostream &)) {
    body << f;
    return *this;
}

Qsf::Response::Response(Request& request) : request(request) {
    headers["content-type"] = "text/html; charset=utf-8";
}

void Qsf::Response::setCookie(std::string key, Qsf::Cookie cookie) {
    cookies[key] = std::move(cookie);
}

void Qsf::Response::unsetCookie(std::string key) {
    cookies.erase(key);
}

void Qsf::Response::flush() {
    // access RequestHandler through Request::Env, which declares Response as a friend
    request.env.request.flush(*this);
    // now that headers and cookies have been sent to the client, make sure they are not included anymore
    isFlushed = true;
    // also, empty the Response object, so that content will not be sent more than once
    setBody("");
}

void Qsf::Response::setCookieMode(int cm) {
    cookieMode = cm;
}
