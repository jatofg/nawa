//
// Created by tobias on 08/01/19.
//

#include <iomanip>
#include <unistd.h>
#include <climits>
#include "Log.h"

Qsf::Log::Log() {
    out = &std::cerr;
    // get hostname
    char chostname[HOST_NAME_MAX+1];
    gethostname(chostname, HOST_NAME_MAX+1);
    hostname = chostname;
    // get pid
    pid = getpid();
}

Qsf::Log::Log(std::ostream *os) : Log() {
    out = os;
}

Qsf::Log::Log(std::string filename) : Log() {
    setOutfile(std::move(filename));
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

    *out << std::put_time(std::localtime(&now), "%b %d %H:%M:%S ") << hostname << ' ' << program_invocation_short_name
         << '[' << pid << "]: [QSF] " << msg << std::endl;
}

void Qsf::Log::operator()(std::string msg) {
    write(std::move(msg));
}
