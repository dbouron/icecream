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
        void CompileJob::setCompilerName(const std::string &name)
        {
            m_compiler_name = name;
        }

        inline
        std::string CompileJob::compilerName() const
        {
            return m_compiler_name;
        }

        inline
        void CompileJob::setLanguage(Language lg)
        {
            m_language = lg;
        }

        inline
        Language CompileJob::language() const
        {
            return m_language;
        }

        inline
        void CompileJob::setCompilerPathname(const std::string& pathname)
        {
            m_compiler_pathname = pathname;
        }

        inline
        std::string CompileJob::compilerPathname() const
        {
            return m_compiler_pathname;
        }

        inline
        void CompileJob::setEnvironmentVersion(const std::string &ver)
        {
            m_environment_version = ver;
        }

        inline
        std::string CompileJob::environmentVersion() const
        {
            return m_environment_version;
        }

        inline
        void CompileJob::setFlags(const ArgumentList &flags)
        {
            m_flags = flags;
        }

        inline
        void CompileJob::setInputFile(const std::string &file)
        {
            m_input_file = file;
        }

        inline
        std::string CompileJob::inputFile() const
        {
            return m_input_file;
        }

        inline
        void CompileJob::setOutputFile(const std::string &file)
        {
            m_output_file = file;
        }

        inline
        std::string CompileJob::outputFile() const
        {
            return m_output_file;
        }

        inline
        void CompileJob::setDwarfFissionEnabled(bool flag)
        {
            m_dwarf_fission = flag;
        }

        inline
        bool CompileJob::dwarfFissionEnabled() const
        {
            return m_dwarf_fission;
        }

        inline
        void CompileJob::setWorkingDirectory(const std::string& dir)
        {
            m_working_directory = dir;
        }

        inline
        std::string CompileJob::workingDirectory() const
        {
            return m_working_directory;
        }

        inline
        void CompileJob::setJobID(unsigned int id)
        {
            m_id = id;
        }

        inline
        unsigned int CompileJob::jobID() const
        {
            return m_id;
        }

        inline
        std::string CompileJob::targetPlatform() const
        {
            return m_target_platform;
        }

        inline
        void CompileJob::setTargetPlatform(const std::string &_target)
        {
            m_target_platform = _target;
        }

        inline
        void CompileJob::appendFlag(std::string arg, ArgumentType argumentType)
        {
            m_flags.push_back(std::make_pair(arg, argumentType));
        }

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
