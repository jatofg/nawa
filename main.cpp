#include <iostream>
#include <fastcgi++/manager.hpp>
#include "RequestHandler.h"

int main() {
    Fastcgipp::Manager<RequestHandler> manager;
    manager.setupSignals();
    manager.listen("127.0.0.1", "8000");
    manager.start();
    manager.join();
    return 0;
}