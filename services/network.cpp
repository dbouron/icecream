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

# include "network.h"

namespace icecream
{
    namespace services
    {
        int prepare_connect(const std::string &hostname, unsigned short p,
                            struct sockaddr_in &remote_addr)
        {
            struct hostent *host = nullptr;
            int remote_fd;
            int i = 1;

            if ((remote_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
            {
                log_perror("socket()");
                return -1;
            }
            host = gethostbyname(hostname.c_str());
            if (!host)
            {
                log_perror("Unknown host");
                close(remote_fd);
                return -1;
            }

            if (host->h_length != 4)
            {
                log_error() << "Invalid address length" << std::endl;
                close(remote_fd);
                return -1;
            }

            setsockopt(remote_fd, IPPROTO_TCP, TCP_NODELAY,
                       (char*)&i, sizeof (i));
            remote_addr.sin_family = AF_INET;
            remote_addr.sin_port = htons(p);
            memcpy(&remote_addr.sin_addr.s_addr, host->h_addr_list[0],
                   host->h_length);
            return remote_fd;
        }

         bool connect_async(int remote_fd, struct sockaddr *remote_addr,
                            size_t remote_size, int timeout)
        {
            fcntl(remote_fd, F_SETFL, O_NONBLOCK);

            // code majorly derived from lynx's http connect (GPL)
            int status = connect(remote_fd, remote_addr, remote_size);

            if ((status < 0) && (errno == EINPROGRESS || errno == EAGAIN))
            {
                struct timeval select_timeout;
                fd_set writefds;
                int ret;

                do
                {
                    /* we select for a specific time and if that succeeds, we connect one
                       final time. Everything else we ignore */

                    select_timeout.tv_sec = timeout;
                    select_timeout.tv_usec = 0;
                    FD_ZERO(&writefds);
                    FD_SET(remote_fd, &writefds);
                    ret = select(remote_fd + 1, nullptr, &writefds, nullptr, &select_timeout);

                    if (ret < 0 && errno == EINTR)
                    {
                        continue;
                    }

                    break;
                } while (1);

                if (ret > 0)
                {
                    /*
                    **  Extra check here for connection success, if we try to
                    **  connect again, and get EISCONN, it means we have a
                    **  successful connection.  But don't check with SOCKS.
                    */
                    status = connect(remote_fd, remote_addr, remote_size);

                    if ((status < 0) && (errno == EISCONN))
                    {
                        status = 0;
                    }
                }
            }

            if (status < 0)
            {
                /*
                **  The connect attempt failed or was interrupted,
                **  so close up the socket.
                */
                close(remote_fd);
                return false;
            }
            else
            {
                /*
                **  Make the socket blocking again on good connect.
                */
                fcntl(remote_fd, F_SETFL, 0);
            }

            return true;
        }



         std::string shorten_filename(const std::string &str)
        {
            std::string::size_type ofs = str.rfind('/');

            for (int i = 2; i--;)
            {
                if (ofs != std::string::npos)
                {
                    ofs = str.rfind('/', ofs - 1);
                }
            }

            return str.substr(ofs + 1);
        }

        /* Returns a filedesc. or a negative value for errors.  */
         int open_send_broadcast(int port, const char* buf, int size)
        {
            int ask_fd;
            struct sockaddr_in remote_addr;

            if ((ask_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
            {
                log_perror("open_send_broadcast socket");
                return -1;
            }

            if (fcntl(ask_fd, F_SETFD, FD_CLOEXEC) < 0)
            {
                log_perror("open_send_broadcast fcntl");
                close(ask_fd);
                return -1;
            }

            int optval = 1;

            if (setsockopt(ask_fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval))
                < 0)
            {
                log_perror("open_send_broadcast setsockopt");
                close(ask_fd);
                return -1;
            }

            struct kde_ifaddrs *addrs;

            int ret = kde_getifaddrs(&addrs);

            if (ret < 0)
            {
                return ret;
            }

            for (struct kde_ifaddrs *addr = addrs; addr != nullptr; addr = addr->ifa_next)
            {
                /*
                 * See if this interface address is IPv4...
                 */

                if (addr->ifa_addr == nullptr || addr->ifa_addr->sa_family != AF_INET
                    || addr->ifa_netmask == nullptr || addr->ifa_name == nullptr)
                {
                    continue;
                }

                if (ntohl(((struct sockaddr_in *) addr->ifa_addr)->sin_addr.s_addr)
                    == 0x7f000001)
                {
                    trace() << "ignoring localhost " << addr->ifa_name << std::endl;
                    continue;
                }

                if ((addr->ifa_flags & IFF_POINTOPOINT)
                    || !(addr->ifa_flags & IFF_BROADCAST))
                {
                    log_info() << "ignoring tunnels " << addr->ifa_name << std::endl;
                    continue;
                }

                if (addr->ifa_broadaddr)
                {
                    log_info() << "broadcast " << addr->ifa_name << " "
                               << inet_ntoa(
                                   ((sockaddr_in *) addr->ifa_broadaddr)->sin_addr)
                               << std::endl;

                    remote_addr.sin_family = AF_INET;
                    remote_addr.sin_port = htons(port);
                    remote_addr.sin_addr =
                        ((sockaddr_in *) addr->ifa_broadaddr)->sin_addr;

                    if (sendto(ask_fd, buf, size, 0, (struct sockaddr *) &remote_addr,
                               sizeof(remote_addr)) != size)
                    {
                        log_perror("open_send_broadcast sendto");
                    }
                }
            }

            kde_freeifaddrs(addrs);
            return ask_fd;
        }

         bool get_broad_answer(int ask_fd, int timeout, char *buf2,
                               struct sockaddr_in *remote_addr, socklen_t *remote_len)
        {
            char buf = PROTOCOL_VERSION;
            fd_set read_set;
            FD_ZERO(&read_set);
            FD_SET(ask_fd, &read_set);
            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = 1000 * (timeout % 1000);
            errno = 0;

            if (select(ask_fd + 1, &read_set, nullptr, nullptr, &tv) <= 0)
            {
                /* Normally this is a timeout, i.e. no scheduler there.  */
                if (errno)
                {
                    log_perror("waiting for scheduler");
                }

                return false;
            }

            *remote_len = sizeof(struct sockaddr_in);

            int len = recvfrom(ask_fd, buf2, BROAD_BUFLEN, 0,
                               (struct sockaddr *) remote_addr, remote_len);
            if (len != BROAD_BUFLEN && len != BROAD_BUFLEN_OLD)
            {
                log_perror("get_broad_answer recvfrom()");
                return false;
            }

            if ((len == BROAD_BUFLEN_OLD && buf2[0] != buf + 1) // PROTOCOL <= 32 scheduler
                || (len == BROAD_BUFLEN && buf2[0] != buf + 2))
            { // PROTOCOL >= 33 scheduler
                log_error() << "wrong answer" << std::endl;
                return false;
            }

            buf2[len - 1] = 0;
            return true;
        }

         void get_broad_data(const char* buf, const char** name, int* version,
                             time_t* start_time)
        {
            if (buf[0] == PROTOCOL_VERSION + 1)
            {
                // Scheduler version 32 or older, didn't send us its version, assume it's 32.
                if (name != nullptr)
                    *name = buf + 1;
                if (version != nullptr)
                    *version = 32;
                if (start_time != nullptr)
                    *start_time = 0; // Unknown too.
            }
            else if (buf[0] == PROTOCOL_VERSION + 2)
            {
                if (version != nullptr)
                {
                    uint32_t tmp_version;
                    memcpy(&tmp_version, buf + 1, sizeof(uint32_t));
                    *version = tmp_version;
                }
                if (start_time != nullptr)
                {
                    uint64_t tmp_time;
                    memcpy(&tmp_time, buf + 1 + sizeof(uint32_t), sizeof(uint64_t));
                    *start_time = tmp_time;
                }
                if (name != nullptr)
                    *name = buf + 1 + sizeof(uint32_t) + sizeof(uint64_t);
            }
            else
            {
                abort();
            }
        }
    } // services
} // icecream
