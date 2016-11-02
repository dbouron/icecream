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

#ifndef ICECREAM_SERVE_H
# define ICECREAM_SERVE_H

# include <string>
# include <cassert>

# include <stdio.h>
# include <stdlib.h>
# include <setjmp.h>
# include <unistd.h>
# include <string.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>

# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
# ifdef HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
# endif /* HAVE_SYS_SIGNAL_H */
# include <sys/param.h>
# include <unistd.h>

# include <sys/time.h>

# ifdef __FreeBSD__
#  include <sys/socket.h>
#  include <sys/uio.h>
# endif

# ifndef O_LARGEFILE
#  define O_LARGEFILE 0
# endif

# ifndef _PATH_TMP
#  define _PATH_TMP "/tmp"
# endif

# include <job.h>
# include <comm.h>

# include "config.h"
# include "environment.h"
# include "exitcode.h"
# include "tempfile.h"
# include "workit.h"
# include "logging.h"
# include "util.h"
# include "file_util.h"
# include "channel.h"
# include "all.h"

namespace icecream
{
    namespace daemon
    {
        extern int nice_level;

        int handle_connection(const std::string &basedir,
                              std::shared_ptr<services::CompileJob> job,
                              std::shared_ptr<services::Channel> client,
                              int & out_fd,
                              unsigned int mem_limit,
                              uid_t user_uid,
                              gid_t user_gid);
    } // daemon
} // icecream

# endif
