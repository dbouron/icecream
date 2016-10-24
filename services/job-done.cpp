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


#include "job-done.h"

namespace icecream
{
    namespace services
    {
        JobDone::JobDone(int id, int exit, unsigned int _flags)
                : Msg(MsgType::JOB_DONE)
                , exitcode(exit)
                , flags(_flags)
                , job_id(id)
        {
            real_msec = 0;
            user_msec = 0;
            sys_msec = 0;
            pfaults = 0;
            in_compressed = 0;
            in_uncompressed = 0;
            out_compressed = 0;
            out_uncompressed = 0;
        }

        void JobDone::fill_from_channel(std::shared_ptr<Channel> c)
        {
            Msg::fill_from_channel(c);
            uint32_t _exitcode = 255;
            *c >> job_id;
            *c >> _exitcode;
            *c >> real_msec;
            *c >> user_msec;
            *c >> sys_msec;
            *c >> pfaults;
            *c >> in_compressed;
            *c >> in_uncompressed;
            *c >> out_compressed;
            *c >> out_uncompressed;
            *c >> flags;
            exitcode = (int) _exitcode;
        }

        void JobDone::send_to_channel(std::shared_ptr<Channel> c) const
        {
            Msg::send_to_channel(c);
            *c << job_id;
            *c << (uint32_t) exitcode;
            *c << real_msec;
            *c << user_msec;
            *c << sys_msec;
            *c << pfaults;
            *c << in_compressed;
            *c << in_uncompressed;
            *c << out_compressed;
            *c << out_uncompressed;
            *c << flags;
        }
    } // services
} // icecream
