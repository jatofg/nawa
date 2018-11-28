#include <iostream>
#include <stdexcept>
#include <fastcgi++/manager.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include "RequestHandler.h"
#include "Config.h"
#include "SysException.h"

int main() {
    // read config.ini
    Qsf::Config config;
    try {
        config.read("config.ini");
    }
    catch(Qsf::SysException& e) {
        std::cerr << "Fatal Error: Could not read or parse config.ini" << std::endl;
        return 1;
    }

    // privilege downgrade
    if(getuid() == 0) {
        if(!config.isSet({"privileges", "user"}) || !config.isSet({"privileges", "group"})) {
            std::cerr << "Fatal Error: Username or password not correctly set in config.ini" << std::endl;
            return 1;
        }
        std::string username = config[{"privileges", "user"}];
        std::string groupname = config[{"privileges", "group"}];
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
    // TODO introduce new function, e.g. version(), in the app and check if compiled for correct QSF release
    //      - can we make this automatic somehow, i.e., having sth in an included header file (Connection.h)
    std::string appPath = config[{"application", "path"}];

    if(appPath.empty()) {
        std::cerr << "Fatal Error: Application path not set in config file" << std::endl;
        return 1;
    }
    void* appOpen = dlopen(appPath.c_str(), RTLD_LAZY);
    if(!appOpen) {
        std::cerr << "Fatal Error: Application file could not be loaded (main): " << dlerror() << std::endl;
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

    // pass config and application to RequestHandler so it can load appHandleRequest
    Qsf::RequestHandler::setConfig(config, appOpen);

    // concurrency
    double cReal;
    try {
        cReal = config.isSet({"system", "threads"})
                ? std::stod(config[{"system", "threads"}]) : 1.0;
    }
    catch(std::invalid_argument& e) {
        std::cerr << "WARNING: Invalid value given for system/concurrency given in the config file." << std::endl;
        cReal = 1.0;
    }

    if(config[{"system", "concurrency"}] == "hardware") {
        cReal = std::max(1.0, std::thread::hardware_concurrency() * cReal);
    }
    auto cInt = static_cast<unsigned int>(cReal);

    Fastcgipp::Manager<Qsf::RequestHandler> manager(cInt);
    manager.setupSignals();

    // socket handling
    std::string mode = config[{"fastcgi", "mode"}];
    if(mode == "tcp") {
        auto fastcgiListen = config[{"fastcgi", "listen"}];
        auto fastcgiPort = config[{"fastcgi", "port"}];
        if(fastcgiListen.empty()) fastcgiListen = "127.0.0.1";
        if(fastcgiPort.empty()) fastcgiPort = "8000";
        if(!manager.listen(fastcgiListen.c_str(), fastcgiPort.c_str())) {
            std::cerr << "Fatal Error: Could not create TCP socket" << std::endl;
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

        if(!manager.listen(fastcgiSocketPath.c_str(), permissions,
                fastcgiOwner.empty() ? nullptr : fastcgiOwner.c_str(),
                fastcgiGroup.empty() ? nullptr : fastcgiGroup.c_str())) {
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

    dlclose(appOpen);
    return 0;
}