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

#ifndef ICECREAM_REMOTE_H
# define ICECREAM_REMOTE_H

# include <config.h>

# include <string>
# include <map>
# include <algorithm>
# include <vector>

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/wait.h>
# ifdef __FreeBSD__
// Grmbl  Why is this needed?  We don't use readv/writev
#  include <sys/uio.h>
# endif
# include <fcntl.h>
# include <signal.h>
# include <limits.h>
# include <assert.h>
# include <unistd.h>
# include <stdio.h>
# include <errno.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include <services/comm.h>
# include <client/client.h>
# include <services/tempfile.h>
# include <client/md5.h>
# include <services/util.h>
# include <services/all.h>
# include <client/cpp.h>
# include <services/job.h>
# include <services/channel.h>

# ifndef O_LARGEFILE
#  define O_LARGEFILE 0
# endif

namespace icecream
{
    namespace client
    {
        extern std::string remote_daemon;

        std::string get_absfilename(const std::string &_file);

        /// permill is the probability it will be compiled three times.
        int build_remote(services::CompileJob &job,
                         services::Channel *scheduler,
                         const services::Environments &envs,
                         int permill);

        services::Environments parse_icecc_version(const std::string &target,
                                                   const std::string &prefix);
    } // client
} // icecream

#endif /* !ICECREAM_REMOTE_H */
