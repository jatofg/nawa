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
    class Session {
        static std::mutex gLock;
        static std::vector<std::mutex> locks;
        static std::vector<std::vector<Types::Compound>> data;
        static std::map<std::string, size_t> cookies;
        Qsf::Connection& connection;
        size_t current;
        std::string generateID();
    public:
        explicit Session(Connection& response);
        virtual ~Session() = default;
        // expiry, garbage collection -> SessionData struct?
        // session ID generation and verification (is it expired, from correct IP, etc.)
        // also: option whether session ID should be bound to 1 IP
        // operator[]
        // write() function or sth like that
    };
}


#endif //QSF_SESSION_H
