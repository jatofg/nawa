/**
 * \file gpctest.cpp
 * \brief NAWA app to test POST, GET, and COOKIE input.
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

#include <nawa/Application.h>
#include <nawa/Encoding.h>

using namespace nawa;
using namespace std;

int init(AppInit &appInit) {
    return 0;
}

int handleRequest(Connection &connection) {

    connection.response << "<!DOCTYPE html>\n"
                           "<html><head><title>NAWA GPC Test</title></head><body>\n";

    auto printEncoded = [&](const string &k, const string &v) {
        connection.response << "<li>[" << Encoding::htmlEncode(k) << "] = " << Encoding::htmlEncode(v) << "</li>";
    };

    if (connection.request.cookie) {
        connection.response << "<p>COOKIE vars:</p><ul>";
        for (auto const& [k, v]: connection.request.cookie) {
            printEncoded(k, v);
        }
        connection.response << "</ul>";
    }

    if (connection.request.get) {
        connection.response << "<p>GET vars:</p><ul>";
        for (auto const& [k, v]: connection.request.get) {
            printEncoded(k, v);
        }
        connection.response << "</ul>";
    }

    if (connection.request.post) {
        connection.response << "<p>POST vars (without files):</p><ul>";
        for (auto const& [k, v]: connection.request.post) {
            printEncoded(k, v);
        }
        connection.response << "</ul>";
    }

    // TODO files -- rewrite files handling (files as multimap in Post)

    connection.response << R"(<p>Some POST form:</p><form name="testform" method="post" action="?" enctype="multipart/form-data">)"
                        << R"(<p>Field one (1): <input type="text" name="one"></p>)"
                        << R"(<p>Field one (2): <input type="text" name="one"></p>)"
                        << R"(<p>Field two: <input type="text" name="two"></p>)"
                        << R"(<p>Field fileupload (1): <input type="file" name="fileupload"></p>)"
                        << R"(<p>Field fileupload (2): <input type="file" name="fileupload"></p>)"
                        << R"(<p><input type="submit" value="Go" name="submit"></p></form>)";

    return 0;
}
