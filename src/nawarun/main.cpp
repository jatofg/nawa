/**
 * \file main.cpp
 * \brief Application main file for nawarun.
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
#include <csignal>
#include <dlfcn.h>
#include <grp.h>
#include <nawa/Application.h>
#include <nawa/Exception.h>
#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/config/Config.h>
#include <nawa/logging/Log.h>
#include <pwd.h>
#include <stdexcept>
#include <thread>
#include <unistd.h>

using namespace nawa;
using namespace std;

namespace {
    unique_ptr<RequestHandler> requestHandlerPtr;
    string configFile;
    atomic<bool> readyToReconfigure(false);
    Log logger;

    // Types of functions that need to be accessed from NAWA applications
    using init_t = int(AppInit &); /**< Type for the init() function of NAWA apps. */
    using handleRequest_t = int(Connection &); /**< Type for the handleRequest(Connection) function of NAWA apps. */
}

// signal handler for SIGINT, SIGTERM, and SIGUSR1
void shutdown(int signum) {
    NLOG_INFO(logger, "Terminating on signal " << signum)

    // terminate worker threads
    if (requestHandlerPtr) {
        // should stop
        requestHandlerPtr->stop();

        // if this did not work, try harder after 10 seconds
        sleep(10);
        if (requestHandlerPtr && signum != SIGUSR1) {
            NLOG_INFO(logger, "Enforcing termination now, ignoring pending requests.")
            requestHandlerPtr->terminate();
        }
    } else {
        exit(0);
    }
}

// load a symbol from an app .so file
void *loadAppSymbol(void *appOpen, const char *symbolName, const string &error) {
    void *symbol = dlsym(appOpen, symbolName);
    auto dlsymError = dlerror();
    if (dlsymError) {
        throw Exception(__FUNCTION__, 11, error, dlsymError);
    }
    return symbol;
}

// free memory of an open app
void closeApp(void *appOpen) {
    dlclose(appOpen);
}

// get the number of threads to use from config
unsigned int getConcurrency(Config const &config) {
    double cReal;
    try {
        cReal = config.isSet({"system", "threads"})
                ? stod(config[{"system", "threads"}]) : 1.0;
    }
    catch (invalid_argument &e) {
        NLOG_WARNING(logger, "WARNING: Invalid value given for system/concurrency given in the config file.")
        cReal = 1.0;
    }
    if (config[{"system", "concurrency"}] == "hardware") {
        cReal = max(1.0, thread::hardware_concurrency() * cReal);
    }
    return static_cast<unsigned int>(cReal);
}

// load the init() and handleRequest() functions of an app
// please note: the init function also becomes unavailable when the HandleRequestFunctionWrapper is destructed
pair<init_t *, shared_ptr<HandleRequestFunctionWrapper>> loadAppFunctions(Config const &config) {
    // load application init function
    string appPath = config[{"application", "path"}];
    if (appPath.empty()) {
        throw Exception(__FUNCTION__, 1, "Application path not set in config file.");
    }
    void *appOpen = dlopen(appPath.c_str(), RTLD_LAZY);
    if (!appOpen) {
        throw Exception(__FUNCTION__, 2, "Application file could not be loaded.", dlerror());
    }

    // reset dl errors
    dlerror();

    // load symbols and check for errors
    // first load nawa_version_major (defined in Application.h, included in Connection.h)
    // the version the app has been compiled against should match the version of this nawarun
    string appVersionError = "Could not read nawa version from application.";
    auto appNawaVersionMajor = (int *) loadAppSymbol(appOpen, "nawa_version_major", appVersionError);
    auto appNawaVersionMinor = (int *) loadAppSymbol(appOpen, "nawa_version_minor", appVersionError);
    if (*appNawaVersionMajor != nawa_version_major || *appNawaVersionMinor != nawa_version_minor) {
        throw Exception(__FUNCTION__, 3, "App has been compiled against another version of NAWA.");
    }
    auto appInit = (init_t *) loadAppSymbol(appOpen, "init", "Could not load init function from application.");
    auto appHandleRequest = (handleRequest_t *) loadAppSymbol(appOpen, "handleRequest",
                                                              "Could not load handleRequest function from application.");
    return {appInit, make_shared<HandleRequestFunctionWrapper>(appHandleRequest, appOpen, closeApp)};
}

// signal handler for SIGHUP (reload configuration and app)
void reload(int signum) {
    if (requestHandlerPtr && readyToReconfigure) {
        NLOG_INFO(logger, "Reloading config and app on signal " << signum)
        readyToReconfigure = false;

        Config config;
        try {
            config.read(configFile);
        }
        catch (Exception const &e) {
            NLOG_ERROR(logger, "ERROR: Could not reload config: " << e.getMessage())
            NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
            NLOG_WARNING(logger, "WARNING: App will not be reloaded as well")
            readyToReconfigure = true;
            return;
        }

        init_t *appInit;
        shared_ptr<HandleRequestFunctionWrapper> appHandleRequest;
        try {
            tie(appInit, appHandleRequest) = loadAppFunctions(config);
        } catch (Exception const &e) {
            NLOG_ERROR(logger, "ERROR: Could not reload app: " << e.getMessage())
            NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
            NLOG_WARNING(logger, "WARNING: Configuration will be reloaded anyway")

            // just reload config, not app
            requestHandlerPtr->reconfigure(nullopt, nullopt, config);
            readyToReconfigure = true;
            return;
        }

        {
            AppInit appInitStruct(config, getConcurrency(config));
            auto initReturn = appInit(appInitStruct);

            // init function of the app should return 0 on success, otherwise we will not reload
            if (initReturn != 0) {
                NLOG_ERROR(logger,
                           "ERROR: App init function returned " << initReturn << " -- cancelling reload of app.")
                NLOG_WARNING(logger, "WARNING: Configuration will be reloaded anyway")

                // just reload config, not app
                requestHandlerPtr->reconfigure(nullopt, nullopt, config);
                readyToReconfigure = true;
                return;
            }

            // reconfigure everything
            requestHandlerPtr->reconfigure(appHandleRequest, appInitStruct.accessFilters(), appInitStruct.config());
            readyToReconfigure = true;
        }
    }
}

int main(int argc, char **argv) {

    // set up signal handlers
    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);
    signal(SIGUSR1, shutdown);
    signal(SIGHUP, reload);

    // read config file
    // nawarun will take the path to the config as an argument
    // if no argument was given, look for a config.ini in the current path
    if (argc > 1) {
        configFile = argv[1];
    } else {
        configFile = "config.ini";
    }
    Config config;
    try {
        config.read(configFile);
    }
    catch (Exception &e) {
        NLOG_ERROR(logger, "Fatal Error: Could not load config: " << e.getMessage())
        NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
        return 1;
    }

    // set up logging
    auto configuredLogLevel = config[{"logging", "level"}];
    if (configuredLogLevel == "off") {
        Log::setOutputLevel(Log::Level::OFF);
    } else if (configuredLogLevel == "error") {
        Log::setOutputLevel(Log::Level::ERROR);
    } else if (configuredLogLevel == "warning") {
        Log::setOutputLevel(Log::Level::WARNING);
    } else if (configuredLogLevel == "debug") {
        Log::setOutputLevel(Log::Level::DEBUG);
    }
    if (config[{"logging", "extended"}] == "on") {
        Log::setExtendedFormat(true);
    }
    Log::lockStream();

    // prepare privilege downgrade and check for errors (downgrade will happen after socket setup)
    auto initialUID = getuid();
    uid_t privUID = -1;
    gid_t privGID = -1;
    vector<gid_t> supplementaryGroups;
    if (initialUID == 0) {
        if (!config.isSet({"privileges", "user"}) || !config.isSet({"privileges", "group"})) {
            NLOG_ERROR(logger, "Fatal Error: Username or password not correctly set in config.ini.")
            return 1;
        }
        string username = config[{"privileges", "user"}];
        string groupname = config[{"privileges", "group"}];
        passwd *privUser;
        group *privGroup;
        privUser = getpwnam(username.c_str());
        privGroup = getgrnam(groupname.c_str());
        if (privUser == nullptr || privGroup == nullptr) {
            NLOG_ERROR(logger, "Fatal Error: Username or groupname invalid")
            return 1;
        }
        privUID = privUser->pw_uid;
        privGID = privGroup->gr_gid;
        if (privUID == 0 || privGID == 0) {
            NLOG_WARNING(logger, "WARNING: nawarun will be running as user or group root. Security risk!")
        } else {
            // get supplementary groups for non-root user
            int n = 0;
            getgrouplist(username.c_str(), privGID, nullptr, &n);
            supplementaryGroups.resize(n, 0);
            if (getgrouplist(username.c_str(), privGID, &supplementaryGroups[0], &n) != n) {
                NLOG_WARNING(logger, "WARNING: Could not get supplementary groups for user " << username)
                supplementaryGroups = {privGID};
            }
        }
    } else {
        NLOG_WARNING(logger, "WARNING: Not starting as root, cannot set privileges.")
    }

    // load init and handleRequest symbols from app
    init_t *appInit;
    shared_ptr<HandleRequestFunctionWrapper> appHandleRequest;
    try {
        tie(appInit, appHandleRequest) = loadAppFunctions(config);
    } catch (Exception const &e) {
        NLOG_ERROR(logger, "Fatal Error: " << e.getMessage())
        NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
        return 1;
    }

    // pass config, app function, and concurrency to RequestHandler
    // already here to make (socket) preparation possible before privilege downgrade
    auto concurrency = getConcurrency(config);
    try {
        requestHandlerPtr = RequestHandler::newRequestHandler(appHandleRequest, config, concurrency);
    } catch (const Exception &e) {
        NLOG_ERROR(logger, "Fatal Error: " << e.getMessage())
        NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
        return 1;
    }

    // do privilege downgrade
    if (initialUID == 0) {
        if (privUID != 0 && privGID != 0 && setgroups(supplementaryGroups.size(), &supplementaryGroups[0]) != 0) {
            NLOG_ERROR(logger, "Fatal Error: Could not set supplementary groups.")
            return 1;
        }
        if (setgid(privGID) != 0 || setuid(privUID) != 0) {
            NLOG_ERROR(logger, "Fatal Error: Could not set privileges.")
            return 1;
        }
    }

    // before manager starts, init app
    {
        AppInit appInitStruct(config, concurrency);
        auto initReturn = appInit(appInitStruct);

        // init function of the app should return 0 on success
        if (initReturn != 0) {
            NLOG_ERROR(logger, "Fatal Error: App init function returned " << initReturn << " -- exiting.")
            return 1;
        }

        // reconfigure request handler using access filters and (potentially altered by app init) config
        requestHandlerPtr->reconfigure(nullopt, appInitStruct.accessFilters(), appInitStruct.config());
    }

    try {
        requestHandlerPtr->start();
        readyToReconfigure = true;
    } catch (const Exception &e) {
        NLOG_ERROR(logger, "Fatal Error: " << e.getMessage())
        NLOG_DEBUG(logger, "Debug info: " << e.getDebugMessage())
    }

    requestHandlerPtr->join();

    // the request handler has to be destroyed before unloading the app (using dlclose)
    requestHandlerPtr.reset(nullptr);

    exit(0);
}