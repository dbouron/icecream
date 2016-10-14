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


#ifndef ICECREAM_FILE_CHUNK_H
# define ICECREAM_FILE_CHUNK_H

# include <string>

# include "msg.h"

namespace icecream
{
    namespace services
    {
        class FileChunk : public Msg
        {
        public:
            FileChunk(unsigned char *_buffer, size_t _len)
                : Msg(MsgType::FILE_CHUNK)
                , buffer(_buffer)
                , len(_len)
                , del_buf(false)
                {
                }

            FileChunk()
                : Msg(MsgType::FILE_CHUNK)
                , buffer(0)
                , len(0)
                , del_buf(true)
                {
                }

            ~FileChunk();

            virtual void fill_from_channel(Channel *c);
            virtual void send_to_channel(Channel *c) const;

            unsigned char *buffer;
            size_t len;
            mutable size_t compressed;
            bool del_buf;

        private:
            FileChunk(const FileChunk &);
            FileChunk &operator=(const FileChunk &);
        };
    } // services
} // icecream

#endif /* !ICECREAM_FILE_CHUNK_H */
