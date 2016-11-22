/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of icecc.

    Copyright (C) 2002, 2003 by Martin Pool <mbp@samba.org>
                  2004 Stephan Kulow <coolo@suse.de>

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

#ifndef ICECREAM_LOCAL_H
# define ICECREAM_LOCAL_H

# include <string>

# include "config.h"

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <unistd.h>
# include <limits.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# ifdef HAVE_SIGNAL_H
#  include <signal.h>
# endif

# define CLIENT_DEBUG 0

# include "client/client.h"
# include "client/util.h"
# include "client/safeguard.h"

# include "services/channel.h"
# include "services/job.h"

namespace icecream
{
    namespace client
    {
        /* Name of this program, for trace.c */
        extern const char *rs_program_name;

        int build_local(services::CompileJob &job, services::Channel *daemon, struct rusage *usage = nullptr);
        std::string find_compiler(const services::CompileJob &job);
        bool compiler_is_clang(const services::CompileJob &job);
        bool compiler_only_rewrite_includes(const services::CompileJob &job);
        std::string compiler_path_lookup(const std::string &compiler);

    } // client
} // icecream

#endif /* !ICECREAM_LOCAL_H */
