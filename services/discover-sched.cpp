/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Michael Matz <matz@suse.de>
                  2004 Stephan Kulow <coolo@suse.de>
                  2007 Dirk Mueller <dmueller@suse.de>

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


#include "discover-sched.h"

namespace icecream
{
    namespace services
    {
        DiscoverSched::DiscoverSched(const std::string &_netname, int _timeout,
                const std::string &_schedname, int port)
                : netname(_netname)
                , schedname(_schedname)
                , timeout(_timeout)
                , ask_fd(-1)
                , sport(port)
                , best_version(0)
                , best_start_time(0)
                , multiple(false)
        {
            time0 = time(0);

            if (schedname.empty())
            {
                const char *get = getenv("USE_SCHEDULER");

                if (get)
                {
                    std::string scheduler = get;
                    size_t colon = scheduler.rfind(':');
                    if (colon == std::string::npos)
                    {
                        schedname = scheduler;
                    }
                    else
                    {
                        schedname = scheduler.substr(0, colon);
                        sport = atoi(scheduler.substr(colon + 1).c_str());
                    }
                }
            }

            if (netname.empty())
            {
                netname = "ICECREAM";
            }
            if (sport == 0)
            {
                sport = 8765;
            }

            if (!schedname.empty())
            {
                netname = ""; // take whatever the machine is giving us
                attempt_scheduler_connect();
            }
            else
            {
                char buf = PROTOCOL_VERSION;
                ask_fd = open_send_broadcast(sport, &buf, 1);
            }
        }

        DiscoverSched::~DiscoverSched()
        {
            if (ask_fd >= 0)
            {
                close (ask_fd);
            }
        }

        bool DiscoverSched::timed_out()
        {
            return (time(0) - time0 >= timeout);
        }

        void DiscoverSched::attempt_scheduler_connect()
        {

            time0 = time(0) + MAX_SCHEDULER_PONG;
            log_info() << "scheduler is on " << schedname << ":" << sport << " (net "
                    << netname << ")" << std::endl;

            if ((ask_fd = prepare_connect(schedname, sport, remote_addr)) >= 0)
            {
                fcntl(ask_fd, F_SETFL, O_NONBLOCK);
            }
        }

        Channel *DiscoverSched::try_get_scheduler()
        {
            if (schedname.empty() || 0 != best_version)
            {
                socklen_t remote_len;
                char buf2[BROAD_BUFLEN];
                /* Try to get the scheduler with the newest version, and if there
                 are several with the same version, choose the one that's been running
                 for the longest time. It should work like this (and it won't work
                 perfectly if there are schedulers and/or daemons with old (<33) version):

                 Whenever a daemon starts, it broadcasts for a scheduler. Schedulers all
                 see the broadcast and respond with their version, start time and netname.
                 Here we select the best one.
                 If a new scheduler is started, it'll broadcast its version and all
                 other schedulers will drop their daemon connections if they have an older
                 version. If the best scheduler quits, all daemons will get their connections
                 closed and will re-discover and re-connect.
                 */

                /* Read/test all packages arrived until now.  */
                while (get_broad_answer(ask_fd, 0/*timeout*/, buf2,
                        (struct sockaddr_in *) &remote_addr, &remote_len))
                {
                    int version;
                    time_t start_time;
                    const char* name;
                    get_broad_data(buf2, &name, &version, &start_time);
                    if (strcasecmp(netname.c_str(), name) == 0)
                    {
                        if (version < 33)
                        {
                            log_info() << "Suitable scheduler found at "
                                    << inet_ntoa(remote_addr.sin_addr) << ":"
                                    << ntohs(remote_addr.sin_port)
                                    << " (unknown version)" << std::endl;
                        }
                        else
                        {
                            log_info() << "Suitable scheduler found at "
                                    << inet_ntoa(remote_addr.sin_addr) << ":"
                                    << ntohs(remote_addr.sin_port) << " (version: "
                                    << version << ")" << std::endl;
                        }
                        if (best_version != 0)
                            multiple = true;
                        if (best_version < version
                                || (best_version == version
                                        && best_start_time > start_time))
                        {
                            schedname = inet_ntoa(remote_addr.sin_addr);
                            sport = ntohs(remote_addr.sin_port);
                            best_version = version;
                            best_start_time = start_time;
                        }
                    }
                }

                if (timed_out())
                {
                    if (best_version == 0)
                    {
                        return 0;
                    }
                    if (multiple)
                        log_info() << "Selecting scheduler at " << schedname << ":"
                                << sport << std::endl;

                    close (ask_fd);
                    ask_fd = -1;
                    attempt_scheduler_connect();

                    if (ask_fd >= 0)
                    {
                        int status = connect(ask_fd, (struct sockaddr *) &remote_addr,
                                sizeof(remote_addr));

                        if (status == 0
                                || (status < 0
                                        && (errno == EISCONN || errno == EINPROGRESS)))
                        {
                            int fd = ask_fd;
                            ask_fd = -1;
                            return Service::createChannel(fd,
                                    (struct sockaddr *) &remote_addr,
                                    sizeof(remote_addr));
                        }
                    }
                }
            }
            else if (ask_fd >= 0)
            {
                int status = connect(ask_fd, (struct sockaddr *) &remote_addr,
                        sizeof(remote_addr));

                if (status == 0 || (status < 0 && errno == EISCONN))
                {
                    int fd = ask_fd;
                    ask_fd = -1;
                    return Service::createChannel(fd, (struct sockaddr *) &remote_addr,
                            sizeof(remote_addr));
                }
            }

            return 0;
        }

        bool DiscoverSched::broadcastData(int port, const char* buf, int len)
        {
            int fd = open_send_broadcast(port, buf, len);
            if (fd >= 0)
            {
                close(fd);
                return true;
            }
            return false;
        }

        std::list<std::string> get_netnames(int timeout, int port)
        {
            std::list<std::string> l;
            int ask_fd;
            struct sockaddr_in remote_addr;
            socklen_t remote_len;
            time_t time0 = time(0);

            char buf = PROTOCOL_VERSION;
            ask_fd = open_send_broadcast(port, &buf, 1);

            do
            {
                char buf2[BROAD_BUFLEN];
                bool first = true;
                /* Wait at least two seconds to give all schedulers a chance to answer
                 (unless that'd be longer than the timeout).*/
                time_t timeout_time = time(NULL) + std::min(2 + 1, timeout);

                /* Read/test all arriving packages.  */
                while (get_broad_answer(ask_fd, first ? timeout : 0, buf2, &remote_addr,
                        &remote_len) && time(NULL) < timeout_time)
                {
                    first = false;
                    const char* name;
                    get_broad_data(buf2, &name, NULL, NULL);
                    l.push_back(name);
                }
            } while (time(0) - time0 < (timeout / 1000));

            close(ask_fd);
            return l;
        }
    } // services
} // icecream
