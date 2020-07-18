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

namespace {
    bool locked = false; /**< If true, the stream and outfile cannot be changed anymore. */
    std::ostream* out; /**< Stream to send the logging output to. */
    std::ofstream logFile; /**< Log file handle in case a file is used and managed by this class. */
    std::unique_ptr<std::string> hostnameStr;
    __pid_t pid = 0;
    std::atomic_uint instanceCount(0);
    std::mutex outLock;
}

nawa::Log::Log() noexcept {
    if(instanceCount == 0) {
        out = &std::cerr;

        // get hostname
        char chostname[HOST_NAME_MAX+1];
        gethostname(chostname, HOST_NAME_MAX+1);
        hostnameStr = std::make_unique<std::string>(chostname);

        // get pid
        pid = getpid();
    }

    // appname is nawa by default
    appname = "nawa";

    ++instanceCount;
}

nawa::Log::Log(std::string appname_) noexcept : Log() {
    appname = std::move(appname_);
}

nawa::Log::Log(const nawa::Log &other) noexcept {
    appname = other.appname;
    ++instanceCount;
}

nawa::Log &nawa::Log::operator=(const nawa::Log &other) noexcept {
    if(this != &other) {
        appname = other.appname;
        ++instanceCount;
    }
    return *this;
}

nawa::Log::~Log() {
    --instanceCount;
    if(instanceCount == 0) {
        if(logFile.is_open()) {
            logFile.close();
        }
        locked = false;
        hostnameStr.reset(nullptr);
    }
}

void nawa::Log::setStream(std::ostream *os) noexcept {
    if(!locked) {
        out = os;
    }
}

void nawa::Log::setOutfile(const std::string &filename) {
    if(!locked) {
        if(logFile.is_open()) {
            logFile.close();
        }
        logFile.open(filename, std::ofstream::out | std::ofstream::app);
        if(!logFile) {
            throw UserException("nawa::Log::setOutfile", 1,
                    "Failed to open requested file for writing.");
        }
        out = &logFile;
    }
}

void nawa::Log::lockStream() noexcept {
    locked = true;
}

bool nawa::Log::isLocked() noexcept {
    return locked;
}

void nawa::Log::setAppname(std::string appname_) noexcept {
    appname = std::move(appname_);
}

void nawa::Log::write(const std::string &msg) {
    auto now = std::time(nullptr);

    std::lock_guard<std::mutex> l(outLock);
    *out << std::put_time(std::localtime(&now), "%b %d %H:%M:%S ") << *hostnameStr << ' ' << program_invocation_short_name
         << '[' << pid << "]: [" << appname << "] " << msg << std::endl;
}

void nawa::Log::operator()(const std::string &msg) {
    write(msg);
}
