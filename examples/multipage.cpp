/**
 * \file multipage.cpp
 * \brief A simple website consisting of multiple 'dynamic' pages and virtual directories
 * for html files and images, and access control.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
 *
 * This file is part of soru.
 *
 * soru is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * soru is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with soru.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <soru/Application.h>

using namespace std;
using namespace soru;

extern "C" int init(AppInit& appInit) {

    // enable access filtering
    appInit.accessFilters.filtersEnabled = true;

    // apply forward filters for images below '/test2/static/images' ...
    ForwardFilter imageFilter;
    imageFilter.pathFilter = {"test2", "static", "images"};
    imageFilter.extensionFilter = "png";
    imageFilter.basePath = "/var/www/multipage/images";
    appInit.accessFilters.forwardFilters.push_back(imageFilter);

    // ... and html below '/test2/static/html'
    ForwardFilter htmlFilter;
    htmlFilter.pathFilter = {"test2", "static", "html"};
    htmlFilter.extensionFilter = "html";
    htmlFilter.basePath = "/var/www/multipage/html";
    appInit.accessFilters.forwardFilters.push_back(htmlFilter);

    // authenticate access to all static resources
    AuthFilter authFilter;
    authFilter.pathFilter = {"test2", "static"};
    authFilter.authName = "Not for everyone!";
    authFilter.authFunction = [](std::string user, std::string password) -> bool {
        return (user == "test" && password == "supersecure");
    };
    appInit.accessFilters.authFilters.push_back(authFilter);

    // for an example of a BlockFilter and regex-based filtering see tests/sorutest.cpp

    return 0;
}

extern "C" int handleRequest(Connection& connection) {

    // we do not have to care about requests for static resources -- the filters are doing that for us!

    auto requestPath = connection.request.env.getRequestPath();

    connection.response << "<!DOCTYPE html><html><head>"
                           "<title>soru Multipage Example</title>"
                           "</head><body><p>Request Path Elements: ";
    for(auto const &e: requestPath) {
        connection.response << e << ", ";
    }
    connection.response << "</p>";

    if(requestPath.size() > 1) {

        // if the request path starts with "/test2/page1", show the following page
        if(requestPath.at(0) == "test2" && requestPath.at(1) == "page1") {
            connection.response << "<h1>First Page</h1>"
                                   "<p>Lorem ipsum sit dolor</p>"
                                   "</body></html>";

            return 0;
        }

    }

    // otherwise, the index will be displayed
    connection.response << "<h1>Index</h1>"
                           "<ul>"
                           "<li><a href=\"/test2/page1\">First Page</a></li>"
                           "<li><a href=\"/test2/static/html/somedocument.html\">A static HTML document</a></li>"
                           "</ul></body></html>";

    return 0;
}