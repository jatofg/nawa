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

#include <nawa/Connection.h>
#include <nawa/Exception.h>
#include <nawa/Utils.h>
#include <nawa/RequestHandlers/HttpRequestHandler.h>
#include <boost/network/protocol/http/server.hpp>
#include <nawa/Log.h>

using namespace nawa;
using namespace std;
namespace http = boost::network::http;
struct HttpHandler;
using HttpServer = http::server<HttpHandler>;

namespace {
    Log LOG;
}

struct InputConsumingHttpHandler : public enable_shared_from_this<InputConsumingHttpHandler> {
    RequestHandler *requestHandler = nullptr;
    ConnectionInitContainer connectionInit;
    ssize_t maxPostSize;
    size_t expectedSize;
    string postBody;

    InputConsumingHttpHandler(RequestHandler *requestHandler, ConnectionInitContainer connectionInit,
                              ssize_t maxPostSize, size_t expectedSize)
            : requestHandler(requestHandler), connectionInit(move(connectionInit)), maxPostSize(maxPostSize),
              expectedSize(expectedSize) {}

    void operator()(HttpServer::connection::input_range input, boost::system::error_code ec,
                    size_t bytesTransferred, HttpServer::connection_ptr httpConn) {
        // TODO error handling?
        if (ec == boost::asio::error::eof) {
            LOG("Request could not be handled (EOF in netlib).");
            httpConn->write(generate_error_page(500));
            return;
        }

        // too large?
        // TODO also check header in advance, before even starting to read
        if (postBody.size() + bytesTransferred > maxPostSize) {
            // TODO headers & status have to be set as well when sending a 500 => create an extra function!
            httpConn->write(generate_error_page(500));
            return;
        }

        // fill POST body
        postBody.insert(postBody.end(), boost::begin(input), boost::end(input));

        // check whether even more data has to be read
        if (postBody.size() < expectedSize) {
            auto self = this->shared_from_this();
            httpConn->read([self](HttpServer::connection::input_range input,
                                  boost::system::error_code ec, std::size_t bytes_transferred,
                                  HttpServer::connection_ptr httpConn) {
                (*self)(input, ec, bytes_transferred, httpConn);
            });
            return;
        }

        // TODO insert POST/files into connectionInit => requires parsing!

        // finally handle the request
        Connection connection(connectionInit);
        requestHandler->handleRequest(connection);
        connection.flushResponse();

    }
};

struct HttpHandler {
    RequestHandler *requestHandler = nullptr;
    Config config;

    void operator()(HttpServer::request const &request, HttpServer::connection_ptr httpConn) {

        //   TODO outsource parsing POST, use POST parsing in fastcgi handler too, then?
        //      - for environment, headers: the request object should have some interesting members

        // TODO parse headers (not in the RH)
        RequestInitContainer requestInit;
        string serverPort = config[{"http", "port"}];
        requestInit.environment = {
                {"remoteAddress",  request.source.substr(0, request.source.find_first_of(':'))},
                {"requestUri",     request.destination},
                {"remotePort",     to_string(request.source_port)},
                {"requestMethod",  request.method},
                {"serverAddress",  config[{"http", "listen"}]},
                {"serverPort",     serverPort},
                {"serverSoftware", "NAWA Development Web Server"},
        };

        // evaluate request headers
        // TODO accept languages (split), split acceptContentTypes?, acceptCharsets (where to find?)
        //      - consistent names for other elements in req handlers?
        for (auto const &h: request.headers) {
            if (h.name == "Host") {
                requestInit.environment["host"] = h.value.substr(0, h.value.find_first_of(':'));
            } else if (h.name == "User-Agent") {
                requestInit.environment["userAgent"] = h.value;
            } else if (h.name == "Accept") {
                requestInit.environment["acceptContentTypes"] = h.value;
            } else if (requestInit.environment.count(to_lowercase(h.name)) == 0) {
                requestInit.environment[to_lowercase(h.name)] = h.value;
            }
        }

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            std::stringstream baseUrl;

            // change following section if HTTPS should ever be implemented (copy from fastcgi)
            baseUrl << "http://" << requestInit.environment["host"];
            if (serverPort != "80") {
                baseUrl << ":" << serverPort;
            }

            auto baseUrlStr = baseUrl.str();
            requestInit.environment["baseUrl"] = baseUrlStr;

            // fullUrlWithQS is the full URL, e.g., https://www.example.com/test?a=b&c=d
            requestInit.environment["fullUrlWithQS"] = baseUrlStr + request.destination;

            // fullUrlWithoutQS is the full URL without query string, e.g., https://www.example.com/test
            baseUrl << request.destination.substr(0, request.destination.find_first_of('?'));
            requestInit.environment["fullUrlWithoutQS"] = baseUrl.str();
        }

        requestInit.getVars = split_query_string(request.destination);

        // TODO POST, COOKIE

        ConnectionInitContainer connectionInit;
        connectionInit.requestInit = move(requestInit);
        connectionInit.config = config;

        connectionInit.flushCallback = [httpConn](FlushCallbackContainer flushInfo) {
            if (!flushInfo.flushedBefore) {
                httpConn->set_status(HttpServer::connection::status_t(flushInfo.status));
                httpConn->set_headers(flushInfo.headers);
            }
            httpConn->write(flushInfo.body);
        };

        // is there POST data to be handled?
        if (request.method == "POST" && connectionInit.requestInit.environment.count("content-length")) {
            try {
                auto contentLength = stoul(connectionInit.requestInit.environment.at("content-length"));
                ssize_t maxPostSize = stol(config[{"post", "max_size"}]) * 1024;

                if (contentLength > maxPostSize) {
                    httpConn->write(generate_error_page(500));
                    return;
                }

                auto inputConsumingHandler = make_shared<InputConsumingHttpHandler>(requestHandler,
                                                                                    move(connectionInit), maxPostSize,
                                                                                    contentLength);
                httpConn->read([inputConsumingHandler](HttpServer::connection::input_range input,
                                                       boost::system::error_code ec, size_t bytesTransferred,
                                                       HttpServer::connection_ptr httpConn) {
                    (*inputConsumingHandler)(input, ec, bytesTransferred, httpConn);
                });
            } catch (const invalid_argument &) {} catch (const out_of_range &) {}
            return;
        }

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

    LOG.setAppname("HttpRequestHandler");

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
