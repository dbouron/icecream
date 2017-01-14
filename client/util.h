/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
 * distcc -- A simple distributed compiler system
 *
 * Copyright (C) 2002, 2003 by Martin Pool <mbp@samba.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <config.h>
#include <client/client.h>
#include <client/local.h>
#include <misc/exitcode.h>
#include <services/job.h>
#include <misc/logging.h>

using namespace icecream::services;

namespace icecream
{
    namespace client
    {
        /* util.c */
        int set_cloexec_flag(int desc, int value);
        int dcc_ignore_sigpipe(int val);

        std::string find_basename(const std::string &sfile);
        std::string find_prefix(const std::string &basename);
        void colorify_output(const std::string &s_ccout);
        bool colorify_wanted(const CompileJob &job);
        bool compiler_has_color_output(const CompileJob &job);
        bool output_needs_workaround(const CompileJob &job);
        bool ignore_unverified();
        int resolve_link(const std::string &file, std::string &resolved);

        bool dcc_unlock(int lock_fd);
        bool dcc_lock_host(int &lock_fd);
    } // client
} // icecream
