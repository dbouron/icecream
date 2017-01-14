/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

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

#ifndef ICECREAM_IGNORE_RESULT_HXX
# define ICECREAM_IGNORE_RESULT_HXX

# include <misc/ignore-result.h>

namespace misc
{
    template<typename T>
    inline T ignore_result(T x __attribute__((unused)))
    {
        return x;
    }
}

# include <misc/ignore-result.hxx>

#endif /* !ICECREAM_IGNORE_RESULT_HXX */
