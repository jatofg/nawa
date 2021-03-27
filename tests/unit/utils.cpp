/**
 * \file utils.cpp
 * \brief Unit tests for the nawa::Utils functions.
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

#include <catch2/catch.hpp>
#include <nawa/util/utils.h>

using namespace nawa;
using namespace std;

TEST_CASE("nawa::utils functions", "[unit][utils]") {

    SECTION("Time conversions") {
        time_t currentTime = time(nullptr);
        CHECK(utils::readSmtpTime("Thu,  7 Nov 2019 16:29:50 +0100") == 1573140590);
        CHECK(utils::readHttpTime(utils::makeHttpTime(currentTime)) == currentTime);
        CHECK(utils::readSmtpTime(utils::makeSmtpTime(currentTime)) == currentTime);
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