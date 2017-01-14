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

#include <services/network.h>

using namespace icecream::services;

namespace
{
    /// Exactly the same code than scheduler/scheduler.cpp.
    int open_tcp_listener(short port)
    {
        int fd;
        struct sockaddr_in myaddr;

        if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            log_perror("socket()");
            return -1;
        }

        int optval = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            log_perror("setsockopt()");
            return -1;
        }

        /* Although we select() on fd we need O_NONBLOCK, due to
           possible network errors making accept() block although select() said
           there was some activity.  */
        if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
            log_perror("fcntl()");
            return -1;
        }

        myaddr.sin_family = AF_INET;
        myaddr.sin_port = htons(port);
        myaddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
            log_perror("bind()");
            return -1;
        }

        if (listen(fd, 10) < 0) {
            log_perror("listen()");
            return -1;
        }

        return fd;
    }
} // anonymous

/// \test Check with empty hostname.
TEST(NetworkTest, EmptyHostname)
{
    struct sockaddr_in remote_addr;

    ASSERT_LT(prepare_connect("", 8765, remote_addr), 0);
}

/// \test Check connection with localhost.
TEST(NetworkTest, LocalHostname)
{
    struct sockaddr_in remote_addr;
    auto fd1 = prepare_connect("localhost", 8765, remote_addr);
    auto fd2 = prepare_connect("127.0.0.1", 8765, remote_addr);

    EXPECT_GE(fd1, 0);
    EXPECT_GE(fd2, 0);

    close(fd1);
    close(fd2);
}

/// \test Check asynchronous connection with localhost.
TEST(NetworkTest, AsyncConnection)
{
    struct sockaddr_in remote_addr;
    auto lfd = open_tcp_listener(8765);
    auto fd = prepare_connect("127.0.0.1", 8765, remote_addr);
    auto timeout = 2;

    ASSERT_GE(fd, 0);
    ASSERT_GE(lfd, 0);
    EXPECT_TRUE(connect_async(fd,
                              reinterpret_cast<struct sockaddr*>(&remote_addr),
                              sizeof(remote_addr),
                              timeout));
    close(fd);
    close(lfd);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
