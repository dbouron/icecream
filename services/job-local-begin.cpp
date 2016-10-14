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


#include "job-local-begin.h"

namespace icecream
{
    namespace services
    {
        void JobLocalBegin::fill_from_channel(Channel *c)
        {
            Msg::fill_from_channel(c);
            *c >> stime;
            *c >> outfile;
            *c >> id;
        }

        void JobLocalBegin::send_to_channel(Channel *c) const
        {
            Msg::send_to_channel(c);
            *c << stime;
            *c << outfile;
            *c << id
        }
    } // services
} // icecream
