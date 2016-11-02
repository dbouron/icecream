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

#ifndef ICECREAM_WORKIT_H
# define ICECREAM_WORKIT_H

# include "config.h"

# include <algorithm>
# include <string>
# include <exception>

# ifdef __FreeBSD__
#  include <sys/param.h>
# endif

# include <stdio.h>
# include <errno.h>

/* According to earlier standards */
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
# include <sys/fcntl.h>
# include <sys/wait.h>
# include <signal.h>
# include <sys/resource.h>
# if HAVE_SYS_USER_H && !defined(__DragonFly__)
#  include <sys/user.h>
# endif
# include <sys/socket.h>

# if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__APPLE__)
#  ifndef RUSAGE_SELF
#   define   RUSAGE_SELF     (0)
#  endif
#  ifndef RUSAGE_CHILDREN
#   define   RUSAGE_CHILDREN     (-1)
#  endif
# endif

# include "comm.h"
# include "platform.h"
# include "util.h"
# include "channel.h"
# include "compile-result.h"
# include "tempfile.h"
# include "assert.h"
# include "exitcode.h"
# include "logging.h"
# include <job.h>
# include <all.h>
# include <protocol.h>

namespace icecream
{
    namespace daemon
    {
        enum JobStats : uint32_t
        {
            in_compressed,
            in_uncompressed,
            out_uncompressed,
            exit_code,
            real_msec,
            user_msec,
            sys_msec,
            sys_pfaults
        };

        /// \todo Rename this exception.
        // No icecream ;(
        class myexception : public std::exception
        {
            int code;
        public:
            myexception(int _exitcode) : exception(), code(_exitcode) {}

            int exitcode() const {
                return code;
            }
        };

        extern int work_it(services::CompileJob &j,
                           unsigned int job_stats[],
                           services::Channel *client,
                           services::CompileResult &msg,
                           const std::string &tmp_root,
                           const std::string &build_path,
                           const std::string &file_name,
                           unsigned long int mem_limit,
                           int client_fd, int job_in_fd);

    } // daemon
} // icecream

#endif
