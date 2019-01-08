//
// Created by tobias on 08/01/19.
//

#include <iomanip>
#include <unistd.h>
#include <climits>
#include "Log.h"

Qsf::Log::Log() {
    out = &std::cerr;
}

Qsf::Log::~Log() {
    if(logFile.is_open()) {
        logFile.close();
    }
}

void Qsf::Log::setStream(std::ostream *os) {
    out = os;
}

void Qsf::Log::setOutfile(std::string filename) {
    if(logFile.is_open()) {
        logFile.close();
    }
    logFile.open(filename, std::ofstream::out | std::ofstream::app);
    out = &logFile;
}

void Qsf::Log::write(std::string msg) {
    auto now = std::time(nullptr);
    std::string hostname = getenv("HOSTNAME");
    *out << std::put_time(std::localtime(&now), "%b %d %H:%M:%S ") << hostname << ' ' << program_invocation_short_name
         << '[' << getpid() << "]: [QSF] " << msg << std::endl;
}

Qsf::Log &Qsf::Log::operator<<(std::string s) {
    write(std::move(s));
    return *this;
}
