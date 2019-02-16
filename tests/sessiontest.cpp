//
// Created by tobias on 11/02/19.
//

#include <random>
#include "app.h"

using namespace Qsf;
using namespace Qsf::Types;

int init(AppInit& appInit) {

    return 0;
}

int handleRequest(Connection& connection) {
    // run this a lot of times in parallel (flood requests) to test multi-threading
    // for example with curl: $ curl -b SESSION=... http://t1.local/test?it=[1-100]
    std::random_device rd;
    unsigned int cm[1000];

    connection.session.start();

    // set session variables
    for(int i = 0; i < 1000; ++i) {
        // fill with a random uint from urandom
        cm[i] = rd();
        connection.session.set("sessiontest" + std::to_string(i), cm[i]);
    }

    // set some other session variables in an alternating fashion
    bool desc = false;
    if(connection.session.isSet("descending") && connection.session["descending"].get<bool>()) {
        desc = true;
        connection.session.set("descending", false);
    }
    else {
        connection.session.set("descending", true);
    }
    for(int i = 0; i < 1000; ++i) {
        int val = desc ? 1000-i : i;
        connection.session.set("sessioncount" + std::to_string(i), val);
    }

    // get sessiontest variables and compare - this should actually fail now and then in a multithreading env
    // so matchcount does not necessarily have to be 1000
    int matchcount = 0;
    for(int i = 0; i < 1000; ++i) {
        if(connection.session["sessiontest" + std::to_string(i)].get<unsigned int>() == cm[i]) {
            ++matchcount;
        }
    }
    connection.response << "Match count for sessiontest vars: " << matchcount << "\n";

    // consistency check for the sessioncount variables - failcount should definitely be zero!
    int failcount = 0;
    for(int i = 0; i < 1000; ++i) {
        auto val = connection.session["sessioncount" + std::to_string(i)].get<int>();
        if(val != i && val != 1000-i) {
            ++failcount;
        }
    }
    connection.response << "Fail count for sessioncount vars: " << failcount << "\n";

    return 0;
}