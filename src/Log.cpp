/**
 * \file Log.cpp
 * \brief Implementation of the Log class.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

nawa::Log::Log() {
    out = &std::cerr;
    // get hostname
    char chostname[HOST_NAME_MAX+1];
    gethostname(chostname, HOST_NAME_MAX+1);
    hostname = chostname;
    // get pid
    pid = getpid();
    // appname is nawa by default
    appname = "nawa";
}

nawa::Log::Log(std::ostream *os) : Log() {
    out = os;
}

nawa::Log::Log(std::string filename) : Log() {
    setOutfile(std::move(filename));
}

nawa::Log::~Log() {
    if(logFile.is_open()) {
        logFile.close();
    }
}

void nawa::Log::setStream(std::ostream *os) {
    out = os;
}

void nawa::Log::setOutfile(std::string filename) {
    if(logFile.is_open()) {
        logFile.close();
    }
    logFile.open(filename, std::ofstream::out | std::ofstream::app);
    if(!logFile) {
        throw UserException("nawa::Log::setOutfile", 1, "Failed to open requested file for writing.");
    }
    out = &logFile;
}

void nawa::Log::setAppname(std::string appname_) {
    appname = std::move(appname_);
}

void nawa::Log::write(std::string msg) {
    auto now = std::time(nullptr);

    *out << std::put_time(std::localtime(&now), "%b %d %H:%M:%S ") << hostname << ' ' << program_invocation_short_name
         << '[' << pid << "]: [" << appname << "] " << msg << std::endl;
}

void nawa::Log::operator()(std::string msg) {
    write(std::move(msg));
}
