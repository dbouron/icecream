/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Stephan Kulow <coolo@suse.de>
                  2002, 2003 by Martin Pool <mbp@samba.org>

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

#include "client.h"

using namespace icecream::services;

namespace icecream
{
    namespace daemon
    {
        Client::Client()
            : job_id(0)
            , channel(nullptr)
            , usecsmsg(nullptr)
            , job(nullptr)
            , client_id(0)
            , pipe_to_child(-1)
            , child_pid(-1)
            , status(UNKNOWN)
        {
        }

        Client::~Client()
        {
            status = static_cast<Status>(-1);
            channel = nullptr;
            usecsmsg = nullptr;
            job = nullptr;

            if (pipe_to_child >= 0)
            {
                close(pipe_to_child);
            }
        }

        std::string Client::status_str(Status status)
        {
            switch (status)
            {
            case UNKNOWN:
                return "unknown";
            case GOTNATIVE:
                return "gotnative";
            case PENDING_USE_CS:
                return "pending_use_cs";
            case JOBDONE:
                return "jobdone";
            case LINKJOB:
                return "linkjob";
            case TOINSTALL:
                return "toinstall";
            case TOCOMPILE:
                return "tocompile";
            case WAITFORCS:
                return "waitforcs";
            case CLIENTWORK:
                return "clientwork";
            case WAITCOMPILE:
                return "waitcompile";
            case WAITFORCHILD:
                return "waitforchild";
            case WAITCREATEENV:
                return "waitcreateenv";
            }

            assert(false);
            return std::string(); // shutup gcc
        }

        std::string Client::dump() const
        {
            std::string ret = status_str(status) + " " + channel->dump();

            switch (status)
            {
            case LINKJOB:
                return ret + " CID: " + toString(client_id) + " " + outfile;
            case TOINSTALL:
                return ret + " " + toString(client_id) + " " + outfile;
            case WAITFORCHILD:
                return ret + " CID: " + toString(client_id) + " PID: " + toString(child_pid) + " PFD: " + toString(pipe_to_child);
            case WAITCREATEENV:
                return ret + " " + toString(client_id) + " " + pending_create_env;
            default:

                if (job_id)
                {
                    std::string jobs;

                    if (usecsmsg)
                    {
                        jobs = " CS: " + usecsmsg->hostname;
                    }

                    return ret + " CID: " + toString(client_id) + " ID: " + toString(job_id) + jobs;
                }
                else
                {
                    return ret + " CID: " + toString(client_id);
                }
            }

            return ret;
        }
    } // daemon
} // icecream
