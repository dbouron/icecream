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
 ** \file tests/test-exitcode.cpp
 ** \brief Checking exit code in libmisc.
 */

#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include <misc/exitcode.h>

/// \test Basic test for WIFEXITED and WEXITSTATUS macros.
TEST(EXITCODE, EXITSTATUS)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid > 0)
    {
      wait(&status);
      EXPECT_EQ(42, shell_exit_status(status));
    }
  else if (pid == 0)
    exit(42);
  else
    FAIL() << "fork() failed";
}

/// \test Basic test for WIFSIGNALED and WTERMSIG macros.
TEST(EXITCODE, TERMSIG)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid > 0)
    {
      wait(&status);
      EXPECT_EQ(SIGTERM, shell_exit_status(status) - 128);
    }
  else if (pid == 0)
    {
      kill(getpid(), SIGTERM);
    }
  else
    FAIL() << "fork() failed";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
