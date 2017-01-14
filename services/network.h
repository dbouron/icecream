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

#ifndef ICECREAM_NETWORK_H
# define ICECREAM_NETWORK_H

# include <string>
# include <iostream>
# include <cstdint>

# include <unistd.h>
# include <config.h>
# include <signal.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <arpa/inet.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# if HAVE_NETINET_TCP_VAR_H
#  include <sys/socketvar.h>
#  include <netinet/tcp_var.h>
# endif
# include <errno.h>
# include <fcntl.h>
# include <netdb.h>
# include <unistd.h>
# include <errno.h>
# include <assert.h>
# include <lzo/lzo1x.h>
# include <stdio.h>
# ifdef HAVE_LIBCAP_NG
#  include <cap-ng.h>
# endif
# include <net/if.h>
# include <sys/ioctl.h>

# include "getifaddrs.h"
# include <misc/logging.h>
# include "protocol.h"

namespace icecream
{
    namespace services
    {
        /*
         * A generic DoS protection. The biggest messages are of type FileChunk
         * which shouldn't be larger than 100kb. so anything bigger than 10 times
         * of that is definitely fishy, and we must reject it (we're running as root,
         * so be cautious).
         */
        constexpr uint32_t MAX_MSG_SIZE = 1 * 1024 * 1024;

        constexpr uint32_t BROAD_BUFLEN = 32;
        constexpr uint32_t BROAD_BUFLEN_OLD = 16;

        int prepare_connect(const std::string &hostname, unsigned short p,
                            struct sockaddr_in &remote_addr);

        bool connect_async(int remote_fd, struct sockaddr *remote_addr,
                           size_t remote_size, int timeout);

        std::string shorten_filename(const std::string &str);

        int open_send_broadcast(int port, const char* buf, int size);

        bool get_broad_answer(int ask_fd, int timeout, char *buf2,
                              struct sockaddr_in *remote_addr,
                              socklen_t *remote_len);

        void get_broad_data(const char* buf, const char** name,
                            int* version, time_t* start_time);
    } // services
} // icecream

#endif /* !ICECREAM_NETWORK_H */
