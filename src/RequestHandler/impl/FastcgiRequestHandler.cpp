/*
 * Copyright (C) 2019-2022 Tobias Flaig.
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

/**
 * \file FastcgiRequestHandler.cpp
 * \brief Implementation of the FastcgiRequestHandler class.
 */

#include <fastcgi++/log.hpp>
#include <fastcgi++/manager.hpp>
#include <fastcgi++/request.hpp>
#include <nawa/Exception.h>
#include <nawa/RequestHandler/RequestHandler.h>
#include <nawa/RequestHandler/impl/FastcgiRequestHandler.h>
#include <nawa/connection/Connection.h>
#include <nawa/connection/ConnectionInitContainer.h>
#include <nawa/logging/Log.h>
#include <nawa/util/utils.h>

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
}// namespace

bool FastcgippRequestAdapter::response() {
    RequestInitContainer requestInit;
    auto requestHandler = any_cast<RequestHandler*>(m_externalObject);

    // fill environment
    {
        auto const& renv = environment();
        auto renvp = [&](string const& k) {
            return renv.parameters.count(k) ? renv.parameters.at(k) : string();
        };
        requestInit.environment = {
                {"content-type", renvp("CONTENT_TYPE")},
        };

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            stringstream baseUrl;
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

        for (auto const& [k, v] : renv.parameters) {
            string envKey;
            // parameters starting with HTTP_ are usually HTTP headers
            // convert them so they should match their original name (lowercase), e.g., HTTP_USER_AGENT => user-agent
            if (k.substr(0, 5) == "HTTP_") {
                envKey = utils::stringReplace(utils::toLowercase(k.substr(5)), {{'_', '-'}});
            } else {
                // other FastCGI parameters are used as they are
                envKey = k;
            }
            if (requestInit.environment.count(envKey) == 0) {
                requestInit.environment[envKey] = v;
            }
        }

        // GET, POST, COOKIE vars, raw POST, POST content type
        requestInit.getVars = utils::toUnorderedMultimap(renv.gets);
        requestInit.postVars = utils::toUnorderedMultimap(renv.posts);
        requestInit.cookieVars = utils::toUnorderedMultimap(renv.cookies);
        requestInit.rawPost = rawPost;
        requestInit.postContentType = renv.contentType;

        // POST files
        for (auto const& [k, fcgiFile] : renv.files) {
            requestInit.postFiles.insert({k, File(fcgiFile.data, fcgiFile.size).filename(fcgiFile.filename).contentType(fcgiFile.contentType)});
        }
    }

    ConnectionInitContainer connectionInit;
    connectionInit.requestInit = std::move(requestInit);
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
    auto requestHandler = any_cast<RequestHandler*>(m_externalObject);
    auto postContentType = environment().contentType;
    auto configPtr = requestHandler->getConfig();

    string rawPostAccess = (*configPtr)[{"post", "raw_access"}];
    if (postContentType.empty() || rawPostAccess == "never" ||
        (rawPostAccess != "always" && (postContentType == "multipart/form-data" || postContentType == "application/x-www-form-urlencoded"))) {
        return false;
    }

    auto postBuffer = environment().postBuffer();
    rawPost = make_shared<string>(postBuffer.data(), postBuffer.size());
    return false;
}

struct FastcgiRequestHandler::Data {
    unique_ptr<Fastcgipp::Manager<FastcgippRequestAdapter>> fastcgippManager;
    bool requestHandlingActive = false;
    bool joined = false;
};

FastcgiRequestHandler::FastcgiRequestHandler(std::shared_ptr<HandleRequestFunctionWrapper> handleRequestFunction,
                                             Config config, int concurrency) {
    data = make_unique<Data>();

    setAppRequestHandler(std::move(handleRequestFunction));
    setConfig(std::move(config));
    auto configPtr = getConfig();

    size_t postMax = 0;
    try {
        postMax = configPtr->isSet({"post", "max_size"})
                          ? static_cast<size_t>(stoul((*configPtr)[{"post", "max_size"}])) * 1024
                          : 0;
    } catch (invalid_argument& e) {
        NLOG_WARNING(logger, "WARNING: Invalid value given for post/max_size given in the config file.")
    }

    // set up fastcgilite logging
    Fastcgipp::Logging::addHeader = false;
    Fastcgipp::Logging::logFunction = [&](string const& msg, Fastcgipp::Logging::Level level) {
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

    data->fastcgippManager = make_unique<Fastcgipp::Manager<FastcgippRequestAdapter>>(concurrency, postMax,
                                                                                      static_cast<RequestHandler*>(this));

    // socket handling
    string mode = (*configPtr)[{"fastcgi", "mode"}];
    if (mode == "tcp") {
        auto fastcgiListen = (*configPtr)[{"fastcgi", "listen"}];
        auto fastcgiPort = (*configPtr)[{"fastcgi", "port"}];
        if (fastcgiListen.empty())
            fastcgiListen = "127.0.0.1";
        char const* fastcgiListenC = fastcgiListen.c_str();
        if (fastcgiListen == "all")
            fastcgiListenC = nullptr;
        if (fastcgiPort.empty())
            fastcgiPort = "8000";
        if (!data->fastcgippManager->listen(fastcgiListenC, fastcgiPort.c_str())) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            "Could not create TCP socket for FastCGI.");
        }
    } else if (mode == "unix") {
        uint32_t permissions = 0xffffffffUL;
        string permStr = (*configPtr)[{"fastcgi", "permissions"}];
        // convert the value from the config file
        if (!permStr.empty()) {
            char const* psptr = permStr.c_str();
            char* endptr;
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

        if (!data->fastcgippManager->listen(fastcgiSocketPath.c_str(), permissions,
                                            fastcgiOwner.empty() ? nullptr : fastcgiOwner.c_str(),
                                            fastcgiGroup.empty() ? nullptr : fastcgiGroup.c_str())) {
            throw Exception(__PRETTY_FUNCTION__, 2,
                            "Could not create UNIX socket for FastCGI.");
        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 3,
                        "Unknown FastCGI socket mode in configuration.");
    }

    // tell fastcgi to use SO_REUSEADDR if enabled in config
    if ((*configPtr)[{"fastcgi", "reuseaddr"}] != "off") {
        data->fastcgippManager->reuseAddress(true);
    }
}

FastcgiRequestHandler::~FastcgiRequestHandler() {
    if (data->requestHandlingActive && !data->joined) {
        data->fastcgippManager->terminate();
    }
    if (!data->joined) {
        data->fastcgippManager->join();
        data->fastcgippManager.reset(nullptr);
    }
}

void FastcgiRequestHandler::start() {
    if (data->requestHandlingActive) {
        return;
    }
    if (data->joined) {
        throw Exception(__PRETTY_FUNCTION__, 10, "FastcgiRequestHandler was already joined.");
    }
    if (data->fastcgippManager) {
        try {
            data->fastcgippManager->start();
            data->requestHandlingActive = true;
        } catch (...) {
            throw Exception(__PRETTY_FUNCTION__, 1,
                            "An unknown error occurred during start of request handling.");
        }
    } else {
        throw Exception(__PRETTY_FUNCTION__, 2, "FastCGI manager is not available.");
    }
}

void FastcgiRequestHandler::stop() noexcept {
    if (data->joined) {
        return;
    }
    if (data->fastcgippManager) {
        data->fastcgippManager->stop();
    }
}

void FastcgiRequestHandler::terminate() noexcept {
    if (data->joined) {
        return;
    }
    if (data->fastcgippManager) {
        data->fastcgippManager->terminate();
    }
}

void FastcgiRequestHandler::join() noexcept {
    if (data->joined) {
        return;
    }
    if (data->fastcgippManager) {
        data->fastcgippManager->join();
        data->joined = true;
        data->fastcgippManager.reset(nullptr);
    }
}
