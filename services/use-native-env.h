/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Michael Matz <matz@suse.de>
                  2004 Stephan Kulow <coolo@suse.de>
                  2007 Dirk Mueller <dmueller@suse.de>

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


#ifndef ICECREAM_USE_NATIVE_ENV_H
# define ICECREAM_USE_NATIVE_ENV_H

# include <string>
# include <list>

# include "msg.h"

namespace icecream
{
    namespace services
    {
        class UseNativeEnv : public Msg {
        public:
            UseNativeEnv()
                : Msg(MsgType::NATIVE_ENV) {
            }

            UseNativeEnv(std::string _native)
                : Msg(MsgType::NATIVE_ENV), nativeVersion(_native) {
            }

            virtual void fill_from_channel(Channel *c);
            virtual void send_to_channel(Channel *c) const;

            std::string nativeVersion;
        };
    } // services
} // icecream

#endif /* !ICECREAM_USE_NATIVE_ENV_H */
