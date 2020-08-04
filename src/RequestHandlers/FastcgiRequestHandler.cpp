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
#include <nawa/RequestHandlers/RequestHandler.h>
#include <nawa/RequestHandlers/FastcgiRequestHandler.h>
#include <nawa/Log.h>
#include <nawa/Connection.h>
#include <nawa/Utils.h>
#include <nawa/Exception.h>

using namespace nawa;
using namespace std;

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
}

struct FastcgippArgumentContainer {
    RequestHandler *requestHandler;
    size_t postMax;
    Config &config;
    RawPostAccess rawPostAccess;
};

class FastcgippRequestAdapter : public Fastcgipp::Request<char> {
    string rawPost;
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
    auto argc = any_cast<FastcgippArgumentContainer>(m_externalObject);

    // fill environment
    {
        auto const &renv = environment();
        requestInit.environment = {
                {"host",               renv.host},
                {"userAgent",          renv.userAgent},
                {"acceptContentTypes", renv.acceptContentTypes},
                {"acceptCharsets",     renv.acceptCharsets},
                {"authorization",      renv.authorization},
                {"referer",            renv.referer},
                {"contentType",        renv.contentType},
                {"root",               renv.root},
                {"scriptName",         renv.scriptName},
                {"requestUri",         renv.requestUri},
                {"serverPort",         to_string(renv.serverPort)},
                {"remotePort",         to_string(renv.remotePort)},
                {"ifModifiedSince",    to_string(renv.ifModifiedSince)},
                {"https",              renv.others.count("HTTPS") ? renv.others.at("HTTPS") : ""},
                {"serverName",         renv.others.count("SERVER_NAME") ? renv.others.at("SERVER_NAME") : ""},
                {"serverSoftware",     renv.others.count("SERVER_SOFTWARE") ? renv.others.at("SERVER_SOFTWARE") : ""},
        };

        auto &requestMethod = requestInit.environment["requestMethod"];
        switch (renv.requestMethod) {
            case Fastcgipp::Http::RequestMethod::ERROR:
                requestMethod = "ERROR";
                break;
            case Fastcgipp::Http::RequestMethod::HEAD:
                requestMethod = "HEAD";
                break;
            case Fastcgipp::Http::RequestMethod::GET:
                requestMethod = "GET";
                break;
            case Fastcgipp::Http::RequestMethod::POST:
                requestMethod = "POST";
                break;
            case Fastcgipp::Http::RequestMethod::PUT:
                requestMethod = "PUT";
                break;
            case Fastcgipp::Http::RequestMethod::DELETE:
                requestMethod = "DELETE";
                break;
            case Fastcgipp::Http::RequestMethod::TRACE:
                requestMethod = "TRACE";
                break;
            case Fastcgipp::Http::RequestMethod::OPTIONS:
                requestMethod = "OPTIONS";
                break;
            case Fastcgipp::Http::RequestMethod::CONNECT:
                requestMethod = "CONNECT";
                break;
        }

        {
            stringstream serverAddr, remoteAddr;
            serverAddr << renv.serverAddress;
            remoteAddr << renv.remoteAddress;
            requestInit.environment["serverAddress"] = serverAddr.str();
            requestInit.environment["remoteAddress"] = remoteAddr.str();
        }

        {
            // the base URL is the URL without the request URI, e.g., https://www.example.com
            std::stringstream baseUrl;
            auto https = renv.others.count("HTTPS");
            baseUrl << (https ? "https://" : "http://")
                    << renv.host;
            if ((!https && renv.serverPort != 80) || (https && renv.serverPort != 443)) {
                baseUrl << ":" << renv.serverPort;
            }
            auto baseUrlStr = baseUrl.str();
            requestInit.environment["baseUrl"] = baseUrlStr;

            // fullUrlWithQS is the full URL, e.g., https://www.example.com/test?a=b&c=d
            requestInit.environment["fullUrlWithQS"] = baseUrlStr + renv.requestUri;

            // fullUrlWithoutQS is the full URL without query string, e.g., https://www.example.com/test
            baseUrl << renv.requestUri.substr(0, renv.requestUri.find_first_of('?'));
            requestInit.environment["fullUrlWithoutQS"] = baseUrl.str();
        }

        for (const auto&[k, v]: renv.others) {
            if (requestInit.environment.count(to_lowercase(k)) == 0) {
                requestInit.environment[to_lowercase(k)] = v;
            }
        }

        // set acceptLanguages
        requestInit.acceptLanguages = renv.acceptLanguages;

        // GET, POST, COOKIE
        requestInit.getVars = renv.gets;
        requestInit.postVars = renv.posts;
        requestInit.cookieVars = renv.cookies;
        requestInit.postContentType = renv.contentType;

        // POST files
        for (auto const &[k, fcgiFile]: renv.files) {
            File f;
            f.filename = fcgiFile.filename;
            f.size = fcgiFile.size;
            f.contentType = fcgiFile.contentType;
            f.dataPtr = fcgiFile.data;
            requestInit.postFiles.insert({k, f});
        }
    }

    requestInit.rawPostCallback = [this]() {
        return rawPost;
    };

    ConnectionInitContainer connectionInit;
    connectionInit.requestInit = move(requestInit);
    connectionInit.config = argc.config;

    connectionInit.flushCallback = [this](const string &raw) {
        dump(raw.c_str(), raw.size());
    };

    Connection connection(connectionInit);
    argc.requestHandler->handleRequest(connection);
    connection.flushResponse();

    return true;
}

bool FastcgippRequestAdapter::inProcessor() {
    auto argc = any_cast<FastcgippArgumentContainer>(m_externalObject);
    auto postContentType = environment().contentType;

    if (argc.rawPostAccess == RawPostAccess::NEVER || (argc.rawPostAccess == RawPostAccess::NONSTANDARD &&
                                                       (postContentType == "multipart/form-data" ||
                                                        postContentType == "application/x-www-form-urlencoded"))) {
        return false;
    }

    auto postBuffer = environment().postBuffer();
    rawPost = std::string(postBuffer.data(), postBuffer.size());
    return false;
}

struct nawa::FastcgiRequestHandler::FastcgippManagerAdapter {
    std::unique_ptr<Fastcgipp::Manager<FastcgippRequestAdapter>> manager;
};

nawa::FastcgiRequestHandler::FastcgiRequestHandler(nawa::HandleRequestFunction handleRequestFunction,
                                                   nawa::Config config_, int concurrency) {
    setAppRequestHandler(move(handleRequestFunction));
    setConfig(move(config_));

    std::string rawPostStr = config[{"post", "raw_access"}];
    auto rawPostAccess = (rawPostStr == "never")
                         ? RawPostAccess::NEVER : ((rawPostStr == "always") ? RawPostAccess::ALWAYS
                                                                            : RawPostAccess::NONSTANDARD);

    FastcgippArgumentContainer arg{
            .requestHandler=this,
            .postMax=0,
            .config=config,
            .rawPostAccess=rawPostAccess
    };

    try {
        arg.postMax = config.isSet({"post", "max_size"})
                      ? static_cast<size_t>(std::stoul(config[{"post", "max_size"}])) * 1024 : 0;
    }
    catch (std::invalid_argument &e) {
        LOG("WARNING: Invalid value given for post/max_size given in the config file.");
    }

    fastcgippManager = std::make_unique<FastcgippManagerAdapter>();
    fastcgippManager->manager = std::make_unique<Fastcgipp::Manager<FastcgippRequestAdapter>>(concurrency, arg.postMax,
                                                                                              arg);

    // socket handling
    string mode = config[{"fastcgi", "mode"}];
    if (mode == "tcp") {
        auto fastcgiListen = config[{"fastcgi", "listen"}];
        auto fastcgiPort = config[{"fastcgi", "port"}];
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
        string permStr = config[{"fastcgi", "permissions"}];
        // convert the value from the config file
        if (!permStr.empty()) {
            const char *psptr = permStr.c_str();
            char *endptr;
            long perm = strtol(psptr, &endptr, 8);
            if (*endptr == '\0') {
                permissions = (uint32_t) perm;
            }
        }

        auto fastcgiSocketPath = config[{"fastcgi", "path"}];
        if (fastcgiSocketPath.empty()) {
            fastcgiSocketPath = "/etc/nawarun/sock.d/nawarun.sock";
        }
        auto fastcgiOwner = config[{"fastcgi", "owner"}];
        auto fastcgiGroup = config[{"fastcgi", "group"}];

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
    if (config[{"fastcgi", "reuseaddr"}] != "off") {
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
