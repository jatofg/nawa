//
// Created on 30/10/18. -> Connection
//

#ifndef QSF_RESPONSE_H
#define QSF_RESPONSE_H

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
        std::time_t expires = 0; /**< Expiry date as time_t structure. 0 for no explicit expiry (session cookie). */
        uint maxAge = 0; /**< Set Max-Age attribute. */
        std::string domain; /**< Set Domain attribute. */
        std::string path; /**< Set Path attribute. */
        bool secure = false; /**< Set Secure attribute. */
        bool httpOnly = false; /**< Set HttpOnly attribute. */
        bool sameSite = false;  /**< Set SameSite attribute. */
    };
    /**
     * Response objects to be passed back to QSF.
     */
    class Connection {
        std::string bodyString;
        std::map<std::string, std::string> headers;
        std::map<std::string, Cookie> cookies;
        bool isFlushed = false;
        Cookie cookiePolicy;
        // set body
        // operator << and >> overloads for stream handling (response)
        // add/remove/set headers
        // handling of html special chars (in some other [static] class?)
        // cookie setting
        // export of data to fcgi++ (must merge response into bodyString) => getRaw (?)
        // constructor should set cookies (?)
        // automatic handling of session cookies
        void clearStream();
        void mergeStream();
    public:
        const Qsf::Request& request;
        const Qsf::Config& config;
        std::stringstream response; /**< Stringstream that allows you to write stuff to the HTTP body comfortably. */
        // TODO deliver cookies in Request as Cookie struct? (possibly bad idea because of missing options)
        /**
         * Create a Response object.
         * @param request Reference to the request object (needed to import cookies and flush the response).
         */
        Connection(Request& request, Config& config);
        /**
         * Set the HTTP response body (everything that comes after the headers). This will overwrite everything
         * that was set previously. You can use the Response object as an ostream instead.
         * @param content Complete content of the HTTP body.
         */
        void setBody(std::string content);
        /**
         * Set the HTTP status code. It will be passed to the web server without checking for validity.
         * @param status The HTTP status code to pass to the web server.
         */
        void setStatus(uint status);
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
         * This method can be used to set default attributes for cookies. Setting a boolean attribute to true means
         * that the corresponding attribute will be sent for all cookies, regardless of the value specified in the
         * Cookie object itself. Values of string attributes will be used as default values if the corresponding
         * attributes are not customized in the Cookie object and ignored otherwise. The content attribute will be
         * ignored.
         * @param policy Cookie object containing the default attributes.
         */
        void setCookiePolicy(Cookie policy);
        /**
         * Get the raw HTTP source of the request. This function is intended to be used primarily by QSF itself.
         * @return A string containing the raw HTTP source (containing headers, incl. cookies, and the body)
         */
        std::string getRaw();
        /**
         * You can add strings and a few other things to the body like this:\n
         * response << "Some HTML content";\n
         * Please note, however, that Response is not a full-featured ostream. In case of problems, please
         * address the stream directly:\n
         * response.body << "Some HTML content";\n
         * This feature might be removed at any time from v0.
         * @param s Something that can be added to an output stream.
         * @return A reference to the Response object itself so you can add more using another "<<".
         */
        Connection& operator<<(std::string s);
        Connection& operator<<(std::ostream&(*f)(std::ostream&));
        /**
         * Flush the Response object, i.e., send headers and body to the client and reset it.
         * Please note that you cannot set cookies and headers anymore after flushing.
         * Attempts to do so will be silently ignored.\n
         * NOTE: Flushing might not work as expected with Apache 2.4 and mod_proxy_fcgi
         */
        void flush();
    };
}

// TODO add stream modifiers if needed

#endif //QSF_RESPONSE_H
