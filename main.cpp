#include <iostream>
#include <fastcgi++/manager.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
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

    Fastcgipp::Manager<Qsf::RequestHandler> manager;
    manager.setupSignals();
    manager.listen("127.0.0.1", "8000");
    manager.start();
    manager.join();
    return 0;
}