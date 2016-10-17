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


#ifndef ICECREAM_GET_CS_H
# define ICECREAM_GET_CS_H

# include <string>
# include <algorithm>

# include "msg.h"
# include "comm.h"
# include "job.h"
# include "protocol.h"
# include "network.h"

namespace icecream
{
    namespace services
    {
        class GetCS : public Msg
        {
        public:
            GetCS();
            GetCS(const Environments &envs, const std::string &f,
                  Language _lang, unsigned int _count,
                  std::string _target, unsigned int _arg_flags,
                  const std::string &host, int _minimal_host_version);

            virtual void fill_from_channel(Channel *c);
            virtual void send_to_channel(Channel *c) const;

            Environments versions;
            std::string filename;
            Language lang;
            uint32_t count; // the number of UseCS messages to answer with - usually 1
            std::string target;
            uint32_t arg_flags;
            uint32_t client_id;
            std::string preferred_host;
            int minimal_host_version;
        };
    } // services
} // icecream

#endif /* !ICECREAM_GET_CS_H */
