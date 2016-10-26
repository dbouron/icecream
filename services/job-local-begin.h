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


#ifndef ICECREAM_JOB_LOCAL_BEGIN_H
# define ICECREAM_JOB_LOCAL_BEGIN_H

# include "msg.h"

namespace icecream
{
    namespace services
    {
        class JobLocalBegin : public Msg {
        public:
            JobLocalBegin(int job_id = 0, const std::string &file = "")
                : Msg(MsgType::JOB_LOCAL_BEGIN)
                , outfile(file)
                , stime(time(nullptr))
                , id(job_id) {
            }

            virtual void fill_from_channel(std::shared_ptr<Channel> c);
            virtual void send_to_channel(std::shared_ptr<Channel> c) const;

            std::string outfile;
            uint32_t stime;
            uint32_t id;
        };
    } // services
} // icecream

#endif /* !ICECREAM_JOB_LOCAL_BEGIN_H */
