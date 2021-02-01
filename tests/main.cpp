/**
 * \file main.cpp
 * \brief Main file for unit tests.
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

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <nawa/Config.h>
#include <nawa/Exception.h>

using namespace nawa;
using namespace std;

Config loadConfig() {
    /*
     * Possible paths for test.ini, order of precedence:
     * 1. ./test.ini
     * 2. ../tests/test.ini
     * 3. ./tests/test.ini
     */
    try {
        return Config("test.ini");
    } catch (Exception const &e) {}

    try {
        return Config("../tests/test.ini");
    } catch (Exception const &e) {}

    try {
        return Config("tests/test.ini");
    } catch (Exception const &e) {}

    return Config();
}

/**
 * TODO missing test cases
 *      unit:
 *          - Config
 *          - Email
 *          - MimeMultipart
 *          - Session
 *          - (Log)
 *          - possibly even Connection, Request
 *      integration:
 *          - (post: sending more than max, files, ...)
 *          - fastcgi
 *          - nawarun
 *          - sessions
 *          - gpc
 *          - access filters
 *          - concurrency
 *          - ...
 */