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

/** \todo
 ** + buffered in/output per Channel
 **    + move read* into Channel, create buffer-fill function
 **    + add timeouting select() there, handle it in the different
 **    + read* functions.
 **    + write* unbuffered / or per message buffer (flush in send_msg)
 ** + think about error handling
 **    + saving errno somewhere (in Channel class)
 ** + handle unknown messages (implement a Unknown holding the content
 **   of the whole data packet?)
 */

#ifndef ICECREAM_CHANNEL_H
# define ICECREAM_CHANNEL_H

# include "comm.h"
# include "msg.h"

namespace icecream
{
    namespace services
    {
        class Msg;

        enum SendFlags : uint32_t
        {
            SendBlocking = 1 << 0,
            SendNonBlocking = 1 << 1,
            SendBulkOnly = 1 << 2
        };

        enum InState : uint32_t
        {
            NEED_PROTO,
            NEED_LEN,
            FILL_BUF,
            HAS_MSG
        };

        class Channel
        {
        public:
            virtual ~Channel();

            void setBulkTransfer();

            std::string dump() const;
            // NULL  <--> channel closed or timeout
            Msg *get_msg(int timeout = 10);

            // false <--> error (msg not send)
            bool send_msg(const Msg &, SendFlags flags = SendFlags::SendBlocking);

            bool has_msg(void) const {
                return eof || instate == InState::HAS_MSG;
            }

            bool read_a_bit(void);

            bool at_eof(void) const {
                return instate != InState::HAS_MSG && eof;
            }

            bool is_text_based(void) const {
                return text_based;
            }

            void readcompressed(unsigned char **buf, size_t &_uclen, size_t &_clen);
            void writecompressed(const unsigned char *in_buf, size_t _in_len,
                                 size_t &_out_len);
            void write_environments(const Environments &envs);
            void read_environments(Environments &envs);
            void read_line(std::string &line);
            void write_line(const std::string &line);

            bool eq_ip(const Channel &s) const;

            Channel &operator>>(uint32_t &);
            Channel &operator>>(std::string &);
            Channel &operator>>(std::list<std::string> &);

            Channel &operator<<(uint32_t);
            Channel &operator<<(const std::string &);
            Channel &operator<<(const std::list<std::string> &);

            // our filedesc
            int fd;

            // the minimum protocol version between me and him
            int protocol;

            std::string name;
            time_t last_talk;

        protected:
            Channel(int _fd, struct sockaddr *, socklen_t, bool text = false);

            bool wait_for_protocol();
            // returns false if there was an error sending something
            bool flush_writebuf(bool blocking);
            void writefull(const void *_buf, size_t count);
            // returns false if there was an error in the protocol setup
            bool update_state(void);
            void chop_input(void);
            void chop_output(void);
            bool wait_for_msg(int timeout);

            char *msgbuf;
            size_t msgbuflen;
            size_t msgofs;
            size_t msgtogo;
            char *inbuf;
            size_t inbuflen;
            size_t inofs;
            size_t intogo;

            InState instate;

            uint32_t inmsglen;
            bool eof;
            bool text_based;

        private:
            friend class Service;

            // deep copied
            struct sockaddr *addr;
            socklen_t addr_len;
        };
    } // services
} // icecream

#endif /* !ICECREAM_CHANNEL_H */
