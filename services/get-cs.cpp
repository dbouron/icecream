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


#include "get-cs.h"

namespace icecream
{
    namespace services
    {
        void GetCS::fill_from_channel(Channel *c)
        {
            Msg::fill_from_channel(c);
            c->read_environments(versions);
            *c >> filename;
            uint32_t _lang;
            *c >> _lang;
            *c >> count;
            *c >> target;
            lang = static_cast<CompileJob::Language>(_lang);
            *c >> arg_flags;
            *c >> client_id;
            preferred_host = string();

            if (is_protocol<22>()(C))
            {
                *c >> preferred_host;
            }

            minimal_host_version = 0;
            if (is_protocol<31>()(C))
            {
                uint32_t ign;
                *c >> ign;
                // Versions 31-33 had this as a separate field, now set a minimal
                // remote version if needed.
                if (ign != 0 && minimal_host_version < 31)
                    minimal_host_version = 31;
            }
            if (is_protocol<34>()(C))
            {
                uint32_t version;
                *c >> version;
                minimal_host_version = max(minimal_host_version, int(version));
            }
        }

        void GetCS::send_to_channel(Channel *c) const
        {
            Msg::send_to_channel(c);
            c->write_environments(versions);
            *c << shorten_filename(filename);
            *c << (uint32_t) lang;
            *c << count;
            *c << target;
            *c << arg_flags;
            *c << client_id;

            if (is_protocol<22>()(C))
            {
                *c << preferred_host;
            }

            if (is_protocol<31>()(C))
            {
                *c << uint32_t(minimal_host_version >= 31 ? 1 : 0);
            }
            if (is_protocol<34>()(C))
            {
                *c << minimal_host_version;
            }
        }
    } // services
} // icecream
