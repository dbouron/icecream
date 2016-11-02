/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Stephan Kulow <coolo@suse.de>

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

#ifndef ICECREAM_LOAD_H
# define ICECREAM_LOAD_H

# include <unistd.h>
# include <stdio.h>
# include <math.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif

# ifdef HAVE_MACH_HOST_INFO_H
#  define USE_MACH 1
# elif !defined( __linux__ ) && !defined(__CYGWIN__)
#  define USE_SYSCTL
# endif

# ifdef USE_MACH
#  include <mach/host_info.h>
#  include <mach/mach_host.h>
#  include <mach/mach_init.h>
# endif

# ifdef HAVE_KINFO_H
#  include <kinfo.h>
# endif

# ifdef HAVE_DEVSTAT_H
#  include <sys/resource.h>
#  include <sys/sysctl.h>
#  include <devstat.h>
# endif

# include "config.h"
# include "stats.h"
# include <comm.h>
# include <logging.h>

namespace icecream
{
    namespace daemon
    {

        /// What the kernel puts as ticks in /proc/stat.
        using load_t = unsigned long long;

        // 'hint' is used to approximate the load, whenever getloadavg() is unavailable.
        bool fill_stats(unsigned long &myidleload,
                        unsigned long &myniceload,
                        unsigned int &memory_fillgrade,
                        icecream::services::Stats *msg,
                        unsigned int hint);
    } // daemon
} // icecream
#endif /* !ICECREAM_LOAD_H */
