/**
 * \file nawatest.cpp
 * \brief NAWA app with some basic tests for filters, encoding, environment, and sessions.
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
#include <nawa/application.h>
#include <nawa/session/Session.h>
#include <nawa/util/encoding.h>
#include <unistd.h>

using namespace nawa;
using namespace std;

int init(nawa::AppInit& appInit) {

    // enable access filtering
    appInit.accessFilters().filtersEnabled() = true;

    // apply a forward filter for images
    ForwardFilter forwardFilter;
    forwardFilter.pathFilter() = {{"test", "images"},
                                  {"test2", "images"}};
    forwardFilter.extensionFilter() = {"png", "jpg", "svg"};
    forwardFilter.basePath() = "/home/tobias/Pictures";
    appInit.accessFilters().forwardFilters().push_back(forwardFilter);

    // send 404 error for non-image files in /test{2}/images
    BlockFilter blockFilter;
    blockFilter.pathFilter() = {{"test", "images"},
                                {"test2", "images"}};
    blockFilter.extensionFilter() = {"png", "jpg", "svg"};
    blockFilter.invertExtensionFilter() = true;
    blockFilter.status() = 404;
    appInit.accessFilters().blockFilters().push_back(blockFilter);

    // apply a block filter for everything that is not in /test{2} or /test{2}/images (and some more restrictions)
    BlockFilter blockFilter2;
    blockFilter2.invert() = true;
    blockFilter2.regexFilterEnabled() = true;
    blockFilter2.regexFilter().assign(R"(/test2?(/images)?(/[A-Za-z0-9_\-]*\.?[A-Za-z]{2,4})?)");
    blockFilter2.status() = 404;
    appInit.accessFilters().blockFilters().push_back(blockFilter2);

    // authenticate access to the images directory
    AuthFilter authFilter;
    authFilter.pathFilter() = {{"test", "images"}};
    authFilter.authName() = "Not for everyone!";
    authFilter.authFunction() = [](string user, string password) -> bool {
        return (user == "test" && password == "supersecure");
    };
    appInit.accessFilters().authFilters().push_back(authFilter);

    return 0;
}

int handleRequest(Connection& connection) {

    auto& resp = connection.responseStream();
    auto& req = connection.request();
    auto& session = connection.session();

    // start a session
    session.start();

    connection.setCookie("TEST", "test");
    connection.setCookiePolicy(Cookie().httpOnly(true));

    string encoded = R"(&lt;input type=&quot;text&quot; value=&quot;t&auml;&Aopf;&#x1D538;&#120120;st&quot;&gt;)";
    string decoded = R"(<input type="text" value="tä𝔸𝔸𝔸st">)";

    resp << "<!DOCTYPE html>\n"
            "<html><head><title>Test</title></head><body>"
            "<p>Hello World! HTML string: "
         << encoding::htmlEncode(decoded, true) << "</p>"
                                                   "<p>Client IP: "
         << encoding::htmlEncode(req.env()["REMOTE_ADDR"]) << "</p>"
                                                              "<p>Request URI: ("
         << req.env().getRequestPath().size() << " elements): "
         << req.env()["REQUEST_URI"] << "</p>"
                                        "<p>HTTPS status: "
         << req.env()["HTTPS"] << "</p>"
                                  "<p>SERVER_NAME: "
         << req.env()["SERVER_NAME"]
         << "</p>"
            "<p>Server software: "
         << req.env()["SERVER_SOFTWARE"] << "</p>"
                                            "<p>Base URL: "
         << req.env()["BASE_URL"] << "</p>"
                                     "<p>Full URL with QS: "
         << req.env()["FULL_URL_WITH_QS"] << "</p>"
                                             "<p>Full URL without QS: "
         << req.env()["FULL_URL_WITHOUT_QS"] << "</p>";

    // test privileges
    auto currentUid = getuid();
    auto currentGid = getgid();
    auto currentEUid = geteuid();
    auto currentEGid = getegid();
    int groupCount = getgroups(0, nullptr);
    resp << "<p>Privileges: uid = " << currentUid << "; gid = " << currentGid << "; euid = "
         << currentEUid << "; egid = " << currentEGid << "</p>";
    vector<gid_t> currentGL(groupCount, 0);
    if (getgroups(groupCount, &currentGL[0]) >= 0) {
        resp << "<p>Current groups: ";
        for (auto const& e : currentGL) {
            resp << e << ", ";
        }
        resp << "</p>";
    }

    // alternative: session["test"].isSet()
    if (session.isSet("test")) {
        resp << "<p>Session available! Value: " << any_cast<string>(session["test"])
             << "</p>";
        session.invalidate();
        session.start();
        session.set("test", string("and even more blah"));

    } else {
        session.set("test", "blah blah blah");
        resp << "<p>There was no session yet, but now there should be one!"
             << "</p>";
    }

    string encodedDecoded = encoding::htmlDecode(encoded);
    if (decoded == encodedDecoded) {
        resp << "<p>yay!</p>";
    } else {
        resp << "<p>" << encoding::htmlEncode(encodedDecoded) << "</p>";
    }

    connection.flushResponse();

    resp << "<p>Hello World 2!</p>"
            "</body></html>";

    return 0;
}
