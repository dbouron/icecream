/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/*
    This file is part of Icecream.

    Copyright (c) 2016 Dimitri Bouron <bouron.d@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/**
 ** \file tests/test-ignore-result.cpp
 ** \brief Checking ignore_result util function in libmisc.
 */

#include <functional>

#include <gtest/gtest.h>

#include <misc/ignore-result.h>

/**
 ** \test Just check that ignore_result() returns correct result.
 ** No side effects on returned value expected.
 */
TEST(CorrectResult, IgnoreResult)
{
    auto f = [] (auto e) { return e; };

    EXPECT_EQ(f(42), misc::ignore_result(f(42)));
    EXPECT_EQ(f(51.0f), misc::ignore_result(f(51.0f)));
    EXPECT_EQ(f("test"), misc::ignore_result(f("test")));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
