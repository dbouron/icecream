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


#ifndef ICECREAM_JOB_DONE_H
# define ICECREAM_JOB_DONE_H

namespace icecream
{
    namespace services
    {
        class JobDone : public Msg {
        public:
            /* FROM_SERVER: this message was generated by the daemon responsible
               for remotely compiling the job (i.e. job->server).
               FROM_SUBMITTER: this message was generated by the daemon connected
               to the submitting client.  */
            enum from_type {
                FROM_SERVER = 0, FROM_SUBMITTER = 1
            };

            JobDone(int job_id = 0, int exitcode = -1,
                    unsigned int flags = FROM_SERVER);

            void set_from(from_type from) {
                flags |= (uint32_t) from;
            }

            bool is_from_server() {
                return (flags & FROM_SUBMITTER) == 0;
            }

            virtual void fill_from_channel(Channel *c);
            virtual void send_to_channel(Channel *c) const;

            uint32_t real_msec; /* real time it used */
            uint32_t user_msec; /* user time used */
            uint32_t sys_msec; /* system time used */
            uint32_t pfaults; /* page faults */

            int exitcode; /* exit code */

            uint32_t flags;

            uint32_t in_compressed;
            uint32_t in_uncompressed;
            uint32_t out_compressed;
            uint32_t out_uncompressed;

            uint32_t job_id;
        };

    } // services
} // icecream

#endif /* !ICECREAM_JOB_DONE_H */
