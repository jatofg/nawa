//
// Created on 30/10/18.
//

#ifndef QSF_RESPONSE_H
#define QSF_RESPONSE_H
#define QSF_COOKIES_NONE 0
#define QSF_COOKIES_SESSION 1
#define QSF_COOKIES_ALL 2

#include <string>
#include <map>
#include <sstream>
#include "Request.h"

namespace Qsf {
    /**
     * Structure for creating Cookie objects. Content can be set using the default constructor.
     */
    struct Cookie {
        /**
         * Create an empty Cookie.
         */
        Cookie() = default;
        /**
         * Create a cookie and directly set the content.
         * @param c Content of the cookie.
         */
        explicit Cookie(std::string c) {
            content = std::move(c);
        }
        std::string content; /**< Content of the cookie. */
        // TODO add options/flags here
    };
    /**
     * Response objects to be passed back to QSF.
     */
    class Response {
        std::string body;
        std::stringstream bodyStream;
        std::map<std::string, std::string> headers;
        std::map<std::string, Cookie> cookies;
        // set body
        // operator << and >> overloads for stream handling (body)
        // add/remove/set headers
        // handling of html special chars (in some other [static] class?)
        // cookie setting
        // export of data to fcgi++ (must merge bodyStream into body) => getRaw (?)
        // constructor should set cookies (?)
        // automatic handling of session cookies
        void clearStream();
        void mergeStream();
    public:
        /**
         * Create a Response object without importing any cookies from the request.
         */
        Response();
        // TODO create enum instead of define macros?
        // TODO add cookie options to constructor?
        // TODO deliver cookies in Request as Cookie struct? (possibly bad idea because of missing options)
        /**
         * Create a Response object and import cookies from the request.
         * @param request Reference to the request object (needed to import cookies).
         * @param cookieMode Which cookies to import, QSF_COOKIES_ALL (all cookies),
         * QSF_COOKIES_SESSION (default, only session cookie), or QSF_COOKIES_NONE (do not import cookies).
         * Cookies will always be added according to the default policy which can be set using setCookieOptions(),
         * therefore permanent cookies will become temporary ones when using QSF_COOKIES_ALL. In addition, you can
         * not rely on the client of course. Therefore, please set all cookies except for the session cookie manually
         * and do not use QSF_COOKIES_ALL, except in cases in which you know what you're doing.
         */
        explicit Response(Request& request, int cookieMode = QSF_COOKIES_SESSION);
        /**
         * Set the HTTP response body (everything that comes after the headers). This will overwrite everything
         * that was set previously. You can use the Response object as an ostream instead.
         * @param content Complete content of the HTTP body.
         */
        void setBody(std::string content);
        /**
         * Set an HTTP header or overwrite an existing one with the same key (keys are case-insensitive and will be
         * converted to lowercase). Please note that the content-type header will be automatically set to
         * "text/html; charset=utf-8", but can be overwritten, of course.
         * @param key Key of the HTTP header (case-insensitive).
         * @param value Value of the HTTP header.
         */
        void setHeader(std::string key, std::string value);
        /**
         * Unset the HTTP header with the specified key if it exists (otherwise do nothing). Please note that you can
         * only unset headers that were previously set in QSF (including content-type), not those that are set by,
         * for example, the web server.
         * @param key Key of the HTTP header (case-insensitive).
         */
        void unsetHeader(std::string key);
        // TODO cookie options: expiry, secure, httponly and possibly more (for specific cookies and setCookieOptions)
        /**
         * Set a new HTTP cookie or overwrite the cookie with the given key. Create a Cookie object first, setting at
         * least the content of the cookie. Please note that no sanitation or checking is currently done
         * (even though this may change in future), so make sure not to accept cookie keys or content from users
         * without proper sanitation! (In practice, only ASCII characters should be used in cookies, and '=' must
         * not be used in the key).
         * @param key Key of the cookie.
         * @param cookie Cookie object containing the value and options of the cookie.
         */
        void setCookie(std::string key, Cookie cookie);
        /**
         * Unset an HTTP cookie that was previously set using setCookie().
         * @param key Key of the cookie.
         */
        void unsetCookie(std::string key);
        /**
         * Get the raw HTTP source of the request. This function is intended to be used primarily by QSF itself.
         * @return A string containing the raw HTTP source (containing headers, incl. cookies, and the body)
         */
        std::string getRaw();
        /**
         * You can use the Response object like an ostream by adding content to the HTTP body like this:\n
         * response << "Some HTML content";
         * @param f Something that can be added to an output stream.
         * @return A reference to the Response object itself so you can add more using another "<<".
         */
        Response& operator<<(std::ostream&(*f)(std::ostream&));
    };
}

// TODO add stream modifiers if needed

#endif //QSF_RESPONSE_H
