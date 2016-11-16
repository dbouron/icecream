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

#ifndef ICECREAM_ARG_H
# define ICECREAM_ARG_H

# include <string>
# include <list>

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <assert.h>
# include <sys/types.h>
# include <sys/stat.h>

# include "config.h"

# include "local.h"
# include "remote.h"
# include "job.h"
# include "channel.h"

namespace icecream
{
    namespace client
    {
        bool analyse_argv(const char * const *argv,
                          services::CompileJob &job, bool icerun,
                          std::list<std::string> *extrafiles);
    } // client
} // icecream

#endif /* !ICECREAM_ARG_H */
