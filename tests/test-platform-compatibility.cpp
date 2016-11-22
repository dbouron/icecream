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
 ** \file tests/test-compilerserver.cpp
 ** \brief Checking icecream::scheduler \c CompileServer class.
 */

#include <gtest/gtest.h>

#include <misc/platform_compatibility.h>

/// \test Check with compatible platform.
TEST(ValidCompatibility, PlatformsCompatibility)
{
    EXPECT_TRUE(misc::is_platform_compatible("x86_64", "x86_64"));
    EXPECT_TRUE(misc::is_platform_compatible("x86_64", "i386"));
    EXPECT_TRUE(misc::is_platform_compatible("i586", "i486"));
    EXPECT_TRUE(misc::is_platform_compatible("ppc64", "ppc"));
    EXPECT_TRUE(misc::is_platform_compatible("s390x", "s390"));
}

/// \test Check with no compatible platform.
TEST(NoCompatibility, PlatformsCompatibility)
{
    EXPECT_FALSE(misc::is_platform_compatible("s390", "s390x"));
    EXPECT_FALSE(misc::is_platform_compatible("i486", "i586"));
    EXPECT_FALSE(misc::is_platform_compatible("x86_64", "ppc64"));
    EXPECT_FALSE(misc::is_platform_compatible("x86_64", "noarch"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
