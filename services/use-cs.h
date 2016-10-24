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


#ifndef ICECREAM_USE_CS_H
# define ICECREAM_USE_CS_H

# include <string>

# include "msg.h"
# include "protocol.h"

namespace icecream
{
    namespace services
    {
        class UseCS : public Msg
        {
        public:
            UseCS() :
                Msg(MsgType::USE_CS)
            {
            }

            UseCS(std::string platform, std::string host, unsigned int p,
                  unsigned int id, bool gotit, unsigned int _client_id,
                  unsigned int matched_host_jobs)
                : Msg(MsgType::USE_CS)
                , job_id(id)
                , hostname(host)
                , port(p)
                , host_platform(platform)
                , got_env(gotit)
                , client_id(_client_id)
                , matched_job_id(matched_host_jobs)
            {
            }

            virtual void fill_from_channel(std::shared_ptr<Channel> c);
            virtual void send_to_channel(std::shared_ptr<Channel> c) const;

            uint32_t job_id;
            std::string hostname;
            uint32_t port;
            std::string host_platform;
            uint32_t got_env;
            uint32_t client_id;
            uint32_t matched_job_id;
        };
    } // services
} // icecream

#endif /* !ICECREAM_USE_CS_H */
