/**
 * \file HttpRequestHandler.cpp
 * \brief Implementation of the HttpRequestHandler class.
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

#include <boost/network/protocol/http/server.hpp>
#include <nawa/Exception.h>
#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/RequestHandler/impl/HttpRequestHandler.h>
#include <nawa/connection/Connection.h>
#include <nawa/connection/ConnectionInitContainer.h>
#include <nawa/logging/Log.h>
#include <nawa/util/MimeMultipart.h>
#include <nawa/util/utils.h>

using namespace nawa;
using namespace std;
namespace http = boost::network::http;
struct HttpHandler;
using HttpServer = http::server<HttpHandler>;

namespace {
    Log logger;

    /**
     * Stores the raw post access level, as read from the config file.
     */
    enum class RawPostAccess {
        NEVER,
        NONSTANDARD,
        ALWAYS
    };

    auto sendServerError = [](HttpServer::connection_ptr& httpConn) {
        httpConn->set_status(HttpServer::connection::internal_server_error);
        httpConn->set_headers(unordered_multimap<string, string>({{"content-type", "text/html; charset=utf-8"}}));
        httpConn->write(utils::generateErrorPage(500));
    };

    inline string getListenAddr(shared_ptr<Config const> const& configPtr) {
        return (*configPtr)[{"http", "listen"}].empty() ? "127.0.0.1" : (*configPtr)[{"http", "listen"}];
    }

    inline string getListenPort(shared_ptr<Config const> const& configPtr) {
        return (*configPtr)[{"http", "port"}].empty() ? "8080" : (*configPtr)[{"http", "port"}];
        ;
    }
}// namespace

struct InputConsumingHttpHandler : public enable_shared_from_this<InputConsumingHttpHandler> {
    RequestHandler* requestHandler = nullptr;
    ConnectionInitContainer connectionInit;
    ssize_t maxPostSize;
    size_t expectedSize;
    string postBody;
    RawPostAccess rawPostAccess;

    InputConsumingHttpHandler(RequestHandler* requestHandler, ConnectionInitContainer connectionInit,
                              ssize_t maxPostSize, size_t expectedSize, RawPostAccess rawPostAccess)
        : requestHandler(requestHandler), connectionInit(move(connectionInit)), maxPostSize(maxPostSize),
          expectedSize(expectedSize), rawPostAccess(rawPostAccess) {}

    void operator()(HttpServer::connection::input_range input, boost::system::error_code ec,
                    size_t bytesTransferred, HttpServer::connection_ptr httpConn) {
        if (ec == boost::asio::error::eof) {
            NLOG_ERROR(logger, "Request with POST data could not be handled.")
            NLOG_DEBUG(logger, "Debug info: boost::asio::error::eof in cpp-netlib while processing POST data")
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
                                  boost::system::error_code ec, size_t bytes_transferred,
                                  HttpServer::connection_ptr httpConn) {
                (*self)(input, ec, bytes_transferred, httpConn);
            });
            return;
        }

        string const multipartContentType = "multipart/form-data";
        string const plainTextContentType = "text/plain";
        auto postContentType = utils::toLowercase(connectionInit.requestInit.environment["content-type"]);
        auto& requestInit = connectionInit.requestInit;

        if (rawPostAccess == RawPostAccess::ALWAYS) {
            requestInit.rawPost = make_shared<string>(postBody);
        }

        if (postContentType == "application/x-www-form-urlencoded") {
            requestInit.postContentType = postContentType;
            requestInit.postVars = utils::splitQueryString(postBody);
        } else if (postContentType.substr(0, multipartContentType.length()) == multipartContentType) {
            try {
                MimeMultipart postData(connectionInit.requestInit.environment["content-type"], move(postBody));
                for (auto const& p : postData.parts()) {
                    // find out whether the part is a file
                    if (!p.filename().empty() || (!p.contentType().empty() &&
                                                  p.contentType().substr(0, plainTextContentType.length()) !=
                                                          plainTextContentType)) {
                        File pf = File(p.content()).contentType(p.contentType()).filename(p.filename());
                        requestInit.postFiles.insert({p.partName(), move(pf)});
                    } else {
                        requestInit.postVars.insert({p.partName(), p.content()});
                    }
                }
            } catch (Exception const&) {}
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
    RequestHandler* requestHandler = nullptr;

    void operator()(HttpServer::request const& request, HttpServer::connection_ptr httpConn) {
        auto configPtr = requestHandler->getConfig();

        RequestInitContainer requestInit;
        requestInit.environment = {
                {"REMOTE_ADDR", request.source.substr(0, request.source.find_first_of(':'))},
                {"REQUEST_URI", request.destination},
                {"REMOTE_PORT", to_string(request.source_port)},
                {"REQUEST_METHOD", request.method},
                {"SERVER_ADDR", getListenAddr(configPtr)},
                {"SERVER_PORT", getListenPort(configPtr)},
                {"SERVER_SOFTWARE", "NAWA Development Web Server"},
        };

        // evaluate request headers
        // TODO accept languages (split), split acceptContentTypes?, acceptCharsets (where to find?)
        //      - consistent names for other elements in req handlers?
        for (auto const& h : request.headers) {
            if (requestInit.environment.count(utils::toLowercase(h.name)) == 0) {
                requestInit.environment[utils::toLowercase(h.name)] = h.value;
            }
        }

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            stringstream baseUrl;

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
            requestInit.getVars = utils::splitQueryString(request.destination);
        }
        requestInit.cookieVars = utils::parseCookies(requestInit.environment["cookie"]);

        ConnectionInitContainer connectionInit;
        connectionInit.requestInit = move(requestInit);
        connectionInit.config = (*configPtr);

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
                string rawPostStr = (*configPtr)[{"post", "raw_access"}];
                auto rawPostAccess = (rawPostStr == "never")
                                             ? RawPostAccess::NEVER
                                             : ((rawPostStr == "always") ? RawPostAccess::ALWAYS
                                                                         : RawPostAccess::NONSTANDARD);

                auto contentLength = stoul(connectionInit.requestInit.environment.at("content-length"));
                ssize_t maxPostSize = stol((*configPtr)[{"post", "max_size"}]) * 1024;

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
            } catch (invalid_argument const&) {
            } catch (out_of_range const&) {}
            return;
        }

        Connection connection(connectionInit);
        requestHandler->handleRequest(connection);
        connection.flushResponse();
    }
};

struct HttpRequestHandler::Data {
    unique_ptr<HttpHandler> handler;
    unique_ptr<HttpServer> server;
    int concurrency = 1;
    vector<thread> threadPool;
    bool requestHandlingActive = false;
    bool joined = false;
};

HttpRequestHandler::HttpRequestHandler(shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction,
                                       Config config,
                                       int concurrency) {
    data = make_unique<Data>();

    setAppRequestHandler(move(handleRequestFunction));
    setConfig(move(config));
    auto configPtr = getConfig();

    logger.setAppname("HttpRequestHandler");

    data->handler = make_unique<HttpHandler>();
    data->handler->requestHandler = this;
    HttpServer::options httpServerOptions(*data->handler);

    // set options from config
    string listenAddr = getListenAddr(configPtr);
    string listenPort = getListenPort(configPtr);
    bool reuseAddr = (*configPtr)[{"http", "reuseaddr"}] != "off";
    data->server = make_unique<HttpServer>(
            httpServerOptions.address(listenAddr).port(listenPort).reuse_address(reuseAddr));

    if (concurrency > 0) {
        data->concurrency = concurrency;
    }

    try {
        data->server->listen();
    } catch (exception const& e) {
        throw Exception(__PRETTY_FUNCTION__, 1,
                        "Could not listen to host/port.", e.what());
    }
}

HttpRequestHandler::~HttpRequestHandler() {
    if (data->requestHandlingActive && !data->joined) {
        data->server->stop();
    }
    if (!data->joined) {
        for (auto& t : data->threadPool) {
            t.join();
        }
        data->threadPool.clear();
    }
}

void HttpRequestHandler::start() {
    if (data->requestHandlingActive) {
        return;
    }
    if (data->joined) {
        throw Exception(__PRETTY_FUNCTION__, 10, "HttpRequestHandler was already joined.");
    }
    if (data->server) {
        try {
            for (int i = 0; i < data->concurrency; ++i) {
                data->threadPool.emplace_back([this] { data->server->run(); });
            }
            data->requestHandlingActive = true;
        } catch (exception const& e) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            string("An error occurred during start of request handling."),
                            e.what());
        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 2, "HTTP handler is not available.");
    }
}

void HttpRequestHandler::stop() noexcept {
    if (data->joined) {
        return;
    }
    if (data->server) {
        data->server->stop();
    }
}

void HttpRequestHandler::terminate() noexcept {
    // TODO find (implement in fork of cpp-netlib) a way to forcefully terminate
    if (data->joined) {
        return;
    }
    if (data->server) {
        data->server->stop();
    }
}

void HttpRequestHandler::join() noexcept {
    if (data->joined) {
        return;
    }
    for (auto& t : data->threadPool) {
        t.join();
    }
    data->joined = true;
    data->threadPool.clear();
}
