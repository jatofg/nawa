/**
 * \file gpctest.cpp
 * \brief NAWA app to test POST, GET, and COOKIE input.
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

#include <nawa/application.h>
#include <nawa/util/encoding.h>

using namespace nawa;
using namespace std;

int init(AppInit& appInit) {
    return 0;
}

int handleRequest(Connection& connection) {

    auto& resp = connection.responseStream();
    auto& req = connection.request();
    auto& session = connection.session();

    session.start();

    if (req.get().count("download") == 1 && session.isSet(req.get()["download"])) {
        try {
            auto downloadFile = any_cast<File const>(session[req.get()["download"]]);
            connection.setHeader("content-type", downloadFile.contentType());
            connection.setHeader("content-disposition", "attachment; filename=\"" + downloadFile.filename() + "\"");
            connection.setHeader("content-length", to_string(downloadFile.size()));
            connection.setResponseBody(downloadFile.toString());
            return 0;
        } catch (bad_any_cast const& e) {
            resp << "Bad any cast: " << e.what();
            return 0;
        }
    }

    resp << "<!DOCTYPE html>\n"
            "<html><head><title>NAWA GPC Test</title></head><body>\n";

    auto printEncoded = [&](string const& k, string const& v) {
        resp << "<li>[" << encoding::htmlEncode(k) << "] = " << encoding::htmlEncode(v) << "</li>";
    };

    if (req.cookie()) {
        resp << "<p>COOKIE vars:</p><ul>";
        for (auto const& [k, v] : req.cookie()) {
            printEncoded(k, v);
        }
        resp << "</ul>";
    }

    if (req.get()) {
        resp << "<p>GET vars:</p><ul>";
        for (auto const& [k, v] : req.get()) {
            printEncoded(k, v);
        }
        resp << "</ul>";
    }

    if (req.post()) {
        resp << "<p>POST vars (without files):</p><ul>";
        for (auto const& [k, v] : req.post()) {
            printEncoded(k, v);
        }
        resp << "</ul>";
    }

    if (req.post().hasFiles()) {
        resp << "<p>POST files:</p><ul>";
        for (auto const& [k, v] : req.post().getFileMultimap()) {
            // skip empty files
            if (v.size() == 0) {
                continue;
            }

            resp << "<li>[" << encoding::htmlEncode(k) << "]: "
                 << R"(<a href="?download=)" << encoding::urlEncode(v.filename()) << R"(">)"
                 << encoding::htmlEncode(v.filename())
                 << "; size: "
                 << v.size()
                 << "; content type: "
                 << encoding::htmlEncode(v.contentType())
                 << "</a></li>";

            // Saving files in session and not even cleaning them up at some point is something that clearly
            // shouldn't be done outside of a test app :)
            session.set(v.filename(), v);
        }
        resp << "</ul>";
    }

    if (req.post().getRaw()) {
        resp << "<p>Raw POST data:</p><pre>" << *req.post().getRaw() << "</pre>";
    }

    resp << R"(<p>Some POST form:</p><form name="testform" method="post" action="?" enctype="multipart/form-data">)"
         << R"(<p>Field one (1): <input type="text" name="one"></p>)"
         << R"(<p>Field one (2): <input type="text" name="one"></p>)"
         << R"(<p>Field two: <input type="text" name="two"></p>)"
         << R"(<p>Field fileupload (1): <input type="file" name="fileupload"></p>)"
         << R"(<p>Field fileupload (2): <input type="file" name="fileupload"></p>)"
         << R"(<p><input type="submit" value="Go" name="submit"></p></form>)";

    return 0;
}
