//
// Created by tobias on 08/01/19.
//

#ifndef QSF_LOG_H
#define QSF_LOG_H

#include <iostream>
#include <mutex>
#include <fstream>

namespace Qsf {
    // TODO use the logger class, either a global object must be used (bs), or some statics are missing here
    /**
     * Simple class for thread-safe logging to stderr or to any other output stream
     */
    class Log {
        std::ostream* out;
        std::ofstream logFile;
        std::mutex outLock;
    public:
        Log();
        virtual ~Log();
        void setStream(std::ostream* os);
        void setOutfile(std::string filename);
        void write(std::string msg);
        Log& operator<<(std::string s);
    };
}

#endif //QSF_LOG_H
