/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Stephan Kulow <coolo@suse.de>

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

#ifndef ICECREAM_ENVIRONMENT_H
# define ICECREAM_ENVIRONMENT_H

# include <list>
# include <string>

# include <unistd.h>
# include <errno.h>
# include <dirent.h>
# include <unistd.h>
# include <fcntl.h>
# include <grp.h>
# include <stdio.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
# ifdef HAVE_SIGNAL_H
#  include <signal.h>
# endif

# include <config.h>
# include <comm.h>
# include <logging.h>
# include "exitcode.h"
# include "util.h"
# include "channel.h"
# include "all.h"
# include "protocol.h"

namespace icecream
{
    namespace daemon
    {
        bool cleanup_cache(const std::string &basedir,
                           uid_t user_uid,
                           gid_t user_gid);

        int start_create_env(const std::string &basedir,
                             uid_t user_uid, gid_t user_gid,
                             const std::string &compiler,
                             const std::list<std::string> &extrafiles);

        size_t finish_create_env(int pipe,
                                 const std::string &basedir,
                                 std::string &native_environment);

        services::Environments available_environmnents(const std::string &basename);

        void save_compiler_timestamps(time_t &gcc_bin_timestamp,
                                      time_t &gpp_bin_timestamp,
                                      time_t &clang_bin_timestamp);

        bool compilers_uptodate(time_t gcc_bin_timestamp,
                                time_t gpp_bin_timestamp,
                                time_t clang_bin_timestamp);

        pid_t start_install_environment(const std::string &basename,
                                        const std::string &target,
                                        const std::string &name,
                                        std::shared_ptr<services::Channel> c,
                                        int& pipe_to_child,
                                        services::FileChunk*& fmsg,
                                        uid_t user_uid, gid_t user_gid);

        size_t finalize_install_environment(const std::string &basename,
                                            const std::string &target,
                                            pid_t pid,
                                            uid_t user_uid,
                                            gid_t user_gid);

        size_t remove_environment(const std::string &basedir,
                                  const std::string &env);

        size_t remove_native_environment(const std::string &env);

        void chdir_to_environment(services::Channel *c,
                                  const std::string &dirname,
                                  uid_t user_uid,
                                  gid_t user_gid);

        bool verify_env(services::Channel *c,
                        const std::string &basedir,
                        const std::string &target,
                        const std::string &env,
                        uid_t user_uid,
                        gid_t user_gid);
    } // daemon
} // icecream
#endif
