/**
 * \file main.cpp
 * \brief Application main file for nawarun.
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

#include <stdexcept>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include <csignal>
#include <thread>
#include <nawa/Config.h>
#include <nawa/Log.h>
#include <nawa/Application.h>
#include <nawa/RequestHandlers/RequestHandler.h>
#include <nawa/Exception.h>

using namespace nawa;
using namespace std;

namespace {

    // this will make the request handler and loaded app accessible for signal handlers
    unique_ptr<RequestHandler> requestHandlerPtr;
    void* appOpen = nullptr;

    // use this for logging
    // TODO enable logging to log file through config
    Log LOG;

    // Types of functions that need to be accessed from NAWA applications
    using init_t = int(AppInit&); /**< Type for the init() function of NAWA apps. */
    using handleRequest_t = int(Connection&); /**< Type for the handleRequest(Connection) function of NAWA apps. */

}

// signal handler for SIGINT, SIGTERM, and SIGUSR1
void shutdown(int signum) {
    LOG("Terminating on signal " + to_string(signum));

    // terminate worker threads
    if(requestHandlerPtr) {
        // should stop
        requestHandlerPtr->stop();

        // this normally doesn't work, so try harder
        sleep(10);
        if(requestHandlerPtr && signum != SIGUSR1) {
            LOG("Enforcing termination now, ignoring pending requests.");
            requestHandlerPtr->terminate();
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
void* loadAppSymbol(const char* symbolName, const string& error) {
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

    // set up logging
    Log::lockStream();

    // read config file
    Config config;
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
    catch(Exception& e) {
        LOG("Fatal Error: " + e.getMessage());
        return 1;
    }

    // prepare privilege downgrade and check for errors (downgrade will happen after socket setup)
    auto initialUID = getuid();
    uid_t privUID = -1;
    gid_t privGID = -1;
    vector<gid_t> supplementaryGroups;
    if(initialUID == 0) {
        if(!config.isSet({"privileges", "user"}) || !config.isSet({"privileges", "group"})) {
            LOG("Fatal Error: Username or password not correctly set in config.ini.");
            return 1;
        }
        string username = config[{"privileges", "user"}];
        string groupname = config[{"privileges", "group"}];
        passwd* privUser;
        group* privGroup;
        privUser = getpwnam(username.c_str());
        privGroup = getgrnam(groupname.c_str());
        if(privUser == nullptr || privGroup == nullptr) {
            LOG("Fatal Error: Username or groupname invalid");
            return 1;
        }
        privUID = privUser->pw_uid;
        privGID = privGroup->gr_gid;
        if(privUID == 0 || privGID == 0) {
            LOG("WARNING: nawarun will be running as user or group root. Security risk!");
        }
        else {
            // get supplementary groups for non-root user
            int n = 0;
            getgrouplist(username.c_str(), privGID, nullptr, &n);
            supplementaryGroups.resize(n, 0);
            if(getgrouplist(username.c_str(), privGID, &supplementaryGroups[0], &n) != n) {
                LOG("WARNING: Could not get supplementary groups for user " + username);
                supplementaryGroups = {privGID};
            }
        }
    } else {
        LOG("WARNING: Not starting as root, cannot set privileges.");
    }

    // load application init function
    string appPath = config[{"application", "path"}];
    if(appPath.empty()) {
        LOG("Fatal Error: Application path not set in config file.");
        return 1;
    }
    appOpen = dlopen(appPath.c_str(), RTLD_LAZY);
    if(!appOpen) {
        LOG(string("Fatal Error: Application file could not be loaded: ") + dlerror());
        return 1;
    }

    // reset dl errors
    dlerror();

    // load symbols and check for errors
    // first load nawa_version_major (defined in Application.h, included in Connection.h)
    // the version the app has been compiled against should match the version of this nawarun
    string appVersionError = "Fatal Error: Could not read nawa version from application: ";
    auto appNawaVersionMajor = (int*) loadAppSymbol("nawa_version_major", appVersionError);
    auto appNawaVersionMinor = (int*) loadAppSymbol("nawa_version_minor", appVersionError);
    if(*appNawaVersionMajor != nawa_version_major || *appNawaVersionMinor != nawa_version_minor) {
        LOG("Fatal Error: App has been compiled against another version of NAWA.");
        return 1;
    }
    auto appInit = (init_t *) loadAppSymbol("init", "Fatal Error: Could not load init function from application: ");
    auto appHandleRequest = (handleRequest_t *) loadAppSymbol("handleRequest",
                                                              "Fatal Error: Could not load handleRequest function from application: ");

    // concurrency
    double cReal;
    try {
        cReal = config.isSet({"system", "threads"})
                ? stod(config[{"system", "threads"}]) : 1.0;
    }
    catch(invalid_argument& e) {
        LOG("WARNING: Invalid value given for system/concurrency given in the config file.");
        cReal = 1.0;
    }
    if(config[{"system", "concurrency"}] == "hardware") {
        cReal = max(1.0, thread::hardware_concurrency() * cReal);
    }
    auto cInt = static_cast<unsigned int>(cReal);

    // pass config, app function, and concurrency to RequestHandler
    // already here to make (socket) preparation possible before privilege downgrade
    try {
        requestHandlerPtr = RequestHandler::newRequestHandler(appHandleRequest, config, cInt);
    } catch (const Exception& e) {
        LOG("Fatal Error: " + e.getMessage());
        return 1;
    }

    // do privilege downgrade
    if(initialUID == 0) {
        if(privUID != 0 && privGID != 0 && setgroups(supplementaryGroups.size(), &supplementaryGroups[0]) != 0) {
            LOG("Fatal Error: Could not set supplementary groups.");
            return 1;
        }
        if(setgid(privGID) != 0 || setuid(privUID) != 0) {
            LOG("Fatal Error: Could not set privileges.");
            return 1;
        }
    }

    // before manager starts, init app
    {
        AppInit appInit1;
        appInit1.config = config;
        appInit1.numThreads = cInt;
        auto initReturn = appInit(appInit1);

        // init function of the app should return 0 on sucess
        if(initReturn != 0) {
            LOG("Fatal Error: App init function returned " + to_string(initReturn) + " -- exiting.");
            return 1;
        }

        // init could have altered the config, take it over
        requestHandlerPtr->setAppInit(appInit1);
        requestHandlerPtr->setConfig(appInit1.config);
    }

    try {
        requestHandlerPtr->start();
    } catch (const Exception &e) {
        LOG("Fatal Error: " + e.getMessage());
    }

    requestHandlerPtr->join();

    // if segfaults occur during shutdown in certain situations, it might be necessary to call Session::destroy.

    dlclose(appOpen);
    exit(0);
}