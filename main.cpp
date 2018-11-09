#include <iostream>
#include <fastcgi++/manager.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include "RequestHandler.h"
#include "inih/INIReader.h"

int main() {
    // read config.ini
    INIReader reader("config.ini");
    if(reader.ParseError() < 0) {
        std::cerr << "Fatal Error: Could not read or parse config.ini" << std::endl;
        return 1;
    }

    // privilege downgrade
    if(getuid() == 0) {
        std::string username = reader.Get("privileges", "user", "-1");
        std::string groupname = reader.Get("privileges", "group", "-1");
        if(username == "-1" || groupname == "-1") {
            std::cerr << "Fatal Error: Username or password not correctly set in config.ini" << std::endl;
            return 1;
        }
        passwd* user = getpwnam(username.c_str());
        group* group = getgrnam(groupname.c_str());
        if(user == nullptr || group == nullptr) {
            std::cerr << "Fatal Error: Username or groupname invalid" << std::endl;
            return 1;
        }
        if(user->pw_uid == 0 || group->gr_gid == 0) {
            std::cerr << "WARNING: QSF will be running as user or group root. Security risk!" << std::endl;
        }
        if(setgid(group->gr_gid) != 0 || setuid(user->pw_uid) != 0) {
            std::cerr << "Fatal Error: Could not set privileges" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "WARNING: Not starting as root, cannot set privileges" << std::endl;
    }

    // load application init function
    std::string appPath = reader.Get("application", "path", "");

    if(appPath.empty()) {
        std::cerr << "Fatal Error: Application path not set in config file" << std::endl;
        return 1;
    }
    void* appOpen = dlopen(appPath.c_str(), RTLD_LAZY);
    if(!appOpen) {
        std::cerr << "Fatal Error: Application file could not be loaded" << std::endl;
        return 1;
    }
    // reset dl errors
    dlerror();
    // load symbols and check for errors
    auto appInit = (Qsf::init_t*) dlsym(appOpen, "init");
    auto dlsymErr = dlerror();
    if(dlsymErr) {
        std::cerr << "Fatal Error: Could not load init function from application: " << dlsymErr << std::endl;
        return 1;
    }
    auto appHandleRequest = (Qsf::handleRequest_t*) dlsym(appOpen, "handleRequest");
    dlsymErr = dlerror();
    if(dlsymErr) {
        std::cerr << "Fatal Error: Could not load handleRequest function from application: " << dlsymErr << std::endl;
        return 1;
    }
    dlclose(appOpen);

    // set post config and pass application path to RequestHandler so it can load appHandleRequest
    // raw_access is translated to an integer according to the macros defined in RequestHandler.h
    std::string rawPostStr = reader.Get("post", "raw_access", "nonstandard");
    uint rawPost = (rawPostStr == "never") ? 0 : ((rawPostStr == "nonstandard") ? 1 : 2);
    Qsf::RequestHandler::setConfig(static_cast<size_t>(reader.GetInteger("post", "max_size", 0)) * 1024, rawPost, appPath);

    // concurrency
    auto cReal = std::max(1.0, reader.GetReal("system", "threads", 1.0));
    if(reader.Get("system", "concurrency", "fixed") == "hardware") {
        cReal = std::max(1.0, std::thread::hardware_concurrency()*cReal);
    }
    auto cInt = static_cast<unsigned int>(cReal);

    Fastcgipp::Manager<Qsf::RequestHandler> manager(cInt);
    manager.setupSignals();

    // socket handling
    std::string mode = reader.Get("fastcgi", "mode", "none");
    if(mode == "tcp") {
        if(!manager.listen(reader.Get("fastcgi", "listen", "127.0.0.1").c_str(),
                reader.Get("fastcgi", "port", "8000").c_str())) {
            std::cerr << "Fatal Error: Could not create TCP socket" << std::endl;
            return 1;
        }
    }
    else if(mode == "unix") {
        uint32_t permissions = 0xffffffffUL;
        std::string permStr = reader.Get("fastcgi", "permissions", "-1");
        // convert the value from the config file
        if(permStr != "-1") {
            const char* psptr = permStr.c_str();
            char* endptr;
            long perm = strtol(psptr, &endptr, 8);
            if(*endptr == '\0') {
                permissions = (uint32_t) perm;
            }
        }
        std::string ownerStr = reader.Get("fastcgi", "owner", "-1");
        std::string groupStr = reader.Get("fastcgi", "group", "-1");
        if(!manager.listen(reader.Get("fastcgi", "path", "/etc/qsf/sock.d/qsf.sock").c_str(), permissions,
                (ownerStr == "-1") ? nullptr : ownerStr.c_str(), (groupStr == "-1") ? nullptr : groupStr.c_str() )) {
            std::cerr << "Fatal Error: Could not create UNIX socket" << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "Fatal Error: Unknown FastCGI socket mode in config.ini" << std::endl;
        return 1;
    }

    // before manager starts, init app
    appInit();

    manager.start();
    manager.join();
    return 0;
}