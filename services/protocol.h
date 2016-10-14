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
 ** \file services/protocol.h
 ** \brief Define is_protocol functor.
 */

#ifndef ICECREAM_PROTOCOL_H
# define ICECREAM_PROTOCOL_H

# include <type_traits>

namespace iceream
{
    namespace services
    {
        /// Actual protocol version.
        constexpr uint32_t PROTOCOL_VERSION = 35;
        /// Minimal version required.
        constexpr uint32_t MIN_PROTOCOL_VERSION = 21;

        /**
         ** \struct is_protocol
         **
         ** \brief \c is_protocol functor replace IS_PROTOCOL_XX macro.
         ** It uses SFINAE concept to constrain minimal protocol version and
         ** to clean up the code if needed.
         **
         ** \tparam N - Protocol version to compare with actual
         **
         ** \note
         ** We could replace
         **
         ** \code{.cpp}
         ** template <uint32_t N>
         ** \endcode
         **
         ** by
         **
         ** \code{.cpp}
         ** template <uint32_t N, typename =
         **           typename std::enable_if<MIN_PROTOCOL_VERSION <= N>::type>
         ** \endcode
         **
         ** instead of using \a bool_t, but some programmer can bypass this
         ** constraint with:
         **
         ** \code{.cpp}
         ** is_protocol<MIN_PROTOCOL_VERSION - 1, std::true_type>()()
         ** \endcode
         **
         ** On the other hand, it could be useful for making exemption.
         */
        template <uint32_t N>
        struct is_protocol
        {
            /// Return type enabled if and only if N >= MIN_PROTOCOL_VERSION.
            using bool_t =
                typename std::enable_if<MIN_PROTOCOL_VERSION <= N, bool>::type;

            bool_t operator(const MsgChannel &) const;
        };
    } // services
} // icecream

#include <protocol.hxx>

#endif /* !ICECREAM_PROTOCOL_H */
