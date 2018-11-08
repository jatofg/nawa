//
// Created by tobias on 30/10/18.
//

#include <algorithm>
#include <iomanip>
#include <locale>
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
        // Add headers to the raw HTTP source
        for(auto const &e: headers) {
            raw << e.first << ": " << e.second << "\r\n";
        }
        // include cookies
        for(auto const &e: cookies) {
            // TODO checking and especially escaping?
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
                raw << "; Expires=" << std::put_time(gmtime(&expiry), "%a, %d %b %Y %H:%M:%S GMT");
                std::locale::global(tmp);
            }
            // Max-Age option
            uint maxAge = (e.second.maxAge > 0) ? e.second.maxAge : cookiePolicy.maxAge;
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
            if(e.second.sameSite || cookiePolicy.sameSite) {
                raw << "; SameSite";
            }
            raw << "\r\n";
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

void Qsf::Response::setStatus(uint status) {
    headers["status"] = std::to_string(status);
}

void Qsf::Response::setCookiePolicy(Cookie policy) {
    cookiePolicy = std::move(policy);
}
