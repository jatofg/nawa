//
// Created by tobias on 19/07/2020.
//

#ifndef NAWA_REQUESTHANDLER_H
#define NAWA_REQUESTHANDLER_H

#include <nawa/Config.h>
#include <nawa/AppInit.h>

namespace nawa {
    // forward declarations
    class Request;
    class Connection;

    // Types of functions that need to be accessed from NAWA applications
    typedef int init_t(nawa::AppInit& appInit); /**< Type for the init() function of NAWA apps. */
    typedef int handleRequest_t(nawa::Connection& connection); /**< Type for the handleRequest(Connection) function of NAWA apps. */

    class RequestHandler {
    protected:
        nawa::Config config;
    public:
        /**
         * Take over the config and dlopen handle to the app library file from main.
         * @param cfg Reference to the Config object representing the NAWA config file(s).
         * @param appOpen dlopen handle which will be used to load the app handleRequest(...) function.
         */
        static void setAppRequestHandler(const nawa::Config &cfg, void *appOpen);
        /**
         * Take over the AppInit struct filled by the init() function of the app.
         * @param _appInit AppInit struct as filled by the app.
         */
        static void setConfig(const nawa::AppInit &_appInit);
        /**
         * Reset the pointer to the AppInit to avoid a segfault on termination and clear session data.
         */
        static void destroyEverything();
        /**
         * Flush response to the browser. This function will be invoked by Connection::flushResponse().
         * @param connection Reference to the Connection object the response will be read from.
         */
        virtual void flush(nawa::Connection& connection) = 0;
        /**
         * Start request handling.
         */
        virtual void start() = 0;
        /**
         * Stop request handling.
         */
        virtual void stop() = 0;
        /**
         * Enforce termination of request handling.
         */
        virtual void terminate() = 0;
        /**
         * Block until request handling has shut down.
         */
        virtual void join() = 0;
    protected:
        int callHandleRequest(Connection& connection);
    };
}

#endif //NAWA_REQUESTHANDLER_H
