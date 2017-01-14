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
 ** \file tests/test-platform.cpp
 ** \brief Checking platform detemination in libmisc.
 */

#include <gtest/gtest.h>

#include <misc/platform.h>

/// \test Ensure that no exception will be throw.
TEST(PLATFORM, UNAME)
{
  EXPECT_NO_THROW(determine_platform);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
