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
 ** \file tests/test-network.cpp
 ** \brief Checking network util function in icecream::services module.
 */

#include <initializer_list>
#include <string>
#include <gtest/gtest.h>

#include "../services/network.h"

using namespace icecream::services;

/// \test Check with empty hostname.
TEST(EmptyHostname, NetworkTest)
{
    struct sockaddr_in remote_addr;

    ASSERT_LT(prepare_connect("", 8765, remote_addr), 0);
}

/// \test Check connection with localhost.
TEST(LocalHostname, NetworkTest)
{
    struct sockaddr_in remote_addr;

    EXPECT_GE(prepare_connect("localhost", 8765, remote_addr), 0);
    ASSERT_GE(prepare_connect("127.0.0.1", 8765, remote_addr), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
