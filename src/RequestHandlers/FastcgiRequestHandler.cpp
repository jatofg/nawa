/**
 * \file FastcgiRequestHandler.cpp
 * \brief Implementation of the FastcgiRequestHandler class.
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

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <fastcgi++/log.hpp>
#include <nawa/RequestHandlers/RequestHandler.h>
#include <nawa/RequestHandlers/FastcgiRequestHandler.h>
#include <nawa/Log.h>
#include <nawa/Connection.h>
#include <nawa/Utils.h>
#include <nawa/Exception.h>

using namespace nawa;
using namespace std;

namespace {
    Log logger("fastcgi");

    /**
     * Stores the raw post access level, as read from the config file.
     */
    enum class RawPostAccess {
        NEVER,
        NONSTANDARD,
        ALWAYS
    };
}

class FastcgippRequestAdapter : public Fastcgipp::Request<char> {
    shared_ptr<string> rawPost;
public:
    /**
     * Request handling happens through a child class of Fastcgipp::Request. All necessary objects from outside that
     * we need are induced by the fastcgipp-lite manager (which has been modified to achieve that).
     */
    FastcgippRequestAdapter() : Fastcgipp::Request<char>() {}

    /**
     * The response function is responsible for handling the request in fastcgipp-lite.
     * @return true, to indicate that we handled the request
     */
    bool response() override;

    /**
     * The inProcessor function tells fastcgipp-lite processes POST data and tells fastcgipp-lite what to do with it
     * afterwards.
     * @return false, to indicate that the multimaps should still be filled by the library
     */
    bool inProcessor() override;
};

bool FastcgippRequestAdapter::response() {
    RequestInitContainer requestInit;
    auto requestHandler = any_cast<RequestHandler *>(m_externalObject);

    // fill environment
    {
        auto const &renv = environment();
        auto renvp = [&](const string &k) {
            return renv.parameters.count(k) ? renv.parameters.at(k) : string();
        };
        requestInit.environment = {
                {"content-type", renvp("CONTENT_TYPE")},
        };

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            std::stringstream baseUrl;
            auto https = renv.parameters.count("HTTPS");
            baseUrl << (https ? "https://" : "http://")
                    << renvp("HTTP_HOST");
            auto baseUrlStr = baseUrl.str();
            requestInit.environment["BASE_URL"] = baseUrlStr;
            auto requestUri = renvp("REQUEST_URI");

            // fullUrlWithQS is the full URL, e.g., https://www.example.com/test?a=b&c=d
            requestInit.environment["FULL_URL_WITH_QS"] = baseUrlStr + requestUri;

            // fullUrlWithoutQS is the full URL without query string, e.g., https://www.example.com/test
            baseUrl << requestUri.substr(0, requestUri.find_first_of('?'));
            requestInit.environment["FULL_URL_WITHOUT_QS"] = baseUrl.str();
        }

        for (const auto&[k, v]: renv.parameters) {
            string envKey;
            // parameters starting with HTTP_ are usually HTTP headers
            // convert them so they should match their original name (lowercase), e.g., HTTP_USER_AGENT => user-agent
            if (k.substr(0, 5) == "HTTP_") {
                envKey = string_replace(to_lowercase(k.substr(5)), {{'_', '-'}});
            } else {
                // other FastCGI parameters are used as they are
                envKey = k;
            }
            if (requestInit.environment.count(envKey) == 0) {
                requestInit.environment[envKey] = v;
            }
        }

        // set acceptLanguages
        // TODO remove in v0.8
        requestInit.acceptLanguages = renv.acceptLanguages;

        // GET, POST, COOKIE vars, raw POST, POST content type
        requestInit.getVars = to_unordered_multimap(renv.gets);
        requestInit.postVars = to_unordered_multimap(renv.posts);
        requestInit.cookieVars = to_unordered_multimap(renv.cookies);
        requestInit.rawPost = rawPost;
        requestInit.postContentType = renv.contentType;

        // POST files
        // TODO use MimeMultipart
        for (auto const &[k, fcgiFile]: renv.files) {
            File f;
            f.filename = fcgiFile.filename;
            f.size = fcgiFile.size;
            f.contentType = fcgiFile.contentType;
            f.dataPtr = fcgiFile.data;
            requestInit.postFiles.insert({k, f});
        }

    }

    ConnectionInitContainer connectionInit;
    connectionInit.requestInit = move(requestInit);
    connectionInit.config = *requestHandler->getConfig();

    connectionInit.flushCallback = [this](FlushCallbackContainer flushInfo) {
        string response;
        if (!flushInfo.flushedBefore) {
            response = "status: " + flushInfo.getStatusString() + "\r\n";
        }
        response += flushInfo.getFullHttp();
        dump(response.c_str(), response.size());
    };

    Connection connection(connectionInit);
    requestHandler->handleRequest(connection);
    connection.flushResponse();

    return true;
}

bool FastcgippRequestAdapter::inProcessor() {
    auto requestHandler = any_cast<RequestHandler *>(m_externalObject);
    auto postContentType = environment().contentType;
    auto configPtr = requestHandler->getConfig();

    string rawPostAccess = (*configPtr)[{"post", "raw_access"}];
    if (postContentType.empty() || rawPostAccess == "never" || (rawPostAccess != "always" &&
                                                                (postContentType == "multipart/form-data" ||
                                                                 postContentType ==
                                                                 "application/x-www-form-urlencoded"))) {
        return false;
    }

    auto postBuffer = environment().postBuffer();
    rawPost = make_shared<string>(postBuffer.data(), postBuffer.size());
    return false;
}

struct nawa::FastcgiRequestHandler::FastcgippManagerAdapter {
    std::unique_ptr<Fastcgipp::Manager<FastcgippRequestAdapter>> manager;
};

nawa::FastcgiRequestHandler::FastcgiRequestHandler(nawa::HandleRequestFunction handleRequestFunction,
                                                   nawa::Config config, int concurrency) {
    setAppRequestHandler(move(handleRequestFunction));
    setConfig(move(config));
    auto configPtr = getConfig();

    size_t postMax = 0;
    try {
        postMax = configPtr->isSet({"post", "max_size"})
                  ? static_cast<size_t>(std::stoul((*configPtr)[{"post", "max_size"}])) * 1024 : 0;
    }
    catch (std::invalid_argument &e) {
        NLOG_WARNING(logger, "WARNING: Invalid value given for post/max_size given in the config file.")
    }

    // set up fastcgilite logging
    Fastcgipp::Logging::addHeader = false;
    Fastcgipp::Logging::logFunction = [&](const std::string &msg, Fastcgipp::Logging::Level level) {
        Log::Level nawaLevel;
        switch (level) {
            case Fastcgipp::Logging::INFO:
                nawaLevel = Log::Level::INFORMATIONAL;
                break;
            case Fastcgipp::Logging::FAIL:
            case Fastcgipp::Logging::ERROR:
                nawaLevel = Log::Level::ERROR;
                break;
            case Fastcgipp::Logging::WARNING:
                nawaLevel = Log::Level::WARNING;
                break;
            case Fastcgipp::Logging::DEBUG:
            case Fastcgipp::Logging::DIAG:
                nawaLevel = Log::Level::DEBUG;
                break;
            default:
                nawaLevel = Log::Level::INFORMATIONAL;
        }
        logger.write(msg, nawaLevel);
    };

    fastcgippManager = std::make_unique<FastcgippManagerAdapter>();
    // TODO it should be possible to change postMax after fastcgilite manager creation
    fastcgippManager->manager = std::make_unique<Fastcgipp::Manager<FastcgippRequestAdapter>>(concurrency, postMax,
                                                                                              static_cast<RequestHandler *>(this));

    // socket handling
    string mode = (*configPtr)[{"fastcgi", "mode"}];
    if (mode == "tcp") {
        auto fastcgiListen = (*configPtr)[{"fastcgi", "listen"}];
        auto fastcgiPort = (*configPtr)[{"fastcgi", "port"}];
        if (fastcgiListen.empty()) fastcgiListen = "127.0.0.1";
        const char *fastcgiListenC = fastcgiListen.c_str();
        if (fastcgiListen == "all") fastcgiListenC = nullptr;
        if (fastcgiPort.empty()) fastcgiPort = "8000";
        if (!fastcgippManager->manager->listen(fastcgiListenC, fastcgiPort.c_str())) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            "Could not create TCP socket for FastCGI.");
        }
    } else if (mode == "unix") {
        uint32_t permissions = 0xffffffffUL;
        string permStr = (*configPtr)[{"fastcgi", "permissions"}];
        // convert the value from the config file
        if (!permStr.empty()) {
            const char *psptr = permStr.c_str();
            char *endptr;
            long perm = strtol(psptr, &endptr, 8);
            if (*endptr == '\0') {
                permissions = (uint32_t) perm;
            }
        }

        auto fastcgiSocketPath = (*configPtr)[{"fastcgi", "path"}];
        if (fastcgiSocketPath.empty()) {
            fastcgiSocketPath = "/etc/nawarun/sock.d/nawarun.sock";
        }
        auto fastcgiOwner = (*configPtr)[{"fastcgi", "owner"}];
        auto fastcgiGroup = (*configPtr)[{"fastcgi", "group"}];

        if (!fastcgippManager->manager->listen(fastcgiSocketPath.c_str(), permissions,
                                               fastcgiOwner.empty() ? nullptr : fastcgiOwner.c_str(),
                                               fastcgiGroup.empty() ? nullptr : fastcgiGroup.c_str())) {
            throw Exception(__PRETTY_FUNCTION__, 2,
                            "Could not create UNIX socket for FastCGI.");

        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 3,
                        "Unknown FastCGI socket mode in config.ini.");
    }

    // tell fastcgi to use SO_REUSEADDR if enabled in config
    if ((*configPtr)[{"fastcgi", "reuseaddr"}] != "off") {
        fastcgippManager->manager->reuseAddress(true);
    }
}

FastcgiRequestHandler::~FastcgiRequestHandler() = default;

void FastcgiRequestHandler::start() {
    if (fastcgippManager && fastcgippManager->manager) {
        try {
            fastcgippManager->manager->start();
        } catch (...) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            "An unknown error occurred during start of request handling.");
        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 2, "FastCGI manager is not available.");
    }
}

void FastcgiRequestHandler::stop() noexcept {
    if (fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->stop();
    }
}

void FastcgiRequestHandler::terminate() noexcept {
    if (fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->terminate();
    }
}

void FastcgiRequestHandler::join() noexcept {
    if (fastcgippManager && fastcgippManager->manager) {
        fastcgippManager->manager->join();
    }
}
