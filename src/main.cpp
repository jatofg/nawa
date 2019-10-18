/**
 * \file main.cpp
 * \brief Application main file for nawarun.
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

#include <iostream>
#include <stdexcept>
#include <fastcgi++/manager.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include <csignal>
#include <nawa/RequestHandler.h>
#include <nawa/Config.h>
#include <nawa/SysException.h>
#include <nawa/Log.h>
#include <nawa/Application.h>

namespace {
    // this will make the manager instance and loaded app accessible for signal handlers
    std::unique_ptr<Fastcgipp::Manager<nawa::RequestHandler>> managerPtr;
    void* appOpen = nullptr;
    // use this for logging
    // TODO enable logging to log file through config
    nawa::Log LOG;
}

// signal handler for SIGINT, SIGTERM, and SIGUSR1
void shutdown(int signum) {
    LOG("Terminating on signal " + std::to_string(signum));
    // terminate worker threads
    if(managerPtr) {
        // should unblock managerPtr->join() and execute the rest of the program
        // (in fact it doesn't, but at least new connections should not be accepted anymore)
        // TODO find the bug or reason for this behavior in libfastcgi++
        managerPtr->stop();
        // this normally doesn't work, so try harder
        sleep(10);
        if(managerPtr && signum != SIGUSR1) {
            LOG("Enforcing termination now, ignoring pending requests.");
            managerPtr->terminate();
        }
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

int main(int argc, char** argv) {

    // set up signal handlers
    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);
    signal(SIGUSR1, shutdown);

    // read config.ini
    nawa::Config config;
    try {
        // nawarun will take the path to the config as an argument
        // if no argument was given, look for a config.ini in the current path
        if(argc > 1) {
            config.read(argv[1]);
        }
        else {
            config.read("config.ini");
        }
    }
    catch(nawa::SysException& e) {
        LOG("Fatal Error: Could not read or parse the configuration file");
        return 1;
    }

    // prepare privilege downgrade and check for errors (downgrade will happen after socket setup)
    auto privUid = getuid();
    passwd* privUser;
    group* privGroup;
    if(privUid == 0) {
        if(!config.isSet({"privileges", "user"}) || !config.isSet({"privileges", "group"})) {
            LOG("Fatal Error: Username or password not correctly set in config.ini");
            return 1;
        }
        std::string username = config[{"privileges", "user"}];
        std::string groupname = config[{"privileges", "group"}];
        privUser = getpwnam(username.c_str());
        privGroup = getgrnam(groupname.c_str());
        if(privUser == nullptr || privGroup == nullptr) {
            LOG("Fatal Error: Username or groupname invalid");
            return 1;
        }
        if(privUser->pw_uid == 0 || privGroup->gr_gid == 0) {
            LOG("WARNING: nawarun will be running as user or group root. Security risk!");
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
    // first load nawa_version_major (defined in Application.h, included in Connection.h)
    // the version the app has been compiled for should match the version of this nawarun
    std::string appVersionError = "Fatal Error: Could not read nawa version from application: ";
    auto appNawaVersionMajor = (int*) loadAppSymbol("nawa_version_major", appVersionError);
    auto appNawaVersionMinor = (int*) loadAppSymbol("nawa_version_minor", appVersionError);
    if(*appNawaVersionMajor != nawa_version_major || *appNawaVersionMinor != nawa_version_minor) {
        LOG("Fatal Error: App has been compiled against another version of nawa.");
    }
    auto appInit = (nawa::init_t*) loadAppSymbol("init", "Fatal Error: Could not load init function from application: ");

    // pass config and application to RequestHandler so it can load appHandleRequest
    // (config will be saved later, here it will only be used to load the appHandleRequest)
    // (app function already loaded here so that errors can be detected before setting up the socket)
    nawa::RequestHandler::setAppRequestHandler(config, appOpen);

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
    managerPtr = std::make_unique<Fastcgipp::Manager<nawa::RequestHandler>>(cInt);
    //managerPtr->setupSignals();

    // socket handling
    std::string mode = config[{"fastcgi", "mode"}];
    if(mode == "tcp") {
        auto fastcgiListen = config[{"fastcgi", "listen"}];
        auto fastcgiPort = config[{"fastcgi", "port"}];
        if(fastcgiListen.empty()) fastcgiListen = "127.0.0.1";
        const char* fastcgiListenC = fastcgiListen.c_str();
        if(fastcgiListen == "all") fastcgiListenC = nullptr;
        if(fastcgiPort.empty()) fastcgiPort = "8000";
        if(!managerPtr->listen(fastcgiListenC, fastcgiPort.c_str())) {
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
            fastcgiSocketPath = "/etc/nawarun/sock.d/nawarun.sock";
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
    if(config[{"fastcgi", "reuseaddr"}] != "off") {
        managerPtr->reuseAddress(true);
    }

    // do privilege downgrade
    if(privUid == 0) {
        if(setgid(privGroup->gr_gid) != 0 || setuid(privUser->pw_uid) != 0) {
            LOG("Fatal Error: Could not set privileges");
            return 1;
        }
    }

    // before manager starts, init app
    {
        nawa::AppInit appInit1;
        appInit1.config = config;
        appInit1.numThreads = cInt;
        auto initReturn = appInit(appInit1);

        // init function of the app should return 0 on sucess
        if(initReturn != 0) {
            LOG("Fatal Error: App init function returned " + std::to_string(initReturn) + " -- exiting");
            return 1;
        }
        nawa::RequestHandler::setConfig(appInit1);
    }

    managerPtr->start();
    managerPtr->join();

    // explicitly destroy AppInit and clear session data to avoid a segfault
    nawa::RequestHandler::destroyEverything();

    dlclose(appOpen);
    exit(0);
}