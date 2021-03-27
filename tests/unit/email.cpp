/**
 * \file email.cpp
 * \brief Unit tests for email.
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
#include <nawa/mail/EmailAddress.h>

using namespace nawa;
using namespace std;

TEST_CASE("nawa::mail::EmailAddress class", "[unit][email][emailaddress]") {
    SECTION("Validity check") {
        CHECK_FALSE(mail::EmailAddress("hello world@example.com").isValid());
        CHECK(mail::EmailAddress("test@bärlin.xy").isValid());
    }
    SECTION("Correct representation") {
        mail::EmailAddress ea1("John Doe", "john.doe@example.com");
        mail::EmailAddress ea2("john.doe@example.com");
        mail::EmailAddress ea3("test@bärlin.xy");
        CHECK(ea1.get() == "John Doe <john.doe@example.com>");
        CHECK(ea1.get(true) == "John Doe <john.doe@example.com>");
        CHECK(ea1.get(false) == "<john.doe@example.com>");
        CHECK(ea2.get(false) == "<john.doe@example.com>");
        CHECK(ea2.get(true) == "<john.doe@example.com>");
        CHECK(ea3.get() == "<test@xn--brlin-gra.xy>");
        CHECK(ea3.get(false, false) == "<test@bärlin.xy>");
    }
}
