/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Stephan Kulow <coolo@suse.de>
                  2002, 2003 by Martin Pool <mbp@samba.org>

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

#ifndef ICECREAM_NATIVE_ENVIRONMENT_H
# define ICECREAM_NATIVE_ENVIRONMENT_H

# include <string>
# include <map>

# include <time.h>

namespace icecream
{
    namespace daemon
    {
        struct NativeEnvironment
        {
            std::string name; // the hash
            std::map<std::string, time_t> extrafilestimes;
            // Timestamps for compiler binaries, if they have changed since the time
            // the native env was built, it needs to be rebuilt.
            time_t gcc_bin_timestamp;
            time_t gpp_bin_timestamp;
            time_t clang_bin_timestamp;
            int create_env_pipe; // if in progress of creating the environment
        };

    }
}

#endif /* !ICECREAM_NATIVE_ENVIRONMENT_H */
