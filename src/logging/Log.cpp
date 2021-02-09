/**
 * \file Log.cpp
 * \brief Implementation of the Log class.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#include <atomic>
#include <climits>
#include <iomanip>
#include <mutex>
#include <nawa/Exception.h>
#include <nawa/logging/Log.h>
#include <unistd.h>

using namespace nawa;
using namespace std;

namespace {
    struct DestructionDetector {
        bool destructed = false;

        ~DestructionDetector() { destructed = true; }
    } destructionDetector;

    bool locked = false; /**< If true, the stream and outfile cannot be changed anymore. */
    ostream *out; /**< Stream to send the logging output to. */
    ofstream logFile; /**< Log file handle in case a file is used and managed by this class. */
    Log::Level outputLevel = Log::Level::INFORMATIONAL; /**< Output log level. */
    bool extendedFormat = false; /**< Use the extended, systemd-style logging. */
    unique_ptr<string> hostnameStr;
    pid_t pid = 0;
    atomic_uint instanceCount(0);
    mutex outLock;
}

struct Log::Impl {
    std::string appname; /**< Name of the current app, appears in brackets in the log. */
    Level defaultLevel; /**< Default log level of this logger object. */

    explicit Impl(Level defaultLevel = Level::INFORMATIONAL) : defaultLevel(defaultLevel) {}
};

Log::Log() noexcept {
    impl = make_unique<Impl>(Level::INFORMATIONAL);

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
    impl->appname = "nawa";

    ++instanceCount;
}

Log::Log(string appname_, Level level) noexcept: Log() {
    impl->appname = move(appname_);
    impl->defaultLevel = level;
}

Log::Log(Level level) noexcept: Log() {
    impl->defaultLevel = level;
}

Log::Log(const Log &other) noexcept {
    impl = make_unique<Impl>(*other.impl);
    ++instanceCount;
}

Log &Log::operator=(const Log &other) noexcept {
    if (this != &other) {
        *impl = *other.impl;
        ++instanceCount;
    }
    return *this;
}

Log::~Log() {
    if (!destructionDetector.destructed) {
        --instanceCount;
        if (instanceCount == 0) {
            if (logFile.is_open()) {
                logFile.close();
            }
            locked = false;
        }
    } else {
        if (logFile.is_open()) {
            logFile.close();
        }
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
            throw Exception(__PRETTY_FUNCTION__, 1,
                            "Failed to open requested file for writing.");
        }
        out = &logFile;
    }
}

void Log::setOutputLevel(Level level) {
    if (!locked) {
        outputLevel = level;
    }
}

void Log::setExtendedFormat(bool useExtendedFormat) {
    if (!locked) {
        extendedFormat = useExtendedFormat;
    }
}

void Log::lockStream() noexcept {
    locked = true;
}

bool Log::isLocked() noexcept {
    return locked;
}

void Log::setAppname(string appname_) noexcept {
    impl->appname = move(appname_);
}

void Log::setDefaultLogLevel(Level level) noexcept {
    impl->defaultLevel = level;
}

void Log::write(const std::string &msg) {
    write(msg, impl->defaultLevel);
}

void Log::write(const string &msg, Level level) {
    if (level <= outputLevel && outputLevel != Level::OFF && level != Level::OFF) {
        auto now = time(nullptr);
        lock_guard<mutex> l(outLock);
        if (extendedFormat) {
            *out << put_time(localtime(&now), "%b %d %H:%M:%S ") << *hostnameStr << ' '
                 << program_invocation_short_name << '[' << pid << "]: ";
        }
        cerr << "[" << impl->appname << "] " << msg << endl;
        out->flush();
    }
}

void Log::operator()(const std::string &msg) {
    write(msg, impl->defaultLevel);
}

void Log::operator()(const string &msg, Level level) {
    write(msg, level);
}
