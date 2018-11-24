//
// Created by tobias on 08/11/18.
//

#ifndef QSF_APPLICATION_H
#define QSF_APPLICATION_H

#include "Connection.h"

namespace Qsf {
    /**
     * Interface for applications that can be loaded by QSF.
     */
    class Application {
    public:
        Application() = default;
        virtual ~Application() = default;
        /**
         * This method will be run once when starting the QSF application. Use it to start background tasks or
         * continuously running parts of your application in separate threads or processes.\n
         * Attention! Requests cannot be handled until this function returned, so refrain from executing long-running
         * actions directly in the thread this function is running in.\n
         * This function is guaranteed to run only once at the start of your program. In contrast to the constructor,
         * it will only run after request handling has been fully set up.
         * @return A return value that may indicate success (0) or failure. As of now, this value has no effect at all.
         */
        virtual int init() { return 0; }
        /**
         * This method will be called whenever a new request arrives and allows you to define the response.
         * Please note that it might run concurrently in multiple threads, so accessing static variables requires using
         * mutexes.
         * @param request A Qsf::Request object containing all properties of the request.
         * @param response A Qsf::Response object that can be filled with the desired response to the user.
         * It is also possible to flush the response with it prematurely, even though it will also be flushed
         * automatically after returning.
         * @return A return value that may indicate success (0) or failure. As of now, this value has no effect at all.
         * In future, it might be possible to avoid flushing the response and let QSF print an error page instead.
         */
        virtual int processRequest(Qsf::Connection& connection) = 0;
    };
}

typedef Qsf::Application* newInstance_t();
typedef void deleteInstance_t();

#endif //QSF_APPLICATION_H
