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
 ** \file tests/test-md5.cpp
 ** \brief Checking md5 algorithm in libmisc.
 */

#include <sstream>

#include <gtest/gtest.h>

#include <misc/md5.h>

/**
 ** \test Copy of original tests in md5.c provided by Peter Deutsch.
 ** Avoid another compilation with -DTEST.
 ** \note There is a mistake for value:
 ** 945399884.61923487334tuvga
 ** On original file, in comment, developer provides expected result:
 ** 0cc175b9c0f1b6a831c399e269772661
 ** But correct hash is:
 ** 4d52535c7692376e7e7e205940a93ae9
 */
TEST(MD5Algorithm, MD5True)
{
    const std::map<const char * const, const char * const> test =
    {
        { "", "d41d8cd98f00b204e9800998ecf8427e" },
        { "945399884.61923487334tuvga", "4d52535c7692376e7e7e205940a93ae9" },
        { "abc", "900150983cd24fb0d6963f7d28e17f72" },
        { "message digest",  "f96b697d7cb7938d525a2f31aaf161d0" },
        { "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
          "d174ab98d277d9f5a5611c2c9f419d9f" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
          "57edf4a22be3c955ac49da2e2107b67a" }
    };

    for (const auto &e : test)
    {
        std::stringstream ss;
        md5_state_t state;
        md5_byte_t digest[16];

        md5_init(&state);
        md5_append(&state,
                   reinterpret_cast<const md5_byte_t *>(e.first),
                   strlen(e.first));
        md5_finish(&state, digest);
        ss << std::hex;
        for (auto di = 0; di < 16; ++di)
        {
            ss << std::setfill('0')
               << std::setw(2)
               << static_cast<unsigned int>(digest[di]);
        }
        EXPECT_EQ(std::string{e.second}, ss.str());
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
