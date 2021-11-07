/**
 * \file multipage.cpp
 * \brief A simple website consisting of multiple 'dynamic' pages and virtual directories
 * for html files and images, and access control.
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

#include <nawa/Application.h>

using namespace std;
using namespace nawa;

int init(AppInit& appInit) {

    // enable access filtering
    appInit.accessFilters().filtersEnabled() = true;

    // apply forward filters for images below '/test/static/images' and '/test2/static/images ...
    ForwardFilter imageFilter;
    imageFilter.pathFilter({{"test", "static", "images"},
                            {"test2", "static", "images"}});
    imageFilter.extensionFilter({"png", "jpeg", "jpg", "gif"});
    imageFilter.basePath("/var/www/multipage/images");
    appInit.accessFilters().forwardFilters().push_back(imageFilter);

    // and block everything else in these directories
    BlockFilter blockNonImages;
    blockNonImages.pathFilter({{"test", "static", "images"},
                               {"test2", "static", "images"}});
    blockNonImages.extensionFilter({"png", "jpeg", "jpg", "gif"});
    blockNonImages.invertExtensionFilter(true);
    blockNonImages.status(404);
    appInit.accessFilters().blockFilters().push_back(blockNonImages);

    // ... and html below '/test/static/html'
    ForwardFilter htmlFilter;
    htmlFilter.pathFilter({{"test", "static", "html"}});
    htmlFilter.extensionFilter({"html", "htm"});
    htmlFilter.basePath("/var/www/multipage/html");
    appInit.accessFilters().forwardFilters().push_back(htmlFilter);

    // and also block everything else there
    BlockFilter blockNonHtml;
    blockNonHtml.pathFilter({{"test", "static", "html"}});
    blockNonHtml.extensionFilter({"html", "htm"});
    blockNonHtml.invertExtensionFilter(true);
    blockNonHtml.status(404);
    appInit.accessFilters().blockFilters().push_back(blockNonHtml);

    // authenticate access to all static resources
    AuthFilter authFilter;
    authFilter.pathFilter({{"test", "static"},
                           {"test2", "static"}});
    authFilter.authName("Not for everyone!");
    authFilter.authFunction() = [](std::string user, std::string password) -> bool {
        return (user == "test" && password == "supersecure");
    };
    appInit.accessFilters().authFilters().push_back(authFilter);

    // for an example of regex-based filtering see tests/apps/nawatest.cpp

    return 0;
}

int handleRequest(Connection& connection) {

    // we do not have to care about requests for static resources -- the filters are doing that for us!

    // shortcuts
    auto& resp = connection.responseStream();
    auto requestPath = connection.request().env().getRequestPath();

    resp << "<!DOCTYPE html><html><head>"
            "<title>nawa Multipage Example</title>"
            "</head><body><p>Request Path Elements: ";
    for (auto const& e : requestPath) {
        resp << e << ", ";
    }
    resp << "</p>";

    if (requestPath.size() > 1) {

        // if the request path starts with "/test/page1", show the following page
        if (requestPath.at(0) == "test" && requestPath.at(1) == "page1") {
            resp << "<h1>First Page</h1>"
                    "<p>Lorem ipsum sit dolor</p>"
                    "</body></html>";

            return 0;
        }
    }

    // otherwise, the index will be displayed
    resp << "<h1>Index</h1>"
            "<ul>"
            "<li><a href=\"/test/page1\">First Page</a></li>"
            "<li><a href=\"/test/static/html/somedocument.html\">A static HTML document</a></li>"
            "</ul></body></html>";

    return 0;
}
