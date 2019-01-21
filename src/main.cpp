/**
 * \file main.cpp
 * \brief Application main file for qsfrunner
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdexcept>
#include <fastcgi++/manager.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include <csignal>
#include "qsf/RequestHandler.h"
#include "qsf/Config.h"
#include "qsf/SysException.h"
#include "qsf/Log.h"
#include "qsf/Application.h"

namespace {
    // this will make the manager instance and loaded app accessible for signal handlers
    std::unique_ptr<Fastcgipp::Manager<Qsf::RequestHandler>> managerPtr;
    void* appOpen = nullptr;
    // use this for logging
    // TODO enable logging to log file through config
    Qsf::Log LOG;
}

// signal handler for SIGINT and SIGTERM
void shutdown(int signum) {
    LOG("Terminating on signal " + std::to_string(signum));
    // terminate worker threads
    if(managerPtr) {
        // should unblock managerPtr->join() and execute the rest of the program
        managerPtr->stop();
    }
    else {
        if(appOpen != nullptr) {
            // app has been already opened, close it
            dlclose(appOpen);
        }
        exit(0);
    }
}

// load a symbol from the app .so file
void* loadAppSymbol(const char* symbolName, const std::string& error) {
    void* symbol = dlsym(appOpen, symbolName);
    auto dlsymError = dlerror();
    if(dlsymError) {
        LOG(error + dlsymError);
        exit(1);
    }
    return symbol;
}

int main() {

    // set up signal handlers
    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);

    // read config.ini
    Qsf::Config config;
    try {
        config.read("config.ini");
    }
    catch(Qsf::SysException& e) {
        LOG("Fatal Error: Could not read or parse config.ini");
        return 1;
    }

    // privilege downgrade
    if(getuid() == 0) {
        if(!config.isSet({"privileges", "user"}) || !config.isSet({"privileges", "group"})) {
            LOG("Fatal Error: Username or password not correctly set in config.ini");
            return 1;
        }
        std::string username = config[{"privileges", "user"}];
        std::string groupname = config[{"privileges", "group"}];
        passwd* user = getpwnam(username.c_str());
        group* group = getgrnam(groupname.c_str());
        if(user == nullptr || group == nullptr) {
            LOG("Fatal Error: Username or groupname invalid");
            return 1;
        }
        if(user->pw_uid == 0 || group->gr_gid == 0) {
            LOG("WARNING: QSF will be running as user or group root. Security risk!");
        }
        if(setgid(group->gr_gid) != 0 || setuid(user->pw_uid) != 0) {
            LOG("Fatal Error: Could not set privileges");
            return 1;
        }
    } else {
        LOG("WARNING: Not starting as root, cannot set privileges");
    }

    // load application init function
    std::string appPath = config[{"application", "path"}];
    if(appPath.empty()) {
        LOG("Fatal Error: Application path not set in config file");
        return 1;
    }
    appOpen = dlopen(appPath.c_str(), RTLD_LAZY);
    if(!appOpen) {
        LOG(std::string("Fatal Error: Application file could not be loaded (main): ") + dlerror());
        return 1;
    }
    // reset dl errors
    dlerror();
    // load symbols and check for errors
    // first load qsf_version_major (defined in Application.h, included in Connection.h)
    // the version the app has been compiled for should match the version of this qsfrunner
    std::string appVersionError = "Fatal Error: Could not read QSF version from application: ";
    auto appQsfVersionMajor = (int*) loadAppSymbol("qsf_version_major", appVersionError);
    auto appQsfVersionMinor = (int*) loadAppSymbol("qsf_version_minor", appVersionError);
    if(*appQsfVersionMajor != qsf_version_major || *appQsfVersionMinor != qsf_version_minor) {
        LOG("Fatal Error: App has been compiled against another version of QSF.");
    }
    auto appInit = (Qsf::init_t*) loadAppSymbol("init", "Fatal Error: Could not load init function from application: ");

    // pass config and application to RequestHandler so it can load appHandleRequest
    Qsf::RequestHandler::setConfig(config, appOpen);

    // concurrency
    double cReal;
    try {
        cReal = config.isSet({"system", "threads"})
                ? std::stod(config[{"system", "threads"}]) : 1.0;
    }
    catch(std::invalid_argument& e) {
        LOG("WARNING: Invalid value given for system/concurrency given in the config file.");
        cReal = 1.0;
    }
    if(config[{"system", "concurrency"}] == "hardware") {
        cReal = std::max(1.0, std::thread::hardware_concurrency() * cReal);
    }
    auto cInt = static_cast<unsigned int>(cReal);

    // set up fastcgi manager
    managerPtr = std::make_unique<Fastcgipp::Manager<Qsf::RequestHandler>>(cInt);
    managerPtr->setupSignals();

    // socket handling
    std::string mode = config[{"fastcgi", "mode"}];
    if(mode == "tcp") {
        auto fastcgiListen = config[{"fastcgi", "listen"}];
        auto fastcgiPort = config[{"fastcgi", "port"}];
        if(fastcgiListen.empty()) fastcgiListen = "127.0.0.1";
        if(fastcgiPort.empty()) fastcgiPort = "8000";
        if(!managerPtr->listen(fastcgiListen.c_str(), fastcgiPort.c_str())) {
            LOG("Fatal Error: Could not create TCP socket");
            return 1;
        }
    }
    else if(mode == "unix") {
        uint32_t permissions = 0xffffffffUL;
        std::string permStr = config[{"fastcgi", "permissions"}];
        // convert the value from the config file
        if(!permStr.empty()) {
            const char* psptr = permStr.c_str();
            char* endptr;
            long perm = strtol(psptr, &endptr, 8);
            if(*endptr == '\0') {
                permissions = (uint32_t) perm;
            }
        }

        auto fastcgiSocketPath = config[{"fastcgi", "path"}];
        if(fastcgiSocketPath.empty()) {
            fastcgiSocketPath = "/etc/qsf/sock.d/qsf.sock";
        }
        auto fastcgiOwner = config[{"fastcgi", "owner"}];
        auto fastcgiGroup = config[{"fastcgi", "group"}];

        if(!managerPtr->listen(fastcgiSocketPath.c_str(), permissions,
                fastcgiOwner.empty() ? nullptr : fastcgiOwner.c_str(),
                fastcgiGroup.empty() ? nullptr : fastcgiGroup.c_str())) {
            LOG("Fatal Error: Could not create UNIX socket");
            return 1;
        }
    }
    else {
        LOG("Fatal Error: Unknown FastCGI socket mode in config.ini");
        return 1;
    }

    // before manager starts, init app
    // TODO receive filters from init somehow
    //  - mention that filters can only be set in init bc of multithreading
    Qsf::AppInit appInit1;
    appInit(appInit1);

    managerPtr->start();
    managerPtr->join();

    dlclose(appOpen);
    return 0;
}