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


#ifndef ICECREAM_DISCOVER_SCHED_H
# define ICECREAM_DISCOVER_SCHED_H

namespace icecream
{
    namespace services
    {
        // this class is also used by icecream-monitor
        class DiscoverSched {
        public:
            /* Connect to a scheduler waiting max. TIMEOUT seconds.
               schedname can be the hostname of a box running a scheduler, to avoid
               broadcasting, port can be specified explicitly */
            DiscoverSched(const std::string &_netname = std::string(), int _timeout = 2,
                          const std::string &_schedname = std::string(), int port = 0);
            ~DiscoverSched();

            bool timed_out();

            int listen_fd() const {
                return schedname.empty() ? ask_fd : -1;
            }

            int connect_fd() const {
                return schedname.empty() ? -1 : ask_fd;
            }

            // compat for icecream monitor
            int get_fd() const {
                return listen_fd();
            }

            /* Attempt to get a conenction to the scheduler.

               Continue to call this while it returns NULL and timed_out()
               returns false. If this returns NULL you should wait for either
               more data on listen_fd() (use select), or a timeout of your own.
            */
            Channel *try_get_scheduler();

            // Returns the hostname of the scheduler - set by constructor or by try_get_scheduler
            std::string schedulerName() const {
                return schedname;
            }

            // Returns the network name of the scheduler - set by constructor or by try_get_scheduler
            std::string networkName() const {
                return netname;
            }

            /// Broadcasts the given data on the given port.
            static bool broadcastData(int port, const char* buf, int size);

        private:
            struct sockaddr_in remote_addr;
            std::string netname;
            std::string schedname;
            int timeout;
            int ask_fd;
            time_t time0;
            unsigned int sport;
            int best_version;
            time_t best_start_time;
            bool multiple;

            void attempt_scheduler_connect();
        };


        /* Return a list of all reachable netnames.  We wait max. WAITTIME
           milliseconds for answers.  */
        std::list<std::string> get_netnames(int waittime = 2000, int port = 8765);
    } // services
} // icecream

#endif /*!ICECREAM_DISCOVER_SCHED_H*/
