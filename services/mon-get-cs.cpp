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


#include "mon-get-cs.h"

namespace icecream
{
    namespace services
    {
        void MonGetCS::fill_from_channel(Channel *c)
        {
            if (is_protocol<29>()(*c))
            {
                Msg::fill_from_channel(c);
                *c >> filename;
                uint32_t _lang;
                *c >> _lang;
                lang = static_cast<CompileJob::Language>(_lang);
            }
            else
            {
                GetCS::fill_from_channel(c);
            }

            *c >> job_id;
            *c >> clientid;
        }

        void MonGetCS::send_to_channel(Channel *c) const
        {
            if (is_protocol<29>()(*c))
            {
                Msg::send_to_channel(c);
                *c << shorten_filename(filename);
                *c << (uint32_t) lang;
            }
            else
            {
                GetCS::send_to_channel(c);
            }

            *c << job_id;
            *c << clientid;
        }
    } // services
} // icecream
