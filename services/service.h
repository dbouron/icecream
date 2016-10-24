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

/**
 ** \file services/service.h
 ** \brief Just convenient functions to create Channels with friendly acces to
 ** \c Channel class.
 **
 ** \todo Move this function to \c Channel ctor?
 */

#ifndef ICECREAM_SERVICE_H
# define ICECREAM_SERVICE_H

# include <string>
# include <iostream>

# include "network.h"
# include "channel.h"

namespace icecream
{
    namespace services
    {
        class Service
        {
        public:
            static std::shared_ptr<Channel> createChannel(const std::string &host,
                                          unsigned short p,
                                          int timeout);
            static std::shared_ptr<Channel> createChannel(const std::string &domain_socket);
            static std::shared_ptr<Channel> createChannel(int remote_fd, struct sockaddr *,
                                                          socklen_t);
        };
    } // services
} // icecream

#endif /* !ICECREAM_SERVICE_H */
