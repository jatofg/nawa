//
// Created by tobias on 23/11/18.
//

#ifndef QSF_SESSION_H
#define QSF_SESSION_H

#include "Connection.h"
#include "Types/Compound.h"
#include <vector>
#include <mutex>

namespace Qsf {
    struct SessionData {
        std::mutex dLock, eLock;
        std::vector<Types::Compound> data;
        time_t expires;
        const std::string sourceIP;
    };

    class Session {
        // TODO get session expiry from config and apply it
        static std::mutex gLock;
        static std::vector<std::shared_ptr<SessionData>> data;
        static std::map<std::string, size_t> cookies;
        Qsf::Connection& connection;
        size_t currentIndex;
        std::shared_ptr<SessionData> currentData;
        std::string generateID();
    public:
        explicit Session(Connection& response);
        virtual ~Session() = default;
        /**
         * Start the session (load existing session basing on a cookie sent by the client or create a new one).
         * This will send a session cookie to the client. The properties/attributes of the session cookie are determined
         * by, with decreasing precedence,\n
         * (1) the Cookie object that may be passed as an optional parameter to this function and will be used as a
         * template for the cookie. See the param description on how to use it.\n
         * (2) the configuration in the QSF configuration file,\n
         * (3) and, of course, by the cookie policy that can be set via Connection::setCookiePolicy.\n
         * The duration (keep-alive) of the session is defined in the QSF config file, but can be overridden by setting
         * the attribute maxAge of the parameter object, see below.\n
         * IMPORTANT! This function will NOT work correctly after flushing the response (as setting a session cookie
         * is impossible then). It is recommended to call start() directly in the beginning of your program.
         * @param properties Template for the cookie sent to the client. You can use it to influence the behavior of
         * Session and to make sure the cookie is properly secured. The attributes will be used as follows:\n
         * - content: will be ignored (and replaced by the session ID)\n
         * - expires: set this to > 0 (e.g., to 1), if the Expires and Max-Age attributes should be set for the cookie.
         * The value will be replaced by the proper expiry time. If 0, attribute inclusion can still be forced by the
         * QSF configuration or Connection::setCookiePolicy. Please note that if using setCookiePolicy, the attributes
         * will be added after Session has set the cookie and the contents will thus be determined by the policy, not
         * by the session. This may lead to unwanted behavior, so please make sure that you set this attribute to > 0
         * here if you are using setCookiePolicy.\n
         * - maxAge: will be used as the session duration (inactive keep-alive, server-side!) if > 0. If expires == 0
         * (and is also not overridden by the QSF config), this attribute will be reset to 0 before setting the cookie.
         * - secure: send the Secure attribute with the cookie.
         * - httpOnly: send the HttpOnly attribute with the cookie.
         * - sameSite: set the SameSite attribute to lax (if sameSite == 1) or strict (if sameSite > 1).
         */
        void start(Cookie properties = Cookie());
        // TODO isEstablished function to check whether a session has been started
        // TODO garbage collection
        // TODO session ID generation and verification (is it expired, from correct IP, etc.)
        // TODO option whether session ID should be bound to 1 IP
        // TODO operator[] (read session data element)
        // TODO write() function or sth like that (write session data element)
    };
}


#endif //QSF_SESSION_H
