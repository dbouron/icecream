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
 ** \file tests/test-services.cpp
 ** \brief Checking flag deduction in icecream::services module.
 */

#include <initializer_list>
#include <string>
#include <gtest/gtest.h>

#include "../services/job.h"

using namespace icecream::services;

/**
 ** \class FlagTest
 **
 ** \brief Setup testing environement for checking
 ** \c icecream::service::CompileJob flags.
 */
class FlagTest : public ::testing::Test
{
public:
    void SetUp(const std::initializer_list<std::string> l)
    {
        ArgumentList arguments_list;

        for (const auto &e : l)
            arguments_list.push_back(std::make_pair(e, ArgumentType::Unspecified));
        compile_job_.setFlags(arguments_list);
    }

protected:
    CompileJob compile_job_;
};

/// \test Check with valid flags.
TEST_F(FlagTest, ValidFlag)
{
    SetUp({"-g", "-O2"});
    ASSERT_EQ(Flag::g | Flag::O2, compile_job_.argumentFlags());
    ASSERT_NE(Flag::O2, compile_job_.argumentFlags());
    ASSERT_NE(Flag::g, compile_job_.argumentFlags());
}

/// \test Check with valid flags in reverse order.
TEST_F(FlagTest, ReverseValidFlag)
{
    SetUp({"-O2", "-g"});
    ASSERT_EQ(Flag::g | Flag::O2, compile_job_.argumentFlags());
    ASSERT_NE(Flag::O2, compile_job_.argumentFlags());
    ASSERT_NE(Flag::g, compile_job_.argumentFlags());
}

/// \test Check with multiple valid flags, looking for some flag overwritting.
TEST_F(FlagTest, MultiValidFlag)
{
    SetUp({"-O1", "-g", "-O3", "-g3"});
    ASSERT_EQ(Flag::Ol2 | Flag::g3, compile_job_.argumentFlags());
    ASSERT_NE(Flag::O2 | Flag::g | Flag::Ol2 | Flag::g3,
              compile_job_.argumentFlags());
}

/// \test Monkey test with unhandled flags.
TEST_F(FlagTest, MonkeyTestFlag)
{
    SetUp({"-", "----", "test.cpp", "-o", "test"});
    ASSERT_EQ(Flag::None, compile_job_.argumentFlags());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
