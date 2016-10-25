/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/*
    This file is part of Icecream.

    Copyright (c) 2016 Dimitri Bouron <bouron.d@gmail.com>

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

/**
 ** \file services/all.h
 ** \brief Export all Msg header files.
 */

#ifndef ICECREAM_SERVICES_ALL_H
# define ICECREAM_SERVICES_ALL_H

# include "msg.h"
# include "ping.h"
# include "end.h"
# include "get-cs.h"
# include "use-cs.h"
# include "get-native-env.h"
# include "use-native-env.h"
# include "compile-file.h"
# include "file-chunk.h"
# include "compile-result.h"
# include "job-begin.h"
# include "job-done.h"
# include "job-local-begin.h"
# include "job-local-done.h"
# include "login.h"
# include "conf-cs.h"
# include "stats.h"
# include "env-transfer.h"
# include "get-internal-status.h"
# include "mon-login.h"
# include "mon-get-cs.h"
# include "mon-job-begin.h"
# include "mon-job-done.h"
# include "mon-local-job-begin.h"
# include "mon-stats.h"
# include "text.h"
# include "status-text.h"
# include "verify-env.h"
# include "verify-env-result.h"
# include "blacklist-host-env.h"

#endif /* !ICECREAM_SERVICES_ALL_H */
