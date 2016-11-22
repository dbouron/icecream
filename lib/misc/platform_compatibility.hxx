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
 ** \file misc/platform_compatibility.hxx
 ** \brief Implement is_platform_compatible function.
 */

#ifndef ICECREAM_PLATFORM_COMPATIBILITY_HXX
# define ICECREAM_PLATFORM_COMPATIBILITY_HXX

# include <misc/platform_compatibility.h>

namespace misc
{
    inline
    bool is_platform_compatible(const std::string &host,
                                const std::string &target)
    {
        if (target == host) {
            return true;
        }

        // the below doesn't work as the unmapped platform is transferred back to the
        // client and that asks the daemon for a platform he can't install (see TODO)
        static const std::multimap<std::string, std::string> platform_map
        {
            {"i386", "i486"},
            {"i386", "i586"},
            {"i386", "i686"},
            {"i386", "x86_64"},

            {"i486", "i586"},
            {"i486", "i686"},
            {"i486", "x86_64"},

            {"i586", "i686"},
            {"i586", "x86_64"},

            {"i686", "x86_64"},

            {"ppc", "ppc64"},
            {"s390", "s390x"},
                };

        std::multimap<std::string, std::string>::const_iterator end = platform_map.upper_bound(target);

        for (std::multimap<std::string, std::string>::const_iterator it = platform_map.lower_bound\
                 (target);
             it != end;
             ++it) {
            if (it->second == host) {
                return true;
            }
        }
        return false;
    }
} // misc

#endif /* !ICECREAM_PLATFORM_COMPATIBILITY_HXX */
