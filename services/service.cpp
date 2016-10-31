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


#include "service.h"

namespace icecream
{
    namespace services
    {
        std::shared_ptr<Channel> Service::createChannel(const std::string &hostname,
                                                        unsigned short p,
                                                        int timeout)
        {
            int remote_fd;
            struct sockaddr_in remote_addr;

            if ((remote_fd = prepare_connect(hostname, p, remote_addr)) < 0)
            {
                return nullptr;
            }

            if (timeout)
            {
                if (!connect_async(remote_fd,
                                   reinterpret_cast<struct sockaddr*>(&remote_addr),
                                   sizeof(remote_addr), timeout))
                {
                    return nullptr;    // remote_fd is already closed
                }
            }
            else
            {
                int i = 2048;
                setsockopt(remote_fd, SOL_SOCKET, SO_SNDBUF, &i, sizeof(i));

                if (connect(remote_fd,
                            reinterpret_cast<struct sockaddr*>(&remote_addr),
                            sizeof(remote_addr)) < 0)
                {
                    close(remote_fd);
                    trace() << "connect failed on " << hostname << std::endl;
                    return nullptr;
                }
            }

            trace() << "connected to " << hostname << std::endl;
            return createChannel(remote_fd,
                                 reinterpret_cast<struct sockaddr*>(&remote_addr),
                                 sizeof(remote_addr));
        }

        std::shared_ptr<Channel> Service::createChannel(int fd, struct sockaddr *_a,
                                                        socklen_t _l)
        {
            std::shared_ptr<Channel> c{new Channel(fd, _a, _l, false)};

            if (!c->wait_for_protocol())
            {
                c.reset();
            }

            return c;
        }

        std::shared_ptr<Channel> Service::createChannel(const std::string &socket_path)
        {
            int remote_fd;
            struct sockaddr_un remote_addr;

            if ((remote_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            {
                log_perror("socket()");
                return nullptr;
            }

            remote_addr.sun_family = AF_UNIX;
            strncpy(remote_addr.sun_path, socket_path.c_str(),
                    sizeof(remote_addr.sun_path) - 1);

            if (connect(remote_fd,
                        reinterpret_cast<struct sockaddr*>(&remote_addr),
                        sizeof(remote_addr)) < 0)
            {
                close(remote_fd);
                trace() << "connect failed on " << socket_path << std::endl;
                return nullptr;
            }

            trace() << "connected to " << socket_path << std::endl;
            return createChannel(remote_fd,
                                 reinterpret_cast<struct sockaddr*>(&remote_addr),
                                 sizeof(remote_addr));
        }
    } // services
} // icecream
