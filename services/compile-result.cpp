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


#include "compile-result.h"

namespace icecream
{
    namespace services
    {
        void CompileResult::fill_from_channel(Channel *c)
        {
            Msg::fill_from_channel(c);
            uint32_t _status = 0;
            *c >> err;
            *c >> out;
            *c >> _status;
            status = _status;
            uint32_t was = 0;
            *c >> was;
            was_out_of_memory = was;
            if (is_protocol<35>()(*c))
            {
                uint32_t dwo = 0;
                *c >> dwo;
                have_dwo_file = dwo;
            }
        }

        void CompileResult::send_to_channel(Channel *c) const
        {
            Msg::send_to_channel(c);
            *c << err;
            *c << out;
            *c << status;
            *c << (uint32_t) was_out_of_memory;
            if (is_protocol<35>()(*c))
            {
                *c << (uint32_t) have_dwo_file;
            }
        }
    } // services
} // icecream
