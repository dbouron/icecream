/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Michael Matz <matz@suse.de>
                  2004 Stephan Kulow <coolo@suse.de>

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

#ifndef ICECREAM_COMM_H
# define ICECREAM_COMM_H

# ifdef __linux__
#   include <stdint.h>
# endif
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>

# include <list>
# include <string>

# include "job.h"

namespace icecream
{
    namespace services
    {
        constexpr uint32_t MAX_SCHEDULER_PONG = 3;
        /// MAX_SCHEDULER_PING must be multiple of MAX_SCHEDULER_PONG
        constexpr uint32_t MAX_SCHEDULER_PING = 12 * MAX_SCHEDULER_PONG;
        /// maximum amount of time in seconds a daemon can be busy installing
        constexpr uint32_t MAX_BUSY_INSTALLING = 120;

        /// A list of pairs of host platform, filename
        using Environments = std::list<std::pair<std::string, std::string>>;

        enum MsgType : uint32_t
        {
            // so far unknown
            UNKNOWN = 'A',

            /* When the scheduler didn't get STATS from a CS
               for a specified time (e.g. 10m), then he sends a
               ping */
            PING,

            /* Either the end of file chunks or connection (A<->A) */
            END,

            TIMEOUT, // unused

            // C --> CS
            GET_NATIVE_ENV,
            // CS -> C
            NATIVE_ENV,

            // C --> S
            GET_CS,
            // S --> C
            USE_CS,  // = 'G'

            // C --> CS
            COMPILE_FILE, // = 'I'
            // generic file transfer
            FILE_CHUNK,
            // CS --> C
            COMPILE_RESULT,

            // CS --> S (after the C got the CS from the S, the CS tells the S when the C asks him)
            JOB_BEGIN,
            JOB_DONE,     // = 'M'

            // C --> CS, CS --> S (forwarded from C), _and_ CS -> C as start ping
            JOB_LOCAL_BEGIN, // = 'N'
            JOB_LOCAL_DONE,

            // CS --> S, first message sent
            LOGIN,
            // CS --> S (periodic)
            STATS,

            // messages between monitor and scheduler
            MON_LOGIN,
            MON_GET_CS,
            MON_JOB_BEGIN, // = 'T'
            MON_JOB_DONE,
            MON_LOCAL_JOB_BEGIN,
            MON_STATS,

            TRANFER_ENV, // = 'X'

            TEXT,
            STATUS_TEXT,
            GET_INTERNALS,

            // S --> CS, answered by LOGIN
            CS_CONF,

            // C --> CS, after installing an environment
            VERIFY_ENV,
            VERIFY_ENV_RESULT,
            // C --> CS, CS --> S (forwarded from C), to not use given host for given environment
            BLACKLIST_HOST_ENV
        };

        enum SpecialExits
        {
            CLIENT_WAS_WAITING_FOR_CS = 200
        };
    } // services
} // icecream

#endif /* !ICECREAM_COMM_H */
