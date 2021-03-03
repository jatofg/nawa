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

TEST_CASE("nawa::Utils functions", "[utils]") {

    SECTION("Time conversions") {
        time_t currentTime = time(nullptr);
        REQUIRE(read_smtp_time("Thu,  7 Nov 2019 16:29:50 +0100") == 1573140590);
        REQUIRE(read_http_time(make_http_time(currentTime)) == currentTime);
        REQUIRE(read_smtp_time(make_smtp_time(currentTime)) == currentTime);
    }

    SECTION("Path splitting") {
        string t1 = "p1/p2/p3";
        string t2 = "/p1/p2/p3";
        string t3 = "/p1/p2/p3/";
        string t4 = "/p1/p2/p3?test=/xyz";
        string t5 = "/p1/p2/p3/?test=/xyz/";
        auto t1_split = split_path(t1);
        REQUIRE(t1_split == split_path(t2));
        REQUIRE(t1_split == split_path(t3));
        REQUIRE(t1_split == split_path(t4));
        REQUIRE(t1_split == split_path(t5));
    }
}