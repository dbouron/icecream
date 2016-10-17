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

# include "login.h"

namespace icecream
{
    namespace services
    {
        Login::Login(unsigned int myport,
                    const std::string &_nodename,
                    const std::string _host_platform)
                : Msg(MsgType::LOGIN)
                , port(myport)
                , max_kids(0)
                , noremote(false)
                , chroot_possible(false)
                , nodename(_nodename)
                , host_platform(_host_platform)
        {
#ifdef HAVE_LIBCAP_NG
            chroot_possible = capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_CHROOT);
#else
            // check if we're root
            chroot_possible = (geteuid() == 0);
#endif
        }

        void Login::fill_from_channel(Channel *c)
        {
            Msg::fill_from_channel(c);
            *c >> port;
            *c >> max_kids;
            c->read_environments(envs);
            *c >> nodename;
            *c >> host_platform;
            uint32_t net_chroot_possible = 0;
            *c >> net_chroot_possible;
            chroot_possible = net_chroot_possible != 0;
            uint32_t net_noremote = 0;

            if (is_protocol<26>()(*c))
            {
                *c >> net_noremote;
            }

            noremote = (net_noremote != 0);
        }

        void Login::send_to_channel(Channel *c) const
        {
            Msg::send_to_channel(c);
            *c << port;
            *c << max_kids;
            c->write_environments(envs);
            *c << nodename;
            *c << host_platform;
            *c << chroot_possible;

            if (is_protocol<26>()(*c))
            {
                *c << noremote;
            }
        }
    } // services
} // icecream
