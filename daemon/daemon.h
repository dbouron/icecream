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

#ifndef ICECREAM_DAEMON_H
# define ICECREAM_DAEMON_H

# include <config.h>

# include <memory>
# include <string>
# include <map>
# include <list>
# include <algorithm>

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <fcntl.h>
# include <errno.h>
# include <netdb.h>
# include <getopt.h>
# ifdef HAVE_SIGNAL_H
#  include <signal.h>
# endif
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/un.h>
# include <sys/param.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <pwd.h>

# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/utsname.h>

# ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
# endif

# ifdef HAVE_SYS_VFS_H
#  include <sys/vfs.h>
# endif

# include <arpa/inet.h>

# ifdef HAVE_RESOLV_H
#  include <resolv.h>
# endif
# include <netdb.h>

# ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
# endif

# ifndef RUSAGE_SELF
#  define RUSAGE_SELF (0)
# endif
# ifndef RUSAGE_CHILDREN
#  define RUSAGE_CHILDREN (-1)
# endif

# ifdef HAVE_LIBCAP_NG
#  include <cap-ng.h>
# endif

# include "native-environment.h"
# include "all.h"
# include "discover-sched.h"
# include "job.h"
# include "client.h"
# include "workit.h"
# include "environment.h"
# include "serve.h"
# include "load.h"

#ifndef __attribute_warn_unused_result__
# define __attribute_warn_unused_result__
#endif

namespace icecream
{
    namespace daemon
    {
        extern volatile sig_atomic_t exit_main_loop;
        extern int mem_limit;
        extern unsigned int max_kids;
        extern size_t cache_size_limit;

        class Daemon
        {
        public:
            Daemon();
            ~Daemon();

            bool reannounce_environments() __attribute_warn_unused_result__;
            int answer_client_requests();
            bool handle_transfer_env(Client *client, Msg *msg) __attribute_warn_unused_result__;
            bool handle_transfer_env_done(Client *client);
            bool handle_get_native_env(Client *client, GetNativeEnv *msg) __attribute_warn_unused_result__;
            bool finish_get_native_env(Client *client, std::string env_key);
            void handle_old_request();
            bool handle_compile_file(Client *client, Msg *msg) __attribute_warn_unused_result__;
            bool handle_activity(Client *client) __attribute_warn_unused_result__;
            bool handle_file_chunk_env(Client *client, Msg *msg) __attribute_warn_unused_result__;
            void handle_end(Client *client, int exitcode);
            int scheduler_get_internals() __attribute_warn_unused_result__;
            void clear_children();
            int scheduler_use_cs(UseCS *msg) __attribute_warn_unused_result__;
            bool handle_get_cs(Client *client, Msg *msg) __attribute_warn_unused_result__;
            bool handle_local_job(Client *client, Msg *msg) __attribute_warn_unused_result__;
            bool handle_job_done(Client *cl, JobDone *m) __attribute_warn_unused_result__;
            bool handle_compile_done(Client *client) __attribute_warn_unused_result__;
            bool handle_verify_env(Client *client, VerifyEnv *msg) __attribute_warn_unused_result__;
            bool handle_blacklist_host_env(Client *client, Msg *msg) __attribute_warn_unused_result__;
            int handle_cs_conf(ConfCS *msg);
            std::string dump_internals() const;
            std::string determine_nodename();
            void determine_system();
            bool maybe_stats(bool force = false);
            bool send_scheduler(const Msg &msg) __attribute_warn_unused_result__;
            void close_scheduler();
            bool reconnect();
            int working_loop();
            bool setup_listen_fds();
            void check_cache_size(const std::string &new_env);
            bool create_env_finished(std::string env_key);

            Clients clients;
            std::map<std::string, time_t> envs_last_use;
            // Map of native environments, the basic one(s) containing just the compiler
            // and possibly more containing additional files (such as compiler plugins).
            // The key is the compiler name and a concatenated list of the additional files
            // (or just the compiler name for the basic ones).
            std::map<std::string, NativeEnvironment> native_environments;
            std::string envbasedir;
            uid_t user_uid;
            gid_t user_gid;
            int warn_icecc_user_errno;
            int tcp_listen_fd;
            int unix_listen_fd;
            std::string machine_name;
            std::string nodename;
            bool noremote;
            bool custom_nodename;
            size_t cache_size;
            std::map<int, Channel *> fd2chan;
            int new_client_id;
            std::string remote_name;
            time_t next_scheduler_connect;
            unsigned long icecream_load;
            struct timeval icecream_usage;
            int current_load;
            int num_cpus;
            std::shared_ptr<Channel> scheduler;
            DiscoverSched *discover;
            std::string netname;
            std::string schedname;
            int scheduler_port;
            int daemon_port;

            int max_scheduler_pong;
            int max_scheduler_ping;
            unsigned int current_kids;
        };
    } // daemon
} // icecream

#endif /* !ICECREAM_DAEMON_H */
