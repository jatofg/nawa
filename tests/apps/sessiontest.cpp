/**
 * \file sessiontest.cpp
 * \brief Test simultaneous access to session variables in multi-threading environments. Use curl, for example, to 
 * simulate simultaneous accesses (see tests/sessiontest.sh for an example).
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

#include <random>
#include <nawa/Application.h>

using namespace nawa;
using namespace std;

int init(AppInit &appInit) {

    return 0;
}

int handleRequest(Connection &connection) {
    // run this a lot of times in parallel (flood requests) to test multi-threading
    // for example with curl: $ curl -b SESSION=... http://t1.local/test?it=[1-100]
    // see sessiontest.sh for an example script
    random_device rd;
    unsigned int cm[1000];

    connection.session.start();

    // set session variables
    for (int i = 0; i < 1000; ++i) {
        // fill with a random uint from urandom
        cm[i] = rd();
        connection.session.set("sessiontest" + to_string(i), cm[i]);
    }

    // set some other session variables in an alternating fashion
    bool desc = false;
    if (connection.session.isSet("descending") && any_cast<bool>(connection.session["descending"])) {
        desc = true;
        connection.session.set("descending", false);
    } else {
        connection.session.set("descending", true);
    }
    for (int i = 0; i < 1000; ++i) {
        int val = desc ? 1000 - i : i;
        connection.session.set("sessioncount" + to_string(i), val);
    }

    // get sessiontest variables and compare - this should actually fail now and then in a multithreading env
    // so matchcount does not necessarily have to be 1000
    int matchcount = 0;
    for (int i = 0; i < 1000; ++i) {
        if (any_cast<unsigned int>(connection.session["sessiontest" + to_string(i)]) == cm[i]) {
            ++matchcount;
        }
    }
    connection.response << "Match count for sessiontest vars: " << matchcount << "\n";

    // consistency check for the sessioncount variables - failcount should definitely be zero!
    int failcount = 0;
    for (int i = 0; i < 1000; ++i) {
        auto val = any_cast<int>(connection.session["sessioncount" + to_string(i)]);
        if (val != i && val != 1000 - i) {
            ++failcount;
        }
    }
    connection.response << "Fail count for sessioncount vars: " << failcount << "\n";

    return 0;
}