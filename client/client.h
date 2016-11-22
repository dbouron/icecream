/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of icecc.

    Copyright (C) 2002, 2003 by Martin Pool <mbp@samba.org>
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

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <services/job.h>
#include <services/comm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdexcept>

#include "services/exitcode.h"
#include "services/logging.h"
#include "services/util.h"
#include "services/service.h"
#include "services/channel.h"
#include "services/protocol.h"

using namespace icecream::services;

namespace icecream
{
    namespace client
    {
        class client_error :  public std::runtime_error
        {
        public:
            client_error(int code, const std::string& what)
                : std::runtime_error(what)
                , errorCode(code)
                {}

            const int errorCode;
        };

        class remote_error : public client_error
        {
        public:
            remote_error(int code, const std::string& what)
                : client_error(code, what)
                {}
        };
    } // client
} // icecream

#endif
