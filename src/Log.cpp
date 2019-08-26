/**
 * \file Log.cpp
 * \brief Implementation of the Log class.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <unistd.h>
#include <climits>
#include <soru/UserException.h>
#include <soru/Log.h>

soru::Log::Log() {
    out = &std::cerr;
    // get hostname
    char chostname[HOST_NAME_MAX+1];
    gethostname(chostname, HOST_NAME_MAX+1);
    hostname = chostname;
    // get pid
    pid = getpid();
}

soru::Log::Log(std::ostream *os) : Log() {
    out = os;
}

soru::Log::Log(std::string filename) : Log() {
    setOutfile(std::move(filename));
}

soru::Log::~Log() {
    if(logFile.is_open()) {
        logFile.close();
    }
}

void soru::Log::setStream(std::ostream *os) {
    out = os;
}

void soru::Log::setOutfile(std::string filename) {
    if(logFile.is_open()) {
        logFile.close();
    }
    logFile.open(filename, std::ofstream::out | std::ofstream::app);
    if(!logFile) {
        throw UserException("soru::Log::setOutfile", 1, "Failed to open requested file for writing.");
    }
    out = &logFile;
}

void soru::Log::write(std::string msg) {
    auto now = std::time(nullptr);

    *out << std::put_time(std::localtime(&now), "%b %d %H:%M:%S ") << hostname << ' ' << program_invocation_short_name
         << '[' << pid << "]: [soru] " << msg << std::endl;
}

void soru::Log::operator()(std::string msg) {
    write(std::move(msg));
}
