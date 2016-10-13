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


#ifndef ICECREAM_STATS_H
# define ICECREAM_STATS_H

namespace icecream
{
    namespace services
    {
        class Stats : public Msg
        {
        public:
            Stats()
                    : Msg(MsgType::STATS)
            {
                load = 0;
            }

            virtual void fill_from_channel(Channel *c);
            virtual void send_to_channel(Channel *c) const;

            /**
             * For now the only load measure we have is the
             * load from 0-1000.
             * This is defined to be a daemon defined value
             * on how busy the machine is. The higher the load
             * is, the slower a given job will compile (preferably
             * linear scale). Load of 1000 means to not schedule
             * another job under no circumstances.
             */
            uint32_t load;

            uint32_t loadAvg1;
            uint32_t loadAvg5;
            uint32_t loadAvg10;
            uint32_t freeMem;
        };
    } // services
} // icecream

#endif /* !ICECREAM_STATS_H */
