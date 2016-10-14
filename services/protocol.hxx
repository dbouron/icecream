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
 ** \file services/protocol.hxx
 ** \brief Implement is_protocol templated method.
 */

#ifndef ICECREAM_PROTOCOL_H
# define ICECREAM_PROTOCOL_H

namespace icecream
{
    namespace services
    {
        template <uint32_t N>
        inline
        bool_t is_protocol<N>::operator()(const Channel &c) const
        {
            return c->protocol >= N;
        }
    } // services
} // icecream

#endif /* !ICECREAM_PROTOCOL_H */
