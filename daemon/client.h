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

#ifndef ICECREAM_CLIENT_H
# define ICECREAM_CLIENT_H

# include <string>
# include <map>

# include "channel.h"
# include "all.h"

using namespace icecream::services;

namespace icecream
{
    namespace daemon
    {
        /**
         ** UNKNOWN: Client was just created - not supposed to be long term
         ** GOTNATIVE: Client asked us for the native env - this is the first step
         ** PENDING_USE_CS: We have a CS from scheduler and need to tell the client
         **          as soon as there is a spot available on the local machine
         ** JOBDONE: This was compiled by a local client and we got a jobdone - awaiting END
         ** LINKJOB: This is a local job (aka link job) by a local client we told the scheduler about
         **          and await the finish of it
         ** TOINSTALL: We're receiving an environment transfer and wait for it to complete.
         ** TOCOMPILE: We're supposed to compile it ourselves
         ** WAITFORCS: Client asked for a CS and we asked the scheduler - waiting for its answer
         ** WAITCOMPILE: Client got a CS and will ask him now (it's not me)
         ** CLIENTWORK: Client is busy working and we reserve the spot (job_id is set if it's a scheduler job)
         ** WAITFORCHILD: Client is waiting for the compile job to finish.
         ** WAITCREATEENV: We're waiting for icecc-create-env to finish.
         */
        enum Status : uint32_t
        {
            UNKNOWN,
            GOTNATIVE,
            PENDING_USE_CS,
            JOBDONE,
            LINKJOB,
            TOINSTALL,
            TOCOMPILE,
            WAITFORCS,
            WAITCOMPILE,
            CLIENTWORK,
            WAITFORCHILD,
            WAITCREATEENV,
            LASTSTATE = WAITCREATEENV
        };

        class Client
        {
        public:
            Client();
            ~Client();

            static std::string status_str(Status status);

            std::string dump() const;

            uint32_t job_id;
            // only useful for LINKJOB or TOINSTALL
            std::string outfile;
            std::shared_ptr<services::Channel> channel;
            services::UseCS *usecsmsg;
            std::shared_ptr<services::CompileJob> job;
            int client_id;
            // pipe to child process, only valid if WAITFORCHILD or TOINSTALL
            int pipe_to_child;
            pid_t child_pid;
            // only for WAITCREATEENV
            std::string pending_create_env;
            Status status;
        };

        class Clients : public std::map<Channel*, Client*>
        {
        public:
            Clients() {
                active_processes = 0;
            }
            unsigned int active_processes;

            Client *find_by_client_id(int id) const {
                for (const_iterator it = begin(); it != end(); ++it)
                    if (it->second->client_id == id) {
                        return it->second;
                    }

                return nullptr;
            }

            Client *find_by_channel(Channel *c) const {
                const_iterator it = find(c);

                if (it == end()) {
                    return nullptr;
                }

                return it->second;
            }

            Client *find_by_pid(pid_t pid) const {
                for (const_iterator it = begin(); it != end(); ++it)
                    if (it->second->child_pid == pid) {
                        return it->second;
                    }

                return nullptr;
            }

            Client *first() {
                iterator it = begin();

                if (it == end()) {
                    return nullptr;
                }

                Client *cl = it->second;
                return cl;
            }

            std::string dump_status(Status s) const {
                int count = 0;

                for (const_iterator it = begin(); it != end(); ++it) {
                    if (it->second->status == s) {
                        count++;
                    }
                }

                if (count) {
                    return toString(count) + " " + Client::status_str(s) + ", ";
                }

                return std::string();
            }

            std::string dump_per_status() const {
                std::string s;

                for (auto i = Status::UNKNOWN; i <= Status::LASTSTATE;
                     i = Status(int(i) + 1))
                {
                    s += dump_status(i);
                }

                return s;
            }

            Client *get_earliest_client(Status s) const {
                // TODO: possibly speed this up in adding some sorted lists
                Client *client = nullptr;
                int min_client_id = 0;

                for (const_iterator it = begin(); it != end(); ++it)
                {
                    if (it->second->status == s && (!min_client_id || min_client_id > it->second->client_id))
                    {
                        client = it->second;
                        min_client_id = client->client_id;
                    }
                }

                return client;
            }
        };
    } // daemon
} // icecream

#endif /* !ICECREAM_CLIENT_H */
