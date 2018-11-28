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
        // will not work properly after flushing (as cookie cannot be set)
        void start();
        // TODO isEstablished function to check whether a session has been started
        // TODO garbage collection
        // TODO session ID generation and verification (is it expired, from correct IP, etc.)
        // TODO option whether session ID should be bound to 1 IP
        // TODO operator[] (read session data element)
        // TODO write() function or sth like that (write session data element)
    };
}


#endif //QSF_SESSION_H
