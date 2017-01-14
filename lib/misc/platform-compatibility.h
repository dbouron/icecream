/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Michael Matz <matz@suse.de>                                                                      2004 Stephan Kulow <coolo@suse.de>
                  2016 Dimitri Bouron <bouron.d@gmail.com>

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
 ** \file misc/platform_compatibility.h
 ** \brief Define is_platform_compatible function.
 */

#ifndef ICECREAM_PLATFORM_COMPATIBILITY_H
# define ICECREAM_PLATFORM_COMPATIBILITY_H

# include <string>
# include <map>

namespace misc
{
    /** Returns true if target architecture is compatible with host
     ** architecture.
     */
    bool is_platform_compatible(const std::string &host,
                                const std::string &target);
} // misc

#include <misc/platform-compatibility.hxx>

#endif /* !ICECREAM_PLATFORM_COMPATIBILITY_H */
