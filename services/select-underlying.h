/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
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
 ** \file services/select-underlying.h
 **
 ** \brief Select between an underlying type, if \a T is a complete
 ** enumeration type, or an original type.
 ** It permits to have a defined behaviour in case of underlying_type
 ** \a T is not an enum.
 */

#ifndef ICECREAM_SELECT_UNDERLYING_H
# define ICECREAM_SELECT_UNDERLYING_H

# include <type_traits>

namespace icecream
{
    namespace services
    {
        /**
         ** \struct underlying_traits
         ** \brief Provides a member alias \c type that names the
         ** underlying type of \a T if and only if \a T is an enum.
         **
         ** \tparam T - type to select underlying type
         */
        template <typename T, typename = typename std::is_enum<T>::type>
        struct underlying_traits
        {
            using type = std::underlying_type_t<T>;
        };

        /**
         ** \struct underlying_traits
         ** \brief Partial template specialization of \c underlying_traits
         ** for types which are not an enum. \c type is an alias of \a T.
         */
        template <typename T>
        struct underlying_traits<T, std::false_type>
        {
            using type = T;
        };

        /// An useful helper for underlying type acces.
        template <typename T>
        using underlying_traits_t = typename underlying_traits<T>::type;
    } // services
} // icecream

#endif /* !ICECREAM_SELECT_UNDERLYING_H */
