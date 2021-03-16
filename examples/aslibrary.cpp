/**
 * \file aslibrary.cpp
 * \brief An example on how to use NAWA as a library.
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

#include <iostream>
#include <nawa/Exception.h>
#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/connection/Connection.h>

using namespace nawa;
using namespace std;

int main() {

    // You can do the initialization right here, no need for an init function.

    // Load the config from a config.ini file, placed in the directory from where this app is executed.
    // Of course, you could also provide an absolute path or let the user provide the location via a command line argument.
    // The categories privileges, application, and system (except for the request handler) are irrelevant when using
    // NAWA as a library.
    Config config;
    try {
        config.read("config.ini");
    } catch (Exception const& e) {
        cerr << "Could not read config.ini file: " << e.getMessage() << endl;
    }
    // You don't have to use an ini file of course, you could also fill the config manually with all necessary options
    // by using config.set(key, value).
    // Also have a look at the nawa::Config docs for details :)

    // Now we need a function which handles the requests. It can be any (Connection&) -> int function,
    // here, we use a lambda for simplicity.
    auto handlingFunction = [](Connection& connection) -> int {
        // Our simple example app will just print "Hello World!" for every request
        // and therefore do exactly the same thing as examples/helloworld.cpp
        // (but in a more complicated way)
        connection.responseStream() << "Hello World!";
        return 0;
    };

    // set up the NAWA request handler
    unique_ptr<RequestHandler> requestHandler;
    try {
        // The last argument is the concurrency, which is the number of worker threads for processing requests
        requestHandler = RequestHandler::newRequestHandler(handlingFunction, config, 1);
    } catch (Exception const& e) {
        cerr << "NAWA request handler could not be created: " << e.getMessage() << endl;
    }

    // At this point, you should consider doing a privilege downgrade (if your app is meant to be started as root), as
    // running with root privileges is a security risk (as you hopefully know).

    // If you want to use access filtering, create a set of filters like this (that's optional, of course):
    AccessFilterList accessFilters;
    // Add filters, as shown in the manual or examples/multipage.cpp
    // ...
    // Then pass them to the request handler:
    requestHandler->setAccessFilters(accessFilters);

    // Now we can start handling requests! Request handling will happen in separate threads, the start() function
    // will return immediately, and you can continue to do other things here while requests are being served.
    try {
        requestHandler->start();
    } catch (Exception const& e) {
        cerr << "NAWA request handling could not be started: " << e.getMessage() << endl;
    }

    // You can control request handling at any time by calling:
    // requestHandler->stop()       to stop accepting new requests and terminating the request handlers as soon as
    //                              existing ones have been served
    // requestHandler->terminate()  to terminate request handling immediately
    // None of these functions will block until termination has completed, only join() will ever block.
    // Tip: Use signal handlers to control request handling.

    // If you want to hot-swap the config, filters, or request handling function, use:
    // requestHandler->reconfigure(handlingFunction, accessFilters, config);
    // (see docs of nawa::RequestHandler::reconfigure() -- you can use std::nullopt for things you don't want to
    // reconfigure)

    // Now, before your main function returns, make sure to block until request handling has terminated:
    requestHandler->join();

    return 0;
}
