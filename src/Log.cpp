/**
 * \file Log.cpp
 * \brief Implementation of the Log class.
 */

/*
 * Copyright (C) 2019-2020 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <unistd.h>
#include <climits>
#include <nawa/UserException.h>
#include <nawa/Log.h>
#include <atomic>
#include <mutex>

using namespace nawa;
using namespace std;

namespace {
    bool locked = false; /**< If true, the stream and outfile cannot be changed anymore. */
    ostream *out; /**< Stream to send the logging output to. */
    ofstream logFile; /**< Log file handle in case a file is used and managed by this class. */
    unique_ptr<string> hostnameStr;
    __pid_t pid = 0;
    atomic_uint instanceCount(0);
    mutex outLock;
}

Log::Log() noexcept {
    if (instanceCount == 0) {
        out = &cerr;

        // get hostname
        char chostname[HOST_NAME_MAX + 1];
        gethostname(chostname, HOST_NAME_MAX + 1);
        hostnameStr = make_unique<string>(chostname);

        // get pid
        pid = getpid();
    }

    // appname is nawa by default
    appname = "nawa";

    ++instanceCount;
}

Log::Log(string appname_) noexcept: Log() {
    appname = move(appname_);
}

Log::Log(const Log &other) noexcept {
    appname = other.appname;
    ++instanceCount;
}

Log &Log::operator=(const Log &other) noexcept {
    if (this != &other) {
        appname = other.appname;
        ++instanceCount;
    }
    return *this;
}

Log::~Log() {
    --instanceCount;
    if (instanceCount == 0) {
        if (logFile.is_open()) {
            logFile.close();
        }
        locked = false;
        hostnameStr.reset(nullptr);
    }
}

void Log::setStream(ostream *os) noexcept {
    if (!locked) {
        out = os;
    }
}

void Log::setOutfile(const string &filename) {
    if (!locked) {
        if (logFile.is_open()) {
            logFile.close();
        }
        logFile.open(filename, ofstream::out | ofstream::app);
        if (!logFile) {
            throw UserException("nawa::Log::setOutfile", 1,
                                "Failed to open requested file for writing.");
        }
        out = &logFile;
    }
}

void Log::lockStream() noexcept {
    locked = true;
}

bool Log::isLocked() noexcept {
    return locked;
}

void Log::setAppname(string appname_) noexcept {
    appname = move(appname_);
}

void Log::write(const string &msg) {
    auto now = time(nullptr);

    lock_guard<mutex> l(outLock);
    *out << put_time(localtime(&now), "%b %d %H:%M:%S ") << *hostnameStr << ' ' << program_invocation_short_name
         << '[' << pid << "]: [" << appname << "] " << msg << endl;
}

void Log::operator()(const string &msg) {
    write(msg);
}
