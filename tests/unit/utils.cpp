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
 * \file utils.cpp
 * \brief Unit tests for the nawa::Utils functions.
 */

#include "nawa/Exception.h"
#include <catch2/catch.hpp>
#include <nawa/util/utils.h>

using namespace nawa;
using namespace std;

TEST_CASE("nawa::utils functions", "[unit][utils]") {

    SECTION("Time conversions") {
        time_t currentTime = time(nullptr);
        time_t smtpTime1;
        CHECK_NOTHROW(smtpTime1 = utils::readSmtpTime("Thu,  7 Nov 2019 16:29:50 +0100"));
        CHECK(smtpTime1 == 1573140590);
        time_t httpTime1;
        CHECK_NOTHROW(httpTime1 = utils::readHttpTime(utils::makeHttpTime(currentTime)));
        CHECK(httpTime1 == currentTime);
        time_t smtpTime2;
        CHECK_NOTHROW(smtpTime2 = utils::readSmtpTime(utils::makeSmtpTime(currentTime)));
        CHECK(smtpTime2 == currentTime);
        CHECK_THROWS_AS(utils::readSmtpTime("test"), Exception);
        CHECK_THROWS_AS(utils::readHttpTime("test"), Exception);
    }

    SECTION("Path splitting") {
        string t1 = "p1/p2/p3";
        string t2 = "/p1/p2/p3";
        string t3 = "/p1/p2/p3/";
        string t4 = "/p1/p2/p3?test=/xyz";
        string t5 = "/p1/p2/p3/?test=/xyz/";
        auto t1_split = utils::splitPath(t1);
        CHECK(t1_split == utils::splitPath(t2));
        CHECK(t1_split == utils::splitPath(t3));
        CHECK(t1_split == utils::splitPath(t4));
        CHECK(t1_split == utils::splitPath(t5));
    }
}
