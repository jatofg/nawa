//
// Created by tobias on 07/01/19.
//

#ifndef QSF_COOKIE_H
#define QSF_COOKIE_H

#include <string>
#include <ctime>

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
        unsigned long maxAge = 0; /**< Set Max-Age attribute. */
        std::string domain; /**< Set Domain attribute. */
        std::string path; /**< Set Path attribute. */
        bool secure = false; /**< Set Secure attribute. */
        bool httpOnly = false; /**< Set HttpOnly attribute. */
        unsigned int sameSite = 0;  /**< Set SameSite attribute. 1 for lax, >1 for strict. */
    };
}

#endif //QSF_COOKIE_H
