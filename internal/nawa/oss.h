/**
 * \file oss.h
 * \brief This file contains helpers for operating-system specific stuff.
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

#ifndef NAWA_OPERATINGSYSTEMSPECIFIC_H
#define NAWA_OPERATINGSYSTEMSPECIFIC_H

#include <climits>
#include <nawa/systemconfig.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace nawa::oss {
    inline time_t getLastModifiedTimeOfFile(struct stat const& fileStat) {
#ifdef NAWA_OS_LINUX
        return fileStat.st_mtim.tv_sec;
#else
        return fileStat.st_mtimespec.tv_sec;
#endif
    }

    inline std::string getSystemHostname() {
#ifdef NAWA_OS_LINUX
        char chostname[HOST_NAME_MAX + 1];
#else
        char chostname[_POSIX_HOST_NAME_MAX + 1];
#endif
        gethostname(chostname, sizeof chostname);
        return std::string(chostname);
    }

    inline std::string getProgramInvocationName() {
#ifdef NAWA_OS_LINUX
        return std::string(program_invocation_short_name);
#else
        return std::string(getprogname());
#endif
    }

#ifdef NAWA_OS_LINUX
    inline gid_t* getGIDPtrForGetgrouplist(gid_t* in) {
        return in;
    }
#else
    inline int* getGIDPtrForGetgrouplist(gid_t* in) {
        return (int*) in;
    }
#endif
}

#endif//NAWA_OPERATINGSYSTEMSPECIFIC_H
