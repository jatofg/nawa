/**
 * \file HttpRequestHandler.cpp
 * \brief Implementation of the HttpRequestHandler class.
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

#include <nawa/RequestHandlers/HttpRequestHandler.h>
#include <nawa/Exception.h>
#include <boost/network/protocol/http/server.hpp>

using namespace nawa;
using namespace std;
namespace http = boost::network::http;
struct HttpHandler;
using HttpServer = http::server<HttpHandler>;

struct HttpHandler {
    void operator()(HttpServer::request const& request, HttpServer::connection_ptr connection) {
        HttpServer::string_type ip = source(request);
        unsigned int port = request.source_port;
        std::ostringstream data;
        data << "Hello, " << ip << ':' << port << '!';
        connection->set_status(HttpServer::connection::ok);
        connection->write(data.str());
    }
};

struct nawa::HttpRequestHandler::HttpHandlerAdapter {
    unique_ptr<HttpHandler> handler;
    unique_ptr<HttpServer> server;
    vector<thread> threadPool;
};

HttpRequestHandler::HttpRequestHandler(HandleRequestFunction handleRequestFunction, Config config_,
                                             int concurrency) {
    setAppRequestHandler(move(handleRequestFunction));
    setConfig(move(config_));

    httpHandler = make_unique<HttpHandlerAdapter>();
    httpHandler->handler = make_unique<HttpHandler>();
    HttpServer::options httpServerOptions(*httpHandler->handler);
    // TODO set options (host, port, ...) from config
    httpHandler->server = make_unique<HttpServer>(httpServerOptions);

    try {
        httpHandler->server->listen();
    } catch (exception const& e) {
        throw Exception(__PRETTY_FUNCTION__, 1,
                        string("Could not listen to host/port: ") + e.what());
    }
}

HttpRequestHandler::~HttpRequestHandler() = default;

void HttpRequestHandler::start() {
    if (httpHandler && httpHandler->server) {
        try {
            httpHandler->threadPool.emplace_back([this]{httpHandler->server->run();});
            // TODO num threads according to config, save thread somewhere?
        } catch (exception const& e) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            string("An error occurred during start of request handling:") + e.what());
        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 2, "FastCGI manager is not available.");
    }
}

void HttpRequestHandler::stop() noexcept {
    if (httpHandler && httpHandler->server) {
        httpHandler->server->stop();
    }
}

void HttpRequestHandler::terminate() noexcept {
    // TODO find a way to terminate
    if (httpHandler && httpHandler->server) {
        httpHandler->server->stop();
    }
}

void HttpRequestHandler::join() noexcept {
    if (httpHandler) {
        for(auto& t: httpHandler->threadPool) {
            t.join();
        }
    }
}
