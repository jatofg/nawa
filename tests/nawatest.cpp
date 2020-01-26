/**
 * \file nawatest.cpp
 * \brief NAWA app with some basic tests for filters, encoding, environment, and sessions.
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

#include <nawa/Application.h>
#include <iostream>
#include <nawa/Encoding.h>
#include <nawa/Session.h>

using namespace nawa;

int init(nawa::AppInit& appInit) {

    // enable access filtering
    appInit.accessFilters.filtersEnabled = true;

    // apply a forward filter for images
    ForwardFilter forwardFilter;
    forwardFilter.pathFilter = {{"test", "images"}, {"test2", "images"}};
    forwardFilter.extensionFilter = {"png", "jpg", "svg"};
    forwardFilter.basePath = "/home/tobias/Pictures";
    appInit.accessFilters.forwardFilters.push_back(forwardFilter);

    // send 404 error for non-image files in /test{2}/images
    BlockFilter blockFilter;
    blockFilter.pathFilter = {{"test", "images"}, {"test2", "images"}};
    blockFilter.extensionFilter = {"png", "jpg", "svg"};
    blockFilter.invertExtensionFilter = true;
    blockFilter.status = 404;
    appInit.accessFilters.blockFilters.push_back(blockFilter);

    // apply a block filter for everything that is not in /test{2} or /test{2}/images (and some more restrictions)
    BlockFilter blockFilter2;
    blockFilter2.invert = true;
    blockFilter2.regexFilterEnabled = true;
    blockFilter2.regexFilter.assign(R"(/test2?(/images)?(/[A-Za-z0-9_\-]*\.?[A-Za-z]{2,4})?)");
    blockFilter2.status = 404;
    appInit.accessFilters.blockFilters.push_back(blockFilter2);

    // authenticate access to the images directory
    AuthFilter authFilter;
    authFilter.pathFilter = {{"test", "images"}};
    authFilter.authName = "Not for everyone!";
    authFilter.authFunction = [](std::string user, std::string password) -> bool {
        return (user == "test" && password == "supersecure");
    };
    appInit.accessFilters.authFilters.push_back(authFilter);

    return 0;
}

int handleRequest(Connection &connection) {

    // start a session
    connection.session.start();

    connection.setCookie("TEST", "test");
    Cookie policy;
    policy.httpOnly = true;
    connection.setCookiePolicy(policy);

    std::string encoded = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
    std::string decoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";

    connection.response << "<!DOCTYPE html>\n"
                  "<html><head><title>Test</title></head><body>"
                  "<p>Hello World! HTML string: " << Encoding::htmlEncode(decoded, true) << "</p>"
                  "<p>Client IP: " << Encoding::htmlEncode(connection.request.env["remoteAddress"]) << "</p>"
                  "<p>Request URI: (" << connection.request.env.getRequestPath().size() << " elements): "
                  << connection.request.env["requestUri"] << "</p>"
                  "<p>HTTPS status: " << connection.request.env["https"] << "</p>"
                  "<p>SERVER_NAME: " << connection.request.env["serverName"] << "</p>"
                  "<p>Server software: " << connection.request.env["serverSoftware"] << "</p>"
                  "<p>Base URL: " << connection.request.env["baseUrl"] << "</p>"
                  "<p>Full URL with QS: " << connection.request.env["fullUrlWithQS"] << "</p>"
                  "<p>Full URL without QS: " << connection.request.env["fullUrlWithoutQS"] << "</p>"
                  ;

    // test privileges
    auto currentUid = getuid();
    auto currentGid = getgid();
    auto currentEUid = geteuid();
    auto currentEGid = getegid();
    int groupCount = getgroups(0, nullptr);
    connection.response << "<p>Privileges: uid = " << currentUid << "; gid = " << currentGid << "; euid = "
                        << currentEUid << "; egid = " << currentEGid << "</p>";
    std::vector<gid_t> currentGL(groupCount, 0);
    if(getgroups(groupCount, &currentGL[0]) >= 0) {
        connection.response << "<p>Current groups: ";
        for(auto const &e: currentGL) {
            connection.response << e << ", ";
        }
        connection.response << "</p>";
    }

    // alternative: connection.session["test"].isSet()
    if(connection.session.isSet("test")) {
        connection.response << "<p>Session available! Value: " << connection.session["test"].get<std::string>() << "</p>";
        connection.session.invalidate();
        connection.session.start();
        connection.session.set("test", std::string("and even more blah"));
        
    }
    else {
        connection.session.set("test", "blah blah blah");
        connection.response << "<p>There was no session yet, but now there should be one!" << "</p>";
    }

    std::string encodedDecoded = Encoding::htmlDecode(encoded);
    if(decoded == encodedDecoded) {
        connection.response << "<p>yay!</p>";
    }
    else {
        connection.response << "<p>" << Encoding::htmlEncode(encodedDecoded) <<"</p>";
    }

    connection.flushResponse();

    connection.response << "<p>Hello World 2!</p>"
                  "</body></html>";

    return 0;
}
