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

#ifndef ICECREAM_CPP_H
# define ICECREAM_CPP_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <assert.h>

# include "config.h"
# include "client/local.h"
# include "client/safeguard.h"
# include "client/util.h"
# include "services/job.h"

namespace icecream
{
    namespace client
    {
        pid_t call_cpp(services::CompileJob &job, int fdwrite, int fdread = -1);
    } // client
} // icecream


#endif /* !ICECREAM_CPP_H */
