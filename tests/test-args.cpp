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
 ** \file tests/test-args.cpp
 ** \brief Checking icecream::client module.
 ** Basic tests for checking arguments analysis.
 */


#include <list>
#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "client.h"

namespace
{
    std::string test_run(const char * const *argv, bool icerun = false)
    {
        std::list<std::string> extrafiles;
        CompileJob job;
        bool local = analyse_argv(argv, job, icerun, &extrafiles);
        std::stringstream ss;

        ss << "local:" << local;
        ss << " language:" << job.language();
        ss << " compiler:" << job.compilerName();
        ss << " local:" << concat_args(job.localFlags());
        ss << " remote:" << concat_args(job.remoteFlags());
        ss << " rest:" << concat_args(job.restFlags());
        return ss.str();
    }
} // anonymous

/**
 ** \class ArgsTest
 **
 ** \brief Setup testing environement for checking
 ** arguments analysis.
 */
class ArgsTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        ICECC_COLOR_DIAGNOSTICS = getenv("ICECC_COLOR_DIAGNOSTICS");
        setenv("ICECC_COLOR_DIAGNOSTICS", "0", 1);
    }

    void TearDown() override
    {
        if (ICECC_COLOR_DIAGNOSTICS)
            setenv("ICECC_COLOR_DIAGNOSTICS", ICECC_COLOR_DIAGNOSTICS, 1);
        else
            unsetenv("ICECC_COLOR_DIAGNOSTICS");
    }

private:
    const char *ICECC_COLOR_DIAGNOSTICS;
};

/// \test Check with valid arguments.
TEST_F(ArgsTest, ValidArgs)
{
    const char * argv1[] = { "gcc", "-D", "TEST=1", "-c", "main.cpp", "-o", "main.o", nullptr };
    const char * argv2[] = { "gcc", "-DTEST=1", "-c", "main.cpp", "-o", "main.o", nullptr };

    EXPECT_EQ("local:0 language:C++ compiler:gcc local:'-D, TEST=1' remote:'-c' rest:''",
              test_run(argv1));
    EXPECT_EQ("local:0 language:C++ compiler:gcc local:'-DTEST=1' remote:'-c' rest:''",
              test_run(argv2));
}

/// \test Check with ambiguous arguments.
/** \todo I am not sure about the exact result expected.
 ** Originally, this test should succeed, but it fails.
 ** Tested on several OS (OpenSuse 13.1, Debian 8, Fedora 24).
 */
TEST_F(ArgsTest, AmbiguousArgs)
{
    const char * argv[] = { "clang", "-D", "TEST1=1", "-I.", "-c", "make1.cpp", "-o", "make.o", nullptr};

    EXPECT_EQ("local:0 language:C++ compiler:clang local:'-D, TEST1=1, -I.' remote:'-c' rest:''",
              test_run(argv));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
