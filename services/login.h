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


#ifndef ICECREAM_LOGIN_H
# define ICECREAM_LOGIN_H

# include <sys/types.h>
# include <unistd.h>

# include "msg.h"
# include "protocol.h"

namespace icecream
{
    namespace services
    {
        class Login : public Msg
        {
        public:
            Login(unsigned int myport, const std::string &_nodename,
                    const std::string _host_platform);
            Login()
                    : Msg(MsgType::LOGIN)
                    , port(0)
            {
            }

            virtual void fill_from_channel(std::shared_ptr<Channel> c);
            virtual void send_to_channel(std::shared_ptr<Channel> c) const;

            uint32_t port;
            Environments envs;
            uint32_t max_kids;
            bool noremote;
            bool chroot_possible;
            std::string nodename;
            std::string host_platform;
        };

    } // services
} // icecream

#endif /* !ICECREAM_LOGIN_H */
