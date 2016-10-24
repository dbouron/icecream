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

#include "job.h"

namespace icecream
{
    namespace services
    {
        std::list<std::string> CompileJob::flags(ArgumentType argumentType) const
        {
            std::list<std::string> args;

            for (const auto &cit : m_flags)
            {
                if (cit.second == argumentType)
                {
                    args.push_back(cit.first);
                }
            }

            return args;
        }

        std::list<std::string> CompileJob::localFlags() const
        {
            return flags(ArgumentType::Local);
        }

        std::list<std::string> CompileJob::remoteFlags() const
        {
            return flags(ArgumentType::Remote);
        }

        std::list<std::string> CompileJob::restFlags() const
        {
            return flags(ArgumentType::Rest);
        }

        std::list<std::string> CompileJob::allFlags() const
        {
            std::list<std::string> args;

            for (const auto &cit : m_flags)
            {
                args.push_back(cit.first);
            }

            return args;
        }

        void CompileJob::setTargetPlatform()
        {
            m_target_platform = determine_platform();
        }

        unsigned int CompileJob::argumentFlags() const
        {
            unsigned int result = static_cast<unsigned int>(Flag::None);

            for (const auto &cit : m_flags)
            {
                const auto arg = cit.first;

                if (arg.at(0) == '-') {
                    if (arg.length() == 1) {
                        continue;
                    }

                    if (arg.at(1) == 'g') {
                        if (arg.length() > 2 && arg.at(2) == '3') {
                            result &= ~Flag::g;
                            result |= Flag::g3;
                        } else {
                            result &= ~Flag::g3;
                            result |= Flag::g;
                        }
                    } else if (arg.at(1) == 'O') {
                        result &= ~(Flag::O | Flag::O2 | Flag::Ol2);

                        if (arg.length() == 2) {
                            result |= Flag::O;
                        } else {
                            assert(arg.length() > 2);

                            if (arg.at(2) == '2') {
                                result |= Flag::O2;
                            } else if (arg.at(2) == '1') {
                                result |= Flag::O;
                            } else if (arg.at(2) != '0') {
                                result |= Flag::Ol2;
                            }
                        }
                    }
                }
            }

            return result;
        }
    } // services
} // icecream
