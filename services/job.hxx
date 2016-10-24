/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
  This file is part of Icecream.

  Copyright (c) 2004-2014 Stephan Kulow <coolo@suse.de>

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

#ifndef ICECREAM_COMPILE_JOB_HXX
# define ICECREAM_COMPILE_JOB_HXX

# include <iterator>

# include "job.h"

namespace icecream
{
    namespace services
    {
        inline
        void appendList(std::list<std::string> &list,
                        const std::list<std::string> &toadd)
        {
            // Cannot splice since toadd is a reference-to-const
            list.insert(list.end(), toadd.begin(), toadd.end());
        }

        inline
        std::ostream &operator<<(std::ostream &output,
                                 const Language &l)
        {
            switch (l)
            {
            case Language::CXX:
                output << "C++";
                break;
            case Language::C:
                output << "C";
                break;
            case Language::Custom:
                output << "<custom>";
                break;
            case Language::OBJC:
                output << "ObjC";
                break;
            }
            return output;
        }

        inline
        std::string concat_args(const std::list<std::string> &args)
        {
            std::stringstream str;
            str << "'";

            for (auto cit = args.cbegin(); cit != args.cend(); ++cit)
            {
                str << *cit;
                if (std::next(cit) != args.cend())
                    str << ", ";
            }
            str << "'";
            return str.str();
        }
    }
}

#endif /* !ICECREAM_COMPILE_JOB_HXX */
