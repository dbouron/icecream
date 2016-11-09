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

#ifndef ICECREAM_COMPILE_JOB_H
# define ICECREAM_COMPILE_JOB_H

# include <list>
# include <string>
# include <iostream>
# include <sstream>
# include <stdio.h>

# include "logging.h"
# include "exitcode.h"
# include "platform.h"

namespace icecream
{
    namespace services
    {
        enum ArgumentType : uint32_t
        {
            Unspecified,
            Local,
            Remote,
            Rest
        };

        enum Language : uint32_t
        {
            C,
            CXX,
            OBJC,
            Custom
        };

        enum Flag : uint32_t
        {
            None = 0,
            g = 0x1,
            g3 = 0x2,
            O = 0x4,
            O2 = 0x8,
            Ol2 = 0x10
        };

        /// Alias for argument list.
        using ArgumentList = std::list<std::pair<std::string, ArgumentType>>;

        class CompileJob
        {
        public:
            CompileJob()
                : m_id(0)
                , m_dwarf_fission(false)
                {
                    setTargetPlatform();
                }

            void setCompilerName(const std::string &name);
            std::string compilerName() const;

            void setLanguage(Language lg);
            Language language() const;

            void setCompilerPathname(const std::string& pathname);
            std::string compilerPathname() const;

            void setEnvironmentVersion(const std::string &ver);
            std::string environmentVersion() const;

            unsigned int argumentFlags() const;
            void setFlags(const ArgumentList &flags);


            void setInputFile(const std::string &file);
            std::string inputFile() const;

            void setOutputFile(const std::string &file);
            std::string outputFile() const;

            void setDwarfFissionEnabled(bool flag);
            bool dwarfFissionEnabled() const;

            void setWorkingDirectory(const std::string& dir);
            std::string workingDirectory() const;

            void setJobID(unsigned int id);
            unsigned int jobID() const;

            std::string targetPlatform() const;
            void setTargetPlatform(const std::string &_target);

            void appendFlag(std::string arg, ArgumentType argumentType);

            std::list<std::string> localFlags() const;
            std::list<std::string> remoteFlags() const;
            std::list<std::string> restFlags() const;
            std::list<std::string> allFlags() const;

        private:
            std::list<std::string> flags(ArgumentType argumentType) const;
            void setTargetPlatform();

            unsigned int m_id;
            Language m_language;
            std::string m_compiler_pathname;
            std::string m_compiler_name;
            std::string m_environment_version;
            ArgumentList m_flags;
            std::string m_input_file, m_output_file;
            std::string m_working_directory;
            std::string m_target_platform;
            bool m_dwarf_fission;
        };
    } // services
} // icecream

# include "job.hxx"

#endif
