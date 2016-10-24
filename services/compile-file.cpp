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


#include "compile-file.h"

namespace icecream
{
    namespace services
    {

        CompileFile::CompileFile(CompileJob *j, bool delete_job)
            : Msg(MsgType::COMPILE_FILE)
            , deleteit(delete_job)
            , job(j)
        {
        }

        CompileFile::~CompileFile()
        {
            if (deleteit)
            {
                delete job;
            }
        }

        void CompileFile::fill_from_channel(std::shared_ptr<Channel> c)
        {
            Msg::fill_from_channel(c);
            uint32_t id, lang;
            std::list<std::string> _l1, _l2;
            std::string version;
            *c >> lang;
            *c >> id;
            *c >> _l1;
            *c >> _l2;
            *c >> version;
            job->setLanguage((Language) lang);
            job->setJobID(id);
            ArgumentList l;

            for (const auto &cit : _l1)
            {
                l.push_back(std::make_pair(cit, ArgumentType::Remote));
            }

            for (const auto &cit : _l2)
            {
                l.push_back(std::make_pair(cit, ArgumentType::Rest));
            }

            job->setFlags(l);
            job->setEnvironmentVersion(version);

            std::string target;
            *c >> target;
            job->setTargetPlatform(target);

            if (is_protocol<30>()(*c))
            {
                std::string compilerName;
                *c >> compilerName;
                job->setCompilerName(compilerName);
            }
            if (is_protocol<34>()(*c))
            {
                std::string inputFile;
                std::string workingDirectory;
                *c >> inputFile;
                *c >> workingDirectory;
                job->setInputFile(inputFile);
                job->setWorkingDirectory(workingDirectory);
            }
            if (is_protocol<35>()(*c))
            {
                std::string outputFile;
                uint32_t dwarfFissionEnabled = 0;
                *c >> outputFile;
                *c >> dwarfFissionEnabled;
                job->setOutputFile(outputFile);
                job->setDwarfFissionEnabled(dwarfFissionEnabled);
            }
        }

        void CompileFile::send_to_channel(std::shared_ptr<Channel> c) const
        {
            Msg::send_to_channel(c);
            *c << (uint32_t) job->language();
            *c << job->jobID();

            if (is_protocol<30>()(*c))
            {
                *c << job->remoteFlags();
            }
            else
            {
                if (job->compilerName().find("clang") != std::string::npos)
                {
                    // Hack for compilerwrapper.
                    std::list <std::string> flags = job->remoteFlags();
                    flags.push_front("clang");
                    *c << flags;
                }
                else
                {
                    *c << job->remoteFlags();
                }
            }

            *c << job->restFlags();
            *c << job->environmentVersion();
            *c << job->targetPlatform();

            if (is_protocol<30>()(*c))
            {
                *c << remote_compiler_name();
            }
            if (is_protocol<34>()(*c))
            {
                *c << job->inputFile();
                *c << job->workingDirectory();
            }
            if (is_protocol<35>()(*c))
            {
                *c << job->outputFile();
                *c << (uint32_t) job->dwarfFissionEnabled();
            }
        }

        // Environments created by icecc-create-env always use the same binary name
        // for compilers, so even if local name was e.g. c++, remote needs to
        // be g++ (before protocol version 30 remote CS even had /usr/bin/{gcc|g++}
        // hardcoded).  For clang, the binary is just clang for both C/C++.
        std::string CompileFile::remote_compiler_name() const
        {
            if (job->compilerName().find("clang") != std::string::npos)
            {
                return "clang";
            }

            return job->language() == Language::CXX ? "g++" : "gcc";
        }

        CompileJob *CompileFile::takeJob()
        {
            assert(deleteit);
            deleteit = false;
            return job;
        }
    } // services
} // icecream
