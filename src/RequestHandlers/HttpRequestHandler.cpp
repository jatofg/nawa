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
#include <nawa/Connection.h>

using namespace nawa;
using namespace std;
namespace http = boost::network::http;
struct HttpHandler;
using HttpServer = http::server<HttpHandler>;

struct HttpHandler {
    RequestHandler *requestHandler = nullptr;
    Config config;

    void operator()(HttpServer::request const &request, HttpServer::connection_ptr httpConn) {
        // TODO use read / create read callback to read headers (and hopefully environment?)
        //      - outsource parsing POST, use POST parsing in fastcgi handler too, then?
        //      - for environment, headers: the request object should have some interesting members

        // TODO fill
        RequestInitContainer requestInit;
        ConnectionInitContainer connectionInit;
        connectionInit.requestInit = move(requestInit);
        connectionInit.config = config;

        connectionInit.flushCallback = [&httpConn](FlushCallbackContainer flushInfo) {
            if (!flushInfo.flushedBefore) {
                httpConn->set_status(HttpServer::connection::status_t(flushInfo.status));
                httpConn->set_headers(flushInfo.headers);
            }
            httpConn->write(flushInfo.body);
        };

        Connection connection(connectionInit);
        requestHandler->handleRequest(connection);
        connection.flushResponse();

    }
};

struct nawa::HttpRequestHandler::HttpHandlerAdapter {
    unique_ptr<HttpHandler> handler;
    unique_ptr<HttpServer> server;
    int concurrency = 1;
    vector<thread> threadPool;
};

HttpRequestHandler::HttpRequestHandler(HandleRequestFunction handleRequestFunction, Config config_,
                                       int concurrency) {
    setAppRequestHandler(move(handleRequestFunction));
    setConfig(move(config_));

    httpHandler = make_unique<HttpHandlerAdapter>();
    httpHandler->handler = make_unique<HttpHandler>();
    httpHandler->handler->requestHandler = this;
    httpHandler->handler->config = config;
    HttpServer::options httpServerOptions(*httpHandler->handler);

    // set options from config
    string listenAddr = config[{"http", "listen"}].empty() ? "127.0.0.1" : config[{"http", "listen"}];
    string listenPort = config[{"http", "port"}].empty() ? "8080" : config[{"http", "port"}];
    bool reuseAddr = (config[{"http", "reuseaddr"}] != "off");
    httpHandler->server = make_unique<HttpServer>(
            httpServerOptions.address(listenAddr).port(listenPort).reuse_address(reuseAddr));

    if (concurrency > 0) {
        httpHandler->concurrency = concurrency;
    }

    try {
        httpHandler->server->listen();
    } catch (exception const &e) {
        throw Exception(__PRETTY_FUNCTION__, 1,
                        string("Could not listen to host/port: ") + e.what());
    }
}

HttpRequestHandler::~HttpRequestHandler() = default;

void HttpRequestHandler::start() {
    if (httpHandler && httpHandler->server) {
        try {
            for (int i = 0; i < httpHandler->concurrency; ++i) {
                httpHandler->threadPool.emplace_back([this] { httpHandler->server->run(); });
            }
        } catch (exception const &e) {
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
        for (auto &t: httpHandler->threadPool) {
            t.join();
        }
    }
}
