//
// Created by tobias on 23/11/18.
//

#ifndef QSF_SESSION_H
#define QSF_SESSION_H

#include "Cookie.h"
#include "Types/Universal.h"
#include <vector>
#include <mutex>
#include <unordered_map>

namespace Qsf {
    class Connection;

    struct SessionData {
        std::mutex dLock, eLock;
        std::unordered_map<std::string, Types::Universal> data;
        time_t expires;
        const std::string sourceIP;
        SessionData() : expires(0) {}
        explicit SessionData(const std::string& sIP) : expires(0), sourceIP(sIP) {}
    };

    class Session {
        static std::mutex gLock; /**< Lock for data. */
        /**
         * Map containing the session data for all sessions. The key is the session ID string.
         */
        static std::unordered_map<std::string, std::shared_ptr<SessionData>> data;
        Qsf::Connection& connection; /**< Reference to the Connection object in order to access objects. */
        /**
         * Pointer to the session data struct for the current session, if established.
         * Can be used to check whether a session is established by checking shared_ptr::use_count()
         * (used by established()).
         */
        std::shared_ptr<SessionData> currentData;
        /**
         * Generate a random, 40 chars session ID.
         * @return The session ID.
         */
        std::string generateID();
        /**
         * Garbage collection by removing every expired session from the data map.
         * Would be best if run async and in fixed intervals (or with 0.xx chance on certain session actions -> see php)
         */
        static void collectGarbage();
    public:
        explicit Session(Connection& connection);
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
        void start(Qsf::Cookie properties = Cookie());
        /**
         * Check whether a session is currently active (has been started).
         * @return True if session is established, false otherwise.
         */
        bool established() const;
        /**
         * Check whether there is a value set for key key.
         * @param key Key to check.
         * @return True if a value exists for this key, false otherwise. Always false if no session established.
         */
        bool isSet(std::string key) const;
        /**
         * Get value at key key.
         * @param key Key to get value for.
         * @return Value at key. If no value exists for that key or no session established, an empty Compound is returned.
         */
        Types::Universal operator[](std::string key) const;
        /**
         * Set key to value. Throws a UserException with error code 1 if no session is established.
         * @param key Key to set.
         * @param value Value to set the key to.
         */
        void set(std::string key, const Types::Universal& value);
        // TODO voluntary session invalidation by user/app using public method
    };
}


#endif //QSF_SESSION_H
