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
#include <nawa/MimeMultipart.h>

using namespace nawa;
using namespace std;
namespace http = boost::network::http;
struct HttpHandler;
using HttpServer = http::server<HttpHandler>;

namespace {
    Log LOG;

    /**
     * Stores the raw post access level, as read from the config file.
     */
    enum class RawPostAccess {
        NEVER,
        NONSTANDARD,
        ALWAYS
    };

    auto sendServerError = [](HttpServer::connection_ptr &httpConn) {
        httpConn->set_status(HttpServer::connection::internal_server_error);
        httpConn->set_headers(unordered_multimap<string, string>({{"content-type", "text/html; charset=utf-8"}}));
        httpConn->write(generate_error_page(500));
    };
}

struct InputConsumingHttpHandler : public enable_shared_from_this<InputConsumingHttpHandler> {
    RequestHandler *requestHandler = nullptr;
    ConnectionInitContainer connectionInit;
    ssize_t maxPostSize;
    size_t expectedSize;
    string postBody;
    RawPostAccess rawPostAccess;

    InputConsumingHttpHandler(RequestHandler *requestHandler, ConnectionInitContainer connectionInit,
                              ssize_t maxPostSize, size_t expectedSize, RawPostAccess rawPostAccess)
            : requestHandler(requestHandler), connectionInit(move(connectionInit)), maxPostSize(maxPostSize),
              expectedSize(expectedSize), rawPostAccess(rawPostAccess) {}

    void operator()(HttpServer::connection::input_range input, boost::system::error_code ec,
                    size_t bytesTransferred, HttpServer::connection_ptr httpConn) {
        // TODO error handling?
        if (ec == boost::asio::error::eof) {
            LOG("Request could not be handled (EOF in netlib).");
            sendServerError(httpConn);
            return;
        }

        // too large?
        if (postBody.size() + bytesTransferred > maxPostSize) {
            sendServerError(httpConn);
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

        string const multipartContentType = "multipart/form-data";
        string const plainTextContentType = "text/plain";
        auto postContentType = to_lowercase(connectionInit.requestInit.environment["content-type"]);
        auto &requestInit = connectionInit.requestInit;

        if (rawPostAccess == RawPostAccess::ALWAYS) {
            requestInit.rawPost = make_shared<string>(postBody);
        }

        if (postContentType == "application/x-www-form-urlencoded") {
            requestInit.postContentType = postContentType;
            requestInit.postVars = split_query_string(postBody);
        } else if (postContentType.substr(0, multipartContentType.length()) == multipartContentType) {
            try {
                MimeMultipart postData(connectionInit.requestInit.environment["content-type"], move(postBody));
                for (auto const &p: postData.parts_) {
                    // find out whether the part is a file
                    if (!p.fileName.empty() || (!p.contentType.empty() &&
                                                p.contentType.substr(0, plainTextContentType.length()) !=
                                                plainTextContentType)) {
                        File pf;
                        pf.contentType = p.contentType;
                        pf.filename = p.fileName;
                        pf.size = p.content.size();
                        // TODO better solution can be used when shifting fcgi handler to use MimeMultipart
                        pf.dataPtr = shared_ptr<char[]>(new char[pf.size]);
                        memcpy(pf.dataPtr.get(), p.content.c_str(), pf.size);
                        requestInit.postFiles.insert({p.partName, move(pf)});
                    } else {
                        requestInit.postVars.insert({p.partName, p.content});
                    }
                }
            } catch (Exception const &) {}
        } else if (rawPostAccess == RawPostAccess::NONSTANDARD) {
            requestInit.rawPost = make_shared<string>(move(postBody));
        }

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

        RequestInitContainer requestInit;
        string serverPort = config[{"http", "port"}];
        requestInit.environment = {
                {"REMOTE_ADDR",  request.source.substr(0, request.source.find_first_of(':'))},
                {"REQUEST_URI",     request.destination},
                {"REMOTE_PORT",     to_string(request.source_port)},
                {"REQUEST_METHOD",  request.method},
                {"SERVER_ADDR",  config[{"http", "listen"}]},
                {"SERVER_PORT",     serverPort},
                {"SERVER_SOFTWARE", "NAWA Development Web Server"},
        };

        // evaluate request headers
        // TODO accept languages (split), split acceptContentTypes?, acceptCharsets (where to find?)
        //      - consistent names for other elements in req handlers?
        for (auto const &h: request.headers) {
            if (requestInit.environment.count(to_lowercase(h.name)) == 0) {
                requestInit.environment[to_lowercase(h.name)] = h.value;
            }
        }

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            std::stringstream baseUrl;

            // change following section if HTTPS should ever be implemented (copy from fastcgi)
            baseUrl << "http://" << requestInit.environment["host"];

            auto baseUrlStr = baseUrl.str();
            requestInit.environment["BASE_URL"] = baseUrlStr;

            // fullUrlWithQS is the full URL, e.g., https://www.example.com/test?a=b&c=d
            requestInit.environment["FULL_URL_WITH_QS"] = baseUrlStr + request.destination;

            // fullUrlWithoutQS is the full URL without query string, e.g., https://www.example.com/test
            baseUrl << request.destination.substr(0, request.destination.find_first_of('?'));
            requestInit.environment["FULL_URL_WITHOUT_QS"] = baseUrl.str();
        }

        if (request.destination.find_first_of('?') != string::npos) {
            requestInit.getVars = split_query_string(request.destination);
        }
        requestInit.cookieVars = parse_cookies(requestInit.environment["cookie"]);

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
                std::string rawPostStr = config[{"post", "raw_access"}];
                auto rawPostAccess = (rawPostStr == "never")
                                     ? RawPostAccess::NEVER : ((rawPostStr == "always") ? RawPostAccess::ALWAYS
                                                                                        : RawPostAccess::NONSTANDARD);

                auto contentLength = stoul(connectionInit.requestInit.environment.at("content-length"));
                ssize_t maxPostSize = stol(config[{"post", "max_size"}]) * 1024;

                if (contentLength > maxPostSize) {
                    sendServerError(httpConn);
                    return;
                }

                auto inputConsumingHandler = make_shared<InputConsumingHttpHandler>(requestHandler,
                                                                                    move(connectionInit), maxPostSize,
                                                                                    contentLength, rawPostAccess);
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
        throw Exception(__PRETTY_FUNCTION__, 2, "HTTP handler is not available.");
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
