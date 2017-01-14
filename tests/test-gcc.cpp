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
 ** \file tests/test-gcc.cpp
 ** \brief Checking gcc heuristic in libmisc.
 */

#include <gtest/gtest.h>

#include <misc/gcc.h>

/// \test Smoke test for min expand heuristic.
TEST(GCC, MIN_EXPAND)
{
    EXPECT_GE(ggc_min_expand_heuristic(256), 30);
    EXPECT_LE(ggc_min_expand_heuristic(8192), 100);
}

/// \test Smoke test for min heapsize heuristic.
TEST(GCC, MIN_HEAPSIZE)
{
    EXPECT_GE(ggc_min_heapsize_heuristic(256), 4 * 1024);
    EXPECT_LE(ggc_min_heapsize_heuristic(8192), 128 * 1024);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
