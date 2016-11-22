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

#include <daemon/daemon.h>

namespace icecream
{
    namespace daemon
    {
        volatile sig_atomic_t exit_main_loop = 0;
        int mem_limit = 100;
        unsigned int max_kids = 0;
        size_t cache_size_limit = 100 * 1024 * 1024;
        struct timeval last_stat;

        Daemon::Daemon()
            : envbasedir("/tmp/icecc-envs")
            , warn_icecc_user_errno(0)
            , tcp_listen_fd(-1)
            , unix_listen_fd(-1)
            , noremote(false)
            , custom_nodename(false)
            , cache_size(0)
            , new_client_id(0)
            , next_scheduler_connect(0)
            , icecream_load(0)
            , current_load(-1000)
            , num_cpus(0)
            , scheduler(nullptr)
            , discover(nullptr)
            , scheduler_port(8765)
            , daemon_port(10245)
            , max_scheduler_pong(services::MAX_SCHEDULER_PONG)
            , max_scheduler_ping(services::MAX_SCHEDULER_PING)
            , current_kids(0)

        {
            if (getuid() == 0)
            {
                struct passwd *pw = getpwnam("icecc");

                if (pw)
                {
                    user_uid = pw->pw_uid;
                    user_gid = pw->pw_gid;
                }
                else
                {
                    /// apparently errno can be 0 on error here
                    warn_icecc_user_errno = errno ? errno : ENOENT;
                    user_uid = 65534;
                    user_gid = 65533;
                }
            }
            else
            {
                user_uid = getuid();
                user_gid = getgid();
            }
            icecream_usage.tv_sec = 0;
            icecream_usage.tv_usec = 0;
        }

        Daemon::~Daemon()
        {
        }

        bool Daemon::setup_listen_fds()
        {
            tcp_listen_fd = -1;

            /// if we only listen to local clients, there is no point in going TCP
            if (!noremote)
            {
                if ((tcp_listen_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
                {
                    log_perror("socket()");
                    return false;
                }

                int optval = 1;

                if (setsockopt(tcp_listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
                {
                    log_perror("setsockopt()");
                    return false;
                }

                int count = 5;

                while (count)
                {
                    struct sockaddr_in myaddr;
                    myaddr.sin_family = AF_INET;
                    myaddr.sin_port = htons(daemon_port);
                    myaddr.sin_addr.s_addr = INADDR_ANY;

                    if (bind(tcp_listen_fd,
                             reinterpret_cast<struct sockaddr *>(&myaddr),
                             sizeof (myaddr)) < 0)
                    {
                        log_perror("bind()");
                        sleep(2);

                        if (!--count) {
                            return false;
                        }

                        continue;
                    } else {
                        break;
                    }
                }

                if (listen(tcp_listen_fd, 20) < 0)
                {
                    log_perror("listen()");
                    return false;
                }

                fcntl(tcp_listen_fd, F_SETFD, FD_CLOEXEC);
            }

            if ((unix_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            {
                log_perror("socket()");
                return false;
            }

            struct sockaddr_un myaddr;

            memset(&myaddr, 0, sizeof (myaddr));

            myaddr.sun_family = AF_UNIX;

            mode_t old_umask = -1U;

            if (getenv("ICECC_TEST_SOCKET") == nullptr)
            {
#ifdef HAVE_LIBCAP_NG
                // We run as system daemon (UID has been already changed).
                if (capng_have_capability( CAPNG_EFFECTIVE, CAP_SYS_CHROOT ))
                {
#else
                    if (getuid() == 0)
                    {
#endif
                        strncpy(myaddr.sun_path, "/var/run/icecc/iceccd.socket", sizeof(myaddr.sun_path) - 1);
                        unlink(myaddr.sun_path);
                        old_umask = umask(0);
                    }
                    else
                    {
                        // Started by user.
                        if (getenv("HOME"))
                        {
                            strncpy(myaddr.sun_path, getenv("HOME"),
                                    sizeof (myaddr.sun_path) - 1);
                            strncat(myaddr.sun_path, "/.iceccd.socket",
                                    sizeof (myaddr.sun_path) - 1 - strlen(myaddr.sun_path));
                            unlink(myaddr.sun_path);
                        }
                        else
                        {
                            log_error() << "launched by user, but $HOME not set" << std::endl;
                            return false;
                        }
                    }
                }
                else
                {
                    strncpy(myaddr.sun_path, getenv("ICECC_TEST_SOCKET"),
                            sizeof (myaddr.sun_path) - 1);
                    unlink(myaddr.sun_path);
                }

                if (bind(unix_listen_fd,
                         reinterpret_cast<struct sockaddr*>(&myaddr),
                         sizeof (myaddr)) < 0)
                {
                    log_perror("bind()");

                    if (old_umask != -1U)
                    {
                        umask(old_umask);
                    }

                    return false;
                }

                if (old_umask != -1U)
                {
                    umask(old_umask);
                }

                if (listen(unix_listen_fd, 20) < 0)
                {
                    log_perror("listen()");
                    return false;
                }

                fcntl(unix_listen_fd, F_SETFD, FD_CLOEXEC);

                return true;
            }

            void Daemon::determine_system()
            {
                struct utsname uname_buf;

                if (uname(&uname_buf))
                {
                    log_perror("uname call failed");
                    return;
                }

                if (nodename.length() && (nodename != uname_buf.nodename))
                {
                    custom_nodename  = true;
                }

                if (!custom_nodename)
                {
                    nodename = uname_buf.nodename;
                }

                machine_name = determine_platform();
            }

            std::string Daemon::determine_nodename()
            {
                if (custom_nodename && !nodename.empty())
                {
                    return nodename;
                }

                // perhaps our host name changed due to network change?
                struct utsname uname_buf;

                if (!uname(&uname_buf))
                {
                    nodename = uname_buf.nodename;
                }

                return nodename;
            }

            bool Daemon::send_scheduler(const services::Msg& msg)
            {
                if (!scheduler)
                {
                    log_error() << "scheduler dead ?!" << std::endl;
                    return false;
                }

                if (!scheduler->send_msg(msg))
                {
                    log_error() << "sending to scheduler failed.." << std::endl;
                    close_scheduler();
                    return false;
                }

                return true;
            }

            bool Daemon::reannounce_environments()
            {
                log_error() << "reannounce_environments " << std::endl;
                Login lmsg(0, nodename, "");
                lmsg.envs = available_environmnents(envbasedir);
                return send_scheduler(lmsg);
            }

            void Daemon::close_scheduler()
            {
                if (!scheduler)
                {
                    return;
                }

                /// \todo Should we really reset it?
                scheduler.reset();
                scheduler = nullptr;
                delete discover;
                discover = nullptr;
                next_scheduler_connect = time(nullptr) + 20 + (rand() & 31);
            }

            bool Daemon::maybe_stats(bool send_ping)
            {
                struct timeval now;
                gettimeofday(&now, nullptr);

                time_t diff_sent = (now.tv_sec - last_stat.tv_sec) * 1000 + (now.tv_usec - last_stat.tv_usec) / 1000;

                if (diff_sent >= max_scheduler_pong * 1000)
                {
                    Stats msg;
                    unsigned int memory_fillgrade;
                    unsigned long idleLoad = 0;
                    unsigned long niceLoad = 0;

                    if (!fill_stats(idleLoad, niceLoad, memory_fillgrade, &msg, clients.active_processes))
                    {
                        return false;
                    }

                    time_t diff_stat = (now.tv_sec - last_stat.tv_sec) * 1000 + (now.tv_usec - last_stat.tv_usec) / 1000;
                    last_stat = now;

                    /* icecream_load contains time in milliseconds we have used for icecream */
                    /* idle time could have been used for icecream, so claim it */
                    icecream_load += idleLoad * diff_stat / 1000;

                    /* add the time of our childrens, but only the time since the last run */
                    struct rusage ru;

                    if (!getrusage(RUSAGE_CHILDREN, &ru))
                    {
                        uint32_t ice_msec = ((ru.ru_utime.tv_sec - icecream_usage.tv_sec) * 1000
                                             + (ru.ru_utime.tv_usec - icecream_usage.tv_usec) / 1000) / num_cpus;

                        /* heuristics when no child terminated yet: account 25% of total nice as our clients */
                        if (!ice_msec && current_kids)
                        {
                            ice_msec = (niceLoad * diff_stat) / (4 * 1000);
                        }

                        icecream_load += ice_msec * diff_stat / 1000;

                        icecream_usage.tv_sec = ru.ru_utime.tv_sec;
                        icecream_usage.tv_usec = ru.ru_utime.tv_usec;
                    }

                    int idle_average = icecream_load;

                    if (diff_sent)
                    {
                        idle_average = icecream_load * 1000 / diff_sent;
                    }

                    if (idle_average > 1000)
                    {
                        idle_average = 1000;
                    }

                    msg.load = ((700 * (1000 - idle_average)) + (300 * memory_fillgrade)) / 1000;

                    if (memory_fillgrade > 600)
                    {
                        msg.load = 1000;
                    }

                    if (idle_average < 100)
                    {
                        msg.load = 1000;
                    }

#ifdef HAVE_SYS_VFS_H
                    struct statfs buf;
                    int ret = statfs(envbasedir.c_str(), &buf);

                    if (!ret && long(buf.f_bavail) < ((long(max_kids + 1 - current_kids) * 4 * 1024 * 1024) / buf.f_bsize))
                    {
                        msg.load = 1000;
                    }

#endif

                    // Matz got in the urine that not all CPUs are always feed
                    mem_limit = std::max(int(msg.freeMem / std::min(std::max(max_kids, 1U), 4U)), int(100U));

                    if (abs(int(msg.load) - current_load) >= 100 || send_ping)
                    {
                        if (!send_scheduler(msg))
                        {
                            return false;
                        }
                    }

                    icecream_load = 0;
                    current_load = msg.load;
                }

                return true;
            }

            std::string Daemon::dump_internals() const
            {
                std::string result;

                result += "Node Name: " + nodename + "\n";
                result += "  Remote name: " + remote_name + "\n";

                for (const auto &cit : fd2chan)
                {
                    result += "  fd2chan[" + toString(cit.first) + "] = " + cit.second->dump() + "\n";
                }

                for (const auto &cit : clients)
                {
                    result += "  client " + toString(cit.second->client_id) + ": " + cit.second->dump() + "\n";
                }

                if (cache_size)
                {
                    result += "  Cache Size: " + toString(cache_size) + "\n";
                }

                result += "  Architecture: " + machine_name + "\n";

                for (const auto &cit : native_environments)
                {
                    result += "  NativeEnv (" + cit.first + "): "
                        + cit.second.name
                        + (cit.second.create_env_pipe ? " (creating)" : "" )
                        + "\n";
                }

                if (!envs_last_use.empty())
                {
                    result += "  Now: " + toString(time(nullptr)) + "\n";
                }

                for (const auto & cit : envs_last_use)
                {
                    result += "  envs_last_use[" + cit.first  + "] = "
                        + toString(cit.second) + "\n";
                }

                result += "  Current kids: " + toString(current_kids) + " (max: " + toString(max_kids) + ")\n";

                if (scheduler)
                {
                    result += "  Scheduler protocol: " + toString(scheduler->protocol) + "\n";
                }

                Stats msg;
                unsigned int memory_fillgrade = 0;
                unsigned long idleLoad = 0;
                unsigned long niceLoad = 0;

                if (fill_stats(idleLoad, niceLoad, memory_fillgrade, &msg, clients.active_processes))
                {
                    result += "  cpu: " + toString(idleLoad) + " idle, "
                        + toString(niceLoad) + " nice\n";
                    result += "  load: " + toString(msg.loadAvg1 / 1000.) + ", icecream_load: "
                        + toString(icecream_load) + "\n";
                    result += "  memory: " + toString(memory_fillgrade)
                        + " (free: " + toString(msg.freeMem) + ")\n";
                }

                return result;
            }

            int Daemon::scheduler_get_internals()
            {
                trace() << "handle_get_internals " << dump_internals() << std::endl;
                return send_scheduler(StatusText(dump_internals())) ? 0 : 1;
            }

            int Daemon::scheduler_use_cs(services::UseCS *msg)
            {
                Client *c = nullptr;
                std::find_if(std::begin(clients), std::end(clients),
                             [msg, &c](auto &e)
                             {
                                 if (static_cast<uint32_t>(e.second->client_id) == msg->client_id)
                                 {
                                     c = e.second;
                                     return true;
                                 }
                                 return false;
                             });
                trace() << "handle_use_cs " << msg->job_id << " " << msg->client_id
                        << " " << c << " " << msg->hostname << " " << remote_name <<  std::endl;

                if (!c) {
                    if (send_scheduler(JobDone(msg->job_id, 107, JobDone::FROM_SUBMITTER))) {
                        return 1;
                    }

                    return 1;
                }

                if (msg->hostname == remote_name && int(msg->port) == daemon_port) {
                    c->usecsmsg = std::make_shared<UseCS>(msg->host_platform,
                                                          "127.0.0.1",
                                                          daemon_port,
                                                          msg->job_id,
                                                          true, 1,
                                                          msg->matched_job_id);
                    c->status = Status::PENDING_USE_CS;
                } else {
                    c->usecsmsg = std::make_shared<UseCS>(msg->host_platform,
                                                          msg->hostname,
                                                          msg->port,
                                                          msg->job_id,
                                                          true, 1,
                                                          msg->matched_job_id);

                    if (!c->channel->send_msg(*msg)) {
                        handle_end(c, 143);
                        return 0;
                    }

                    c->status = Status::WAITCOMPILE;
                }

                c->job_id = msg->job_id;

                return 0;
            }

            bool Daemon::handle_transfer_env(Client *client, services::Msg *_msg)
            {
                log_error() << "handle_transfer_env" << std::endl;

                assert(client->status != Status::TOINSTALL &&
                       client->status != Status::TOCOMPILE &&
                       client->status != Status::WAITCOMPILE);
                assert(client->pipe_to_child < 0);

                EnvTransfer *emsg = static_cast<EnvTransfer *>(_msg);
                std::string target = emsg->target;

                if (target.empty())
                {
                    target =  machine_name;
                }

                int sock_to_stdin = -1;
                FileChunk *fmsg = nullptr;

                pid_t pid = start_install_environment(envbasedir, target, emsg->name, client->channel,
                                                      sock_to_stdin, fmsg, user_uid, user_gid);

                client->status = Status::TOINSTALL;
                client->outfile = emsg->target + "/" + emsg->name;
                current_kids++;

                if (pid > 0)
                {
                    log_error() << "got pid " << pid << std::endl;
                    client->pipe_to_child = sock_to_stdin;
                    client->child_pid = pid;

                    if (!handle_file_chunk_env(client, fmsg))
                    {
                        pid = 0;
                    }
                }

                if (pid <= 0)
                {
                    handle_transfer_env_done(client);
                }

                delete fmsg;
                return pid > 0;
            }

            bool Daemon::handle_transfer_env_done(Client *client)
            {
                log_error() << "handle_transfer_env_done" << std::endl;

                assert(client->outfile.size());
                assert(client->status == Status::TOINSTALL);

                size_t installed_size = finalize_install_environment(envbasedir, client->outfile,
                                                                     client->child_pid, user_uid, user_gid);

                if (client->pipe_to_child >= 0)
                {
                    installed_size = 0;
                    close(client->pipe_to_child);
                    client->pipe_to_child = -1;
                }

                client->status = Status::UNKNOWN;
                std::string current = client->outfile;
                client->outfile.clear();
                client->child_pid = -1;
                assert(current_kids > 0);
                current_kids--;

                log_error() << "installed_size: " << installed_size << std::endl;

                if (installed_size)
                {
                    cache_size += installed_size;
                    envs_last_use[current] = time(nullptr);
                    log_error() << "installed " << current << " size: " << installed_size
                                << " all: " << cache_size << std::endl;
                }

                check_cache_size(current);

                bool r = reannounce_environments(); // do that before the file compiles

                // we do that here so we're not given out in case of full discs
                if (!maybe_stats(true))
                {
                    r = false;
                }

                return r;
            }

            void Daemon::check_cache_size(const std::string &new_env)
            {
                time_t now = time(nullptr);

                while (cache_size > cache_size_limit)
                {
                    std::string oldest;
                    // I don't dare to use (time_t)-1
                    time_t oldest_time = time(nullptr) + 90000;
                    std::string oldest_native_env_key;

                    for (const auto &cit : envs_last_use)
                    {
                        trace() << "considering cached environment: " << cit.first << " " << cit.second << " " << oldest_time << std::endl;
                        // ignore recently used envs (they might be in use _right_ now)
                        int keep_timeout = 200;
                        std::string native_env_key;

                        // If it is a native environment, allow removing it only after a longer period,
                        // unless there are many native environments.
                        for (const auto &lcit : native_environments)
                        {
                            if (lcit.second.name == cit.first) {
                                native_env_key = lcit.first;

                                if (native_environments.size() < 5) {
                                    keep_timeout = 24 * 60 * 60;    // 1 day
                                }

                                if (lcit.second.create_env_pipe) {
                                    keep_timeout = 365 * 24 * 60 * 60; // do not remove if it's still being created
                                }
                                break;
                            }
                        }

                        if (cit.second < oldest_time && now - cit.second > keep_timeout)
                        {
                            bool env_currently_in_use = false;

                            for (const auto &lcit : clients)
                            {
                                if (lcit.second->status == Status::TOCOMPILE
                                    || lcit.second->status == Status::TOINSTALL
                                    || lcit.second->status == Status::WAITFORCHILD)
                                {
                                    assert(lcit.second->job);
                                    std::string envforjob = lcit.second->job->targetPlatform() + "/"
                                        + lcit.second->job->environmentVersion();

                                    if (envforjob == cit.first)
                                    {
                                        env_currently_in_use = true;
                                    }
                                }
                            }

                            if (!env_currently_in_use)
                            {
                                oldest_time = cit.second;
                                oldest = cit.first;
                                oldest_native_env_key = native_env_key;
                            }
                        }
                    }

                    if (oldest.empty() || oldest == new_env)
                    {
                        break;
                    }

                    size_t removed;

                    if (!oldest_native_env_key.empty()) {
                        removed = remove_native_environment(oldest);
                        native_environments.erase(oldest_native_env_key);
                        trace() << "removing " << oldest << " " << oldest_time << " " << removed << std::endl;
                    } else {
                        removed = remove_environment(envbasedir, oldest);
                        trace() << "removing " << envbasedir << "/" << oldest << " " << oldest_time
                                << " " << removed << std::endl;
                    }

                    cache_size -= std::min(removed, cache_size);
                    envs_last_use.erase(oldest);
                }
            }

            bool Daemon::handle_get_native_env(Client *client, GetNativeEnv *msg)
            {
                std::string env_key;
                std::map<std::string, time_t> extrafilestimes;
                env_key = msg->compiler;

                for (const auto &cit : msg->extrafiles)
                {
                    env_key += ':';
                    env_key += cit;
                    struct stat st;

                    if (stat(cit.c_str(), &st) != 0)
                    {
                        log_error() << "Extra file " << cit << " for environment not found." << std::endl;
                        client->channel->send_msg(End());
                        handle_end(client, 122);
                        return false;
                    }

                    extrafilestimes[cit] = st.st_mtime;
                }

                if (native_environments[env_key].name.length())
                {
                    const NativeEnvironment &env = native_environments[env_key];

                    if (!compilers_uptodate(env.gcc_bin_timestamp, env.gpp_bin_timestamp, env.clang_bin_timestamp)
                        || env.extrafilestimes != extrafilestimes
                        || access(env.name.c_str(), R_OK) != 0)
                    {
                        trace() << "native_env needs rebuild" << std::endl;
                        cache_size -= remove_native_environment(env.name);
                        envs_last_use.erase(env.name);
                        if (env.create_env_pipe)
                        {
                            close(env.create_env_pipe);
                            // TODO kill the still running icecc-create-env process?
                        }
                        native_environments.erase(env_key);   // invalidates 'env'
                    }
                }

                trace() << "get_native_env " << native_environments[env_key].name
                        << " (" << env_key << ")" << std::endl;

                client->status = Status::WAITCREATEENV;
                client->pending_create_env = env_key;

                if (native_environments[env_key].name.length())
                { // already available
                    return finish_get_native_env(client, env_key);
                }
                else
                {
                    NativeEnvironment &env = native_environments[env_key]; // also inserts it
                    if (!env.create_env_pipe) { // start creating it only if not already in progress
                        env.extrafilestimes = extrafilestimes;
                        trace() << "start_create_env " << env_key << std::endl;
                        env.create_env_pipe = start_create_env(envbasedir, user_uid, user_gid, msg->compiler, msg->extrafiles);
                    }
                    else
                    {
                        trace() << "waiting for already running create_env " << env_key << std::endl;
                    }
                }
                return true;
            }

            bool Daemon::finish_get_native_env(Client *client, std::string env_key)
            {
                assert(client->status == Status::WAITCREATEENV);
                assert(client->pending_create_env == env_key);
                UseNativeEnv m(native_environments[env_key].name);

                if (!client->channel->send_msg(m)) {
                    handle_end(client, 138);
                    return false;
                }

                envs_last_use[native_environments[env_key].name] = time(nullptr);
                client->status = Status::GOTNATIVE;
                client->pending_create_env.clear();
                return true;
            }

            bool Daemon::create_env_finished(std::string env_key)
            {
                assert(native_environments.count(env_key));
                NativeEnvironment &env = native_environments[env_key];

                trace() << "create_env_finished " << env_key << std::endl;
                assert(env.create_env_pipe);
                size_t installed_size = finish_create_env(env.create_env_pipe, envbasedir, env.name);
                env.create_env_pipe = 0;

                // we only clean out cache on next target install
                cache_size += installed_size;
                trace() << "cache_size = " << cache_size << std::endl;

                if (!installed_size)
                {
                    for (const auto &cit : clients)
                    {
                        if (cit.second->pending_create_env == env_key)
                        {
                            cit.second->channel->send_msg(End());
                            handle_end(cit.second, 121);
                        }
                    }
                    return false;
                }

                save_compiler_timestamps(env.gcc_bin_timestamp, env.gpp_bin_timestamp, env.clang_bin_timestamp);
                envs_last_use[env.name] = time(nullptr);
                check_cache_size(env.name);

                for (const auto &cit : clients)
                {
                    if (cit.second->pending_create_env == env_key)
                        finish_get_native_env(cit.second, env_key);
                }
                return true;
            }

            bool Daemon::handle_job_done(Client *cl, JobDone *m)
            {
                if (cl->status == Status::CLIENTWORK)
                {
                    clients.active_processes--;
                }

                cl->status = Status::JOBDONE;
                JobDone *msg = static_cast<JobDone *>(m);
                trace() << "handle_job_done " << msg->job_id << " " << msg->exitcode << std::endl;

                if (!m->is_from_server()
                    && (m->user_msec + m->sys_msec) <= m->real_msec)
                {
                    icecream_load += (m->user_msec + m->sys_msec) / num_cpus;
                }

                assert(msg->job_id == cl->job_id);
                cl->job_id = 0; // the scheduler doesn't have it anymore
                return send_scheduler(*msg);
            }

            void Daemon::handle_old_request()
            {
                while ((current_kids + clients.active_processes) < max_kids)
                {

                    Client *client = clients.get_earliest_client(Status::LINKJOB);

                    if (client)
                    {
                        trace() << "send JobLocalBegin to client" << std::endl;

                        if (!client->channel->send_msg(JobLocalBegin()))
                        {
                            log_warning() << "can't send start message to client" << std::endl;
                            handle_end(client, 112);
                        }
                        else
                        {
                            client->status = Status::CLIENTWORK;
                            clients.active_processes++;
                            trace() << "pushed local job " << client->client_id << std::endl;

                            if (!send_scheduler(JobLocalBegin(client->client_id, client->outfile)))
                            {
                                return;
                            }
                        }

                        continue;
                    }

                    client = clients.get_earliest_client(Status::PENDING_USE_CS);

                    if (client)
                    {
                        trace() << "pending " << client->dump() << std::endl;

                        if (client->channel->send_msg(*client->usecsmsg))
                        {
                            client->status = Status::CLIENTWORK;
                            /* we make sure we reserve a spot and the rest is done if the
                             * client contacts as back with a Compile request */
                            clients.active_processes++;
                        }
                        else
                        {
                            handle_end(client, 129);
                        }

                        continue;
                    }

                    /* we don't want to handle TOCOMPILE jobs as long as our load
                       is too high */
                    if (current_load >= 1000)
                    {
                        break;
                    }

                    client = clients.get_earliest_client(Status::TOCOMPILE);

                    if (client)
                    {
                        std::shared_ptr<CompileJob> job = client->job;
                        assert(job);
                        int sock = -1;
                        pid_t pid = -1;

                        trace() << "requests--" << job->jobID() << std::endl;

                        std::string envforjob = job->targetPlatform() + "/" + job->environmentVersion();
                        envs_last_use[envforjob] = time(nullptr);
                        pid = handle_connection(envbasedir, job, client->channel, sock, mem_limit, user_uid, user_gid);
                        trace() << "handle connection returned " << pid << std::endl;

                        if (pid > 0)
                        {
                            current_kids++;
                            client->status = Status::WAITFORCHILD;
                            client->pipe_to_child = sock;
                            client->child_pid = pid;

                            if (!send_scheduler(JobBegin(job->jobID())))
                            {
                                log_info() << "failed sending scheduler about " << job->jobID() << std::endl;
                            }
                        }
                        else
                        {
                            handle_end(client, 117);
                        }

                        continue;
                    }

                    break;
                }
            }

            bool Daemon::handle_compile_done(Client *client)
            {
                assert(client->status == Status::WAITFORCHILD);
                assert(client->child_pid > 0);
                assert(client->pipe_to_child >= 0);

                JobDone *msg = new JobDone(client->job->jobID(), -1, JobDone::FROM_SERVER);
                assert(msg);
                assert(current_kids > 0);
                current_kids--;

                unsigned int job_stat[8];
                int end_status = 151;

                if (read(client->pipe_to_child, job_stat, sizeof(job_stat)) == sizeof(job_stat))
                {
                    msg->in_uncompressed = job_stat[JobStats::in_uncompressed];
                    msg->in_compressed = job_stat[JobStats::in_compressed];
                    msg->out_compressed = msg->out_uncompressed = job_stat[JobStats::out_uncompressed];
                    end_status = msg->exitcode = job_stat[JobStats::exit_code];
                    msg->real_msec = job_stat[JobStats::real_msec];
                    msg->user_msec = job_stat[JobStats::user_msec];
                    msg->sys_msec = job_stat[JobStats::sys_msec];
                    msg->pfaults = job_stat[JobStats::sys_pfaults];
                    end_status = job_stat[JobStats::exit_code];
                }

                close(client->pipe_to_child);
                client->pipe_to_child = -1;
                std::string envforjob = client->job->targetPlatform() + "/" + client->job->environmentVersion();
                envs_last_use[envforjob] = time(nullptr);

                bool r = send_scheduler(*msg);
                handle_end(client, end_status);
                delete msg;
                return r;
            }

            bool Daemon::handle_compile_file(Client *client, services::Msg *msg)
            {
                std::shared_ptr<CompileJob> job = dynamic_cast<CompileFile *>(msg)->takeJob();
                assert(client);
                assert(job);
                client->job = job;

                if (client->status == Status::CLIENTWORK) {
                    assert(job->environmentVersion() == "__client");

                    if (!send_scheduler(JobBegin(job->jobID()))) {
                        trace() << "can't reach scheduler to tell him about compile file job "
                                << job->jobID() << std::endl;
                        return false;
                    }

                    // no scheduler is not an error case!
                } else {
                    client->status = Status::TOCOMPILE;
                }

                return true;
            }

            bool Daemon::handle_verify_env(Client *client, VerifyEnv *msg)
            {
                assert(msg);
                bool ok = verify_env(client->channel.get(), envbasedir, msg->target, msg->environment, user_uid, user_gid);
                trace() << "Verify environment done, " << (ok ? "success" : "failure") << ", environment " << msg->environment
                        << " (" << msg->target << ")" << std::endl;
                VerifyEnvResult resultmsg(ok);

                if (!client->channel->send_msg(resultmsg)) {
                    log_error() << "sending verify end result failed.." << std::endl;
                    return false;
                }

                return true;
            }

            bool Daemon::handle_blacklist_host_env(Client *client, services::Msg *msg)
            {
                // just forward
                assert(dynamic_cast<BlacklistHostEnv *>(msg));
                assert(client);

                if (!scheduler) {
                    return false;
                }

                return send_scheduler(*msg);
            }

            void Daemon::handle_end(Client *client, int exitcode)
            {
#ifdef ICECC_DEBUG
                trace() << "handle_end " << client->dump() << std::endl;
                trace() << dump_internals() << std::endl;
#endif
                fd2chan.erase(client->channel->fd);

                if (client->status == Status::TOINSTALL && client->pipe_to_child >= 0) {
                    close(client->pipe_to_child);
                    client->pipe_to_child = -1;
                    handle_transfer_env_done(client);
                }

                if (client->status == Status::CLIENTWORK) {
                    clients.active_processes--;
                }

                if (client->status == Status::WAITCOMPILE && exitcode == 119) {
                    /* the client sent us a real good bye, so forget about the scheduler */
                    client->job_id = 0;
                }

                /* Delete from the clients map before send_scheduler, which causes a
                   double deletion. */
                if (!clients.erase(client->channel.get())) {
                    log_error() << "client can't be erased: " << client->channel << std::endl;
                    flush_debug();
                    log_error() << dump_internals() << std::endl;
                    flush_debug();
                    assert(false);
                }

                if (scheduler && client->status != Status::WAITFORCHILD) {
                    int job_id = client->job_id;

                    if (client->status == Status::TOCOMPILE) {
                        job_id = client->job->jobID();
                    }

                    if (client->status == Status::WAITFORCS) {
                        job_id = client->client_id; // it's all we have
                        exitcode = CLIENT_WAS_WAITING_FOR_CS; // this is the message
                    }

                    if (job_id > 0) {
                        JobDone::from_type flag = JobDone::FROM_SUBMITTER;

                        switch (client->status) {
                        case Status::TOCOMPILE:
                            flag = JobDone::FROM_SERVER;
                            break;
                        case Status::UNKNOWN:
                        case Status::GOTNATIVE:
                        case Status::JOBDONE:
                        case Status::WAITFORCHILD:
                        case Status::LINKJOB:
                        case Status::TOINSTALL:
                        case Status::WAITCREATEENV:
                            assert(false);   // should not have a job_id
                            break;
                        case Status::WAITCOMPILE:
                        case Status::PENDING_USE_CS:
                        case Status::CLIENTWORK:
                        case Status::WAITFORCS:
                            flag = JobDone::FROM_SUBMITTER;
                            break;
                        }

                        trace() << "scheduler->send_msg( JobDone( " << client->dump() << ", " << exitcode << "))\n";

                        if (!send_scheduler(JobDone(job_id, exitcode, flag))) {
                            trace() << "failed to reach scheduler for remote job done msg!" << std::endl;
                        }
                    } else if (client->status == Status::CLIENTWORK) {
                        // Clientwork && !job_id == LINK
                        trace() << "scheduler->send_msg( JobLocalDone( " << client->client_id << ") );\n";

                        if (!send_scheduler(JobLocalDone(client->client_id))) {
                            trace() << "failed to reach scheduler for local job done msg!" << std::endl;
                        }
                    }
                }

                delete client;
            }

            void Daemon::clear_children()
            {
                while (!clients.empty()) {
                    Client *cl = clients.first();
                    handle_end(cl, 116);
                }

                while (current_kids > 0) {
                    int status;
                    pid_t child;

                    while ((child = waitpid(-1, &status, 0)) < 0 && errno == EINTR) {}

                    current_kids--;
                }

                // they should be all in clients too
                assert(fd2chan.empty());

                fd2chan.clear();
                new_client_id = 0;
                trace() << "cleared children\n";
            }

            bool Daemon::handle_get_cs(Client *client, services::Msg *msg)
            {
                GetCS *umsg = dynamic_cast<GetCS *>(msg);
                assert(client);
                client->status = Status::WAITFORCS;
                umsg->client_id = client->client_id;
                trace() << "handle_get_cs " << umsg->client_id << std::endl;

                if (!scheduler) {
                    /* now the thing is this: if there is no scheduler
                       there is no point in trying to ask him. So we just
                       redefine this as local job */
                    client->usecsmsg = std::make_shared<UseCS>(umsg->target,
                                                               "127.0.0.1",
                                                               daemon_port,
                                                               umsg->client_id,
                                                               true, 1, 0);
                    client->status = Status::PENDING_USE_CS;
                    client->job_id = umsg->client_id;
                    return true;
                }

                return send_scheduler(*umsg);
            }

            int Daemon::handle_cs_conf(ConfCS *msg)
            {
                max_scheduler_pong = msg->max_scheduler_pong;
                max_scheduler_ping = msg->max_scheduler_ping;
                return 0;
            }

            bool Daemon::handle_local_job(Client *client, services::Msg *msg)
            {
                client->status = Status::LINKJOB;
                client->outfile = dynamic_cast<JobLocalBegin *>(msg)->outfile;
                return true;
            }

            bool Daemon::handle_file_chunk_env(Client *client, services::Msg *msg)
            {
                /* this sucks, we can block when we're writing
                   the file chunk to the child, but we can't let the child
                   handle Channel itself due to Channel's stupid
                   caching layer inbetween, which causes us to lose partial
                   data after the MsgType::END msg of the env transfer.  */

                assert(client && client->status == Status::TOINSTALL);

                if (msg->type == MsgType::FILE_CHUNK && client->pipe_to_child >= 0) {
                    FileChunk *fcmsg = static_cast<FileChunk *>(msg);
                    ssize_t len = fcmsg->len;
                    off_t off = 0;

                    while (len) {
                        ssize_t bytes = write(client->pipe_to_child, fcmsg->buffer + off, len);

                        if (bytes < 0 && errno == EINTR) {
                            continue;
                        }

                        if (bytes == -1) {
                            log_perror("write to transfer env pipe failed. ");

                            delete msg;
                            msg = nullptr;
                            handle_end(client, 137);
                            return false;
                        }

                        len -= bytes;
                        off += bytes;
                    }

                    return true;
                }

                if (msg->type == MsgType::END) {
                    close(client->pipe_to_child);
                    client->pipe_to_child = -1;
                    return handle_transfer_env_done(client);
                }

                if (client->pipe_to_child >= 0) {
                    handle_end(client, 138);
                }

                return false;
            }

            bool Daemon::handle_activity(Client *client)
            {
                assert(client->status != Status::TOCOMPILE);

                std::shared_ptr<services::Msg> msg = client->channel->get_msg();

                if (!msg) {
                    handle_end(client, 118);
                    return false;
                }

                bool ret = false;

                if (client->status == Status::TOINSTALL && client->pipe_to_child >= 0) {
                    ret = handle_file_chunk_env(client, msg.get());
                }

                if (ret) {
                    return ret;
                }

                switch (msg->type) {
                case MsgType::GET_NATIVE_ENV:
                    ret = handle_get_native_env(client, dynamic_cast<GetNativeEnv *>(msg.get()));
                    break;
                case MsgType::COMPILE_FILE:
                    ret = handle_compile_file(client, msg.get());
                    break;
                case MsgType::TRANFER_ENV:
                    ret = handle_transfer_env(client, msg.get());
                    break;
                case MsgType::GET_CS:
                    ret = handle_get_cs(client, msg.get());
                    break;
                case MsgType::END:
                    handle_end(client, 119);
                    ret = false;
                    break;
                case MsgType::JOB_LOCAL_BEGIN:
                    ret = handle_local_job(client, msg.get());
                    break;
                case MsgType::JOB_DONE:
                    ret = handle_job_done(client, dynamic_cast<JobDone *>(msg.get()));
                    break;
                case MsgType::VERIFY_ENV:
                    ret = handle_verify_env(client, dynamic_cast<VerifyEnv *>(msg.get()));
                    break;
                case MsgType::BLACKLIST_HOST_ENV:
                    ret = handle_blacklist_host_env(client, msg.get());
                    break;
                default:
                    log_error() << "not compile: " << (char)msg->type << "protocol error on client "
                                << client->dump() << std::endl;
                    client->channel->send_msg(End());
                    handle_end(client, 120);
                    ret = false;
                }

                return ret;
            }

            int Daemon::answer_client_requests()
            {
#ifdef ICECC_DEBUG

                if (clients.size() + current_kids) {
                    log_info() << dump_internals() << std::endl;
                }

                log_info() << "clients " << clients.dump_per_status() << " " << current_kids
                           << " (" << max_kids << ")" << std::endl;

#endif

                /* reap zombis */
                int status;

                while (waitpid(-1, &status, WNOHANG) < 0 && errno == EINTR) {}

                handle_old_request();

                /* collect the stats after the children exited icecream_load */
                if (scheduler) {
                    maybe_stats();
                }

                fd_set listen_set;
                struct timeval tv;

                FD_ZERO(&listen_set);
                int max_fd = 0;

                if (tcp_listen_fd != -1) {
                    FD_SET(tcp_listen_fd, &listen_set);
                    max_fd = tcp_listen_fd;
                }

                FD_SET(unix_listen_fd, &listen_set);

                if (unix_listen_fd > max_fd) { // very likely
                    max_fd = unix_listen_fd;
                }

                for (auto cit = fd2chan.cbegin(); cit != fd2chan.cend();)
                {
                    int i = cit->first;
                    Channel *c = cit->second;
                    ++cit;
                    /* don't select on a fd that we're currently not interested in.
                       Avoids that we wake up on an event we're not handling anyway */
                    Client *client = clients.find_by_channel(c);
                    assert(client);
                    int current_status = client->status;
                    bool ignore_channel = current_status == Status::TOCOMPILE
                        || current_status == Status::WAITFORCHILD;

                    if (!ignore_channel && (!c->has_msg() || handle_activity(client)))
                    {
                        if (i > max_fd)
                        {
                            max_fd = i;
                        }

                        FD_SET(i, &listen_set);
                    }

                    if (current_status == Status::WAITFORCHILD
                        && client->pipe_to_child != -1)
                    {
                        if (client->pipe_to_child > max_fd)
                        {
                            max_fd = client->pipe_to_child;
                        }

                        FD_SET(client->pipe_to_child, &listen_set);
                    }
                }

                if (scheduler)
                {
                    FD_SET(scheduler->fd, &listen_set);

                    if (max_fd < scheduler->fd)
                    {
                        max_fd = scheduler->fd;
                    }
                }
                else if (discover && discover->listen_fd() >= 0)
                {
                    /* We don't explicitely check for discover->get_fd() being in
                       the selected set below.  If it's set, we simply will return
                       and our call will make sure we try to get the scheduler.  */
                    FD_SET(discover->listen_fd(), &listen_set);

                    if (max_fd < discover->listen_fd())
                    {
                        max_fd = discover->listen_fd();
                    }
                }

                for (const auto &cit : native_environments)
                {
                    if (cit.second.create_env_pipe)
                    {
                        FD_SET(cit.second.create_env_pipe, &listen_set);
                        if (max_fd < cit.second.create_env_pipe)
                            max_fd = cit.second.create_env_pipe;
                    }
                }

                tv.tv_sec = max_scheduler_pong;
                tv.tv_usec = 0;

                int ret = select(max_fd + 1, &listen_set, nullptr, nullptr, &tv);

                if (ret < 0 && errno != EINTR)
                {
                    log_perror("select");
                    return 5;
                }

                if (ret > 0)
                {
                    bool had_scheduler = scheduler.get();

                    if (scheduler && FD_ISSET(scheduler->fd, &listen_set))
                    {
                        while (!scheduler->read_a_bit() || scheduler->has_msg())
                        {
                            services::Msg *msg = scheduler->get_msg().get();

                            if (!msg)
                            {
                                log_error() << "scheduler closed connection" << std::endl;
                                close_scheduler();
                                clear_children();
                                return 1;
                            }

                            ret = 0;

                            switch (msg->type)
                            {
                            case MsgType::PING:

                                if (!services::is_protocol<27>()(*scheduler))
                                {
                                    ret = !send_scheduler(Ping());
                                }

                                break;
                            case MsgType::USE_CS:
                                ret = scheduler_use_cs(static_cast<UseCS *>(msg));
                                break;
                            case MsgType::GET_INTERNALS:
                                ret = scheduler_get_internals();
                                break;
                            case MsgType::CS_CONF:
                                ret = handle_cs_conf(static_cast<ConfCS *>(msg));
                                break;
                            default:
                                log_error() << "unknown scheduler type " << (char)msg->type << std::endl;
                                ret = 1;
                            }

                            delete msg;

                            if (ret) {
                                return ret;
                            }
                        }
                    }

                    int listen_fd = -1;

                    if (tcp_listen_fd != -1 && FD_ISSET(tcp_listen_fd, &listen_set))
                    {
                        listen_fd = tcp_listen_fd;
                    }

                    if (FD_ISSET(unix_listen_fd, &listen_set))
                    {
                        listen_fd = unix_listen_fd;
                    }

                    if (listen_fd != -1)
                    {
                        struct sockaddr cli_addr;
                        socklen_t cli_len = sizeof cli_addr;
                        int acc_fd = accept(listen_fd, &cli_addr, &cli_len);

                        if (acc_fd < 0)
                        {
                            log_perror("accept error");
                        }

                        if (acc_fd == -1 && errno != EINTR)
                        {
                            log_perror("accept failed:");
                            return EXIT_CONNECT_FAILED;
                        }

                        std::shared_ptr<Channel> c = Service::createChannel(acc_fd, &cli_addr, cli_len);

                        if (!c)
                        {
                            return 0;
                        }

                        trace() << "accepted " << c->fd << " " << c->name << std::endl;

                        Client *client = new Client;
                        client->client_id = ++new_client_id;
                        client->channel = c;
                        clients[c.get()] = client;

                        fd2chan[c->fd] = c.get();

                        while (!c->read_a_bit() || c->has_msg())
                        {
                            if (!handle_activity(client)) {
                                break;
                            }

                            if (client->status == Status::TOCOMPILE
                                || client->status == Status::WAITFORCHILD)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (auto cit = fd2chan.cbegin();
                             max_fd && cit != fd2chan.cend();)
                        {
                            int i = cit->first;
                            Channel *c = cit->second;
                            Client *client = clients.find_by_channel(c);
                            assert(client);
                            ++cit;

                            if (client->status == Status::WAITFORCHILD
                                && client->pipe_to_child >= 0
                                && FD_ISSET(client->pipe_to_child, &listen_set))
                            {
                                max_fd--;

                                if (!handle_compile_done(client))
                                {
                                    return 1;
                                }
                            }

                            if (FD_ISSET(i, &listen_set))
                            {
                                assert(client->status != Status::TOCOMPILE);

                                while (!c->read_a_bit() || c->has_msg())
                                {
                                    if (!handle_activity(client))
                                    {
                                        break;
                                    }

                                    if (client->status == Status::TOCOMPILE
                                        || client->status == Status::WAITFORCHILD)
                                    {
                                        break;
                                    }
                                }

                                max_fd--;
                            }
                        }

                        for (auto cit = native_environments.cbegin();
                             cit != native_environments.end();)
                        {
                            if (cit->second.create_env_pipe && FD_ISSET(cit->second.create_env_pipe, &listen_set))
                            {
                                if(!create_env_finished(cit->first))
                                {
                                    native_environments.erase(cit++);
                                    continue;
                                }
                            }
                            ++cit;
                        }

                    }

                    if (had_scheduler && !scheduler) {
                        clear_children();
                        return 2;
                    }

                }

                return 0;
            }

            bool Daemon::reconnect()
            {
                if (scheduler) {
                    return true;
                }

                if (!discover && next_scheduler_connect > time(nullptr))
                {
                    trace() << "timeout.." << std::endl;
                    return false;
                }

#ifdef ICECC_DEBUG
                trace() << "reconn " << dump_internals() << std::endl;
#endif

                if (!discover || (NULL == (scheduler = discover->try_get_scheduler()) && discover->timed_out())) {
                    delete discover;
                    discover = new DiscoverSched(netname, max_scheduler_pong, schedname, scheduler_port);
                }

                if (!scheduler) {
                    log_warning() << "scheduler not yet found." << std::endl;
                    return false;
                }

                delete discover;
                discover = nullptr;
                sockaddr_in name;
                socklen_t len = sizeof(name);
                int error = getsockname(scheduler->fd, (struct sockaddr*)&name, &len);

                if (!error) {
                    remote_name = inet_ntoa(name.sin_addr);
                } else {
                    remote_name = std::string();
                }

                log_info() << "Connected to scheduler (I am known as " << remote_name << ")" << std::endl;
                current_load = -1000;
                gettimeofday(&last_stat, nullptr);
                icecream_load = 0;

                services::Login lmsg(daemon_port, determine_nodename(), machine_name);
                lmsg.envs = available_environmnents(envbasedir);
                lmsg.max_kids = max_kids;
                lmsg.noremote = noremote;
                return send_scheduler(lmsg);
            }

            int Daemon::working_loop()
            {
                for (;;) {
                    reconnect();

                    int ret = answer_client_requests();

                    if (ret) {
                        trace() << "answer_client_requests returned " << ret << std::endl;
                        close_scheduler();
                    }

                    if (exit_main_loop)
                        break;
                }
                return 0;
            }
    } // daemon
} // icecream
