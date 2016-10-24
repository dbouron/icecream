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


#include "channel.h"

namespace icecream
{
    namespace services
    {
        Channel::Channel(int _fd, struct sockaddr *_a, socklen_t _l, bool text)
            : fd(_fd)
        {
            addr_len = _l;

            if (addr_len && _a)
            {
                addr = static_cast<struct sockaddr*>(malloc(addr_len));
                memcpy(addr, _a, addr_len);
                char buf[16384] = "";
                if (addr->sa_family == AF_UNIX)
                    name = reinterpret_cast<sockaddr_un*>(addr)->sun_path;
                else
                {
                    if (int error = getnameinfo(addr, addr_len, buf, sizeof(buf), NULL,
                            0, NI_NUMERICHOST))
                        log_error() << "getnameinfo(): " << error << endl;
                    name = buf;
                }
            }
            else
            {
                addr = 0;
                name = "";
            }

            // not using new/delete because of the need of realloc()
            msgbuf = static_cast<char*>(malloc(128));
            msgbuflen = 128;
            msgofs = 0;
            msgtogo = 0;
            inbuf = static_cast<char*>(malloc(128));
            inbuflen = 128;
            inofs = 0;
            intogo = 0;
            eof = false;
            text_based = text;

            int on = 1;

            if (!setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on)))
            {
        #if defined( TCP_KEEPIDLE )
                int keepidle = TCP_KEEPIDLE;
        #else
                int keepidle = TCPCTL_KEEPIDLE;
        #endif

                int sec;
                sec = MAX_SCHEDULER_PING - 3 * MAX_SCHEDULER_PONG;
                setsockopt(_fd, IPPROTO_TCP, keepidle, (char *) &sec, sizeof(sec));

        #if defined( TCP_KEEPINTVL )
                int keepintvl = TCP_KEEPINTVL;
        #else
                int keepintvl = TCPCTL_KEEPINTVL;
        #endif

                sec = MAX_SCHEDULER_PONG;
                setsockopt(_fd, IPPROTO_TCP, keepintvl, (char *) &sec, sizeof(sec));

        #ifdef TCP_KEEPCNT
                sec = 3;
                setsockopt(_fd, IPPROTO_TCP, TCP_KEEPCNT, (char *) &sec, sizeof(sec));
        #endif
            }

            if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
            {
                log_perror("Channel fcntl()");
            }

            if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
            {
                log_perror("Channel fcntl() 2");
            }

            if (text_based)
            {
                instate = NEED_LEN;
                protocol = PROTOCOL_VERSION;
            }
            else
            {
                instate = NEED_PROTO;
                protocol = -1;
                unsigned char vers[4] = { PROTOCOL_VERSION, 0, 0, 0 };
                //writeuint32 ((uint32_t) PROTOCOL_VERSION);
                writefull(vers, 4);

                if (!flush_writebuf(true))
                {
                    protocol = 0;    // unusable
                }
            }

            last_talk = time(0);
        }

        Channel::~Channel()
        {
            if (fd >= 0)
            {
                close (fd);
            }

            fd = -1;

            if (msgbuf)
            {
                free (msgbuf);
            }

            if (inbuf)
            {
                free (inbuf);
            }

            if (addr)
            {
                free (addr);
            }
        }

        /* Tries to fill the inbuf completely.  */
        bool Channel::read_a_bit()
        {
            chop_input();
            size_t count = inbuflen - inofs;

            if (count < 128)
            {
                inbuflen = (inbuflen + 128 + 127) & ~static_cast<size_t>(127);
                inbuf = static_cast<char*>(realloc(inbuf, inbuflen));
                count = inbuflen - inofs;
            }

            char *buf = inbuf + inofs;
            bool error = false;

            while (count)
            {
                if (eof)
                {
                    break;
                }

                ssize_t ret = read(fd, buf, count);
                if (ret > 0)
                {
                    count -= ret;
                    buf += ret;
                }
                else if (ret < 0 && errno == EINTR)
                {
                    continue;
                }
                else if (ret < 0)
                {
                    // EOF or some error
                    if (errno != EAGAIN)
                    {
                        error = true;
                    }
                }
                else if (ret == 0)
                {
                    eof = true;
                }

                break;
            }

            inofs = buf - inbuf;

            if (!update_state())
            {
                error = true;
            }

            return !error;
        }

        bool Channel::update_state(void)
        {
            switch (instate)
            {
            case NEED_PROTO:

                while (inofs - intogo >= 4)
                {
                    if (protocol == 0)
                    {
                        return false;
                    }

                    uint32_t remote_prot = 0;
                    unsigned char vers[4];
                    //readuint32 (remote_prot);
                    memcpy(vers, inbuf + intogo, 4);
                    intogo += 4;

                    for (int i = 0; i < 4; ++i)
                    {
                        remote_prot |= vers[i] << (i * 8);
                    }

                    if (protocol == -1)
                    {
                        /* The first time we read the remote protocol.  */
                        protocol = 0;

                        if (remote_prot < MIN_PROTOCOL_VERSION
                                || remote_prot > (1 << 20))
                        {
                            remote_prot = 0;
                            return false;
                        }

                        if (remote_prot > PROTOCOL_VERSION)
                        {
                            remote_prot = PROTOCOL_VERSION;    // ours is smaller
                        }

                        for (int i = 0; i < 4; ++i)
                        {
                            vers[i] = remote_prot >> (i * 8);
                        }

                        writefull(vers, 4);

                        if (!flush_writebuf(true))
                        {
                            return false;
                        }

                        protocol = -1 - remote_prot;
                    }
                    else if (protocol < -1)
                    {
                        /* The second time we read the remote protocol.  */
                        protocol = -(protocol + 1);

                        if (static_cast<int>(remote_prot) != protocol)
                        {
                            protocol = 0;
                            return false;
                        }

                        instate = NEED_LEN;
                        /* Don't consume bytes from messages.  */
                        break;
                    }
                    else
                    {
                        trace() << "NEED_PROTO but protocol > 0" << endl;
                    }
                }

                /* FALLTHROUGH if the protocol setup was complete (instate was changed
                 to NEED_LEN then).  */
                if (instate != NEED_LEN)
                {
                    break;
                }

            case NEED_LEN:

                if (text_based)
                {
                    // Skip any leading whitespace
                    for (; inofs < intogo; ++inofs)
                        if (inbuf[inofs] >= ' ')
                        {
                            break;
                        }

                    // Skip until next newline
                    for (inmsglen = 0; inmsglen < inofs - intogo; ++inmsglen)
                        if (inbuf[intogo + inmsglen] < ' ')
                        {
                            instate = HAS_MSG;
                            break;
                        }

                    break;
                }
                else if (inofs - intogo >= 4)
                {
                    (*this) >> inmsglen;

                    if (inmsglen > MAX_MSG_SIZE)
                    {
                        return false;
                    }

                    if (inbuflen - intogo < inmsglen)
                    {
                        inbuflen = (inmsglen + intogo + 127)
                            & ~static_cast<size_>(127);
                        inbuf = static_cast<char*>(realloc(inbuf, inbuflen));
                    }

                    instate = FILL_BUF;
                    /* FALLTHROUGH */
                }
                else
                {
                    break;
                }

            case FILL_BUF:

                if (inofs - intogo >= inmsglen)
                {
                    instate = HAS_MSG;
                }
                /* FALLTHROUGH */
                else
                {
                    break;
                }

            case HAS_MSG:
                /* handled elsewere */
                break;
            }

            return true;
        }

        void Channel::chop_input()
        {
            /* Make buffer smaller, if there's much already read in front
             of it, or it is cheap to do.  */
            if (intogo > 8192 || inofs - intogo <= 16)
            {
                if (inofs - intogo != 0)
                {
                    memmove(inbuf, inbuf + intogo, inofs - intogo);
                }

                inofs -= intogo;
                intogo = 0;
            }
        }

        void Channel::chop_output()
        {
            if (msgofs > 8192 || msgtogo <= 16)
            {
                if (msgtogo)
                {
                    memmove(msgbuf, msgbuf + msgofs, msgtogo);
                }

                msgofs = 0;
            }
        }

        void Channel::writefull(const void *_buf, size_t count)
        {
            if (msgtogo + count >= msgbuflen)
            {
                /* Realloc to a multiple of 128.  */
                msgbuflen = (msgtogo + count + 127) & ~(size_t) 127;
                msgbuf = (char *) realloc(msgbuf, msgbuflen);
            }

            memcpy(msgbuf + msgtogo, _buf, count);
            msgtogo += count;
        }

        bool Channel::flush_writebuf(bool blocking)
        {
            const char *buf = msgbuf + msgofs;
            bool error = false;

            while (msgtogo)
            {
        #ifdef MSG_NOSIGNAL
                ssize_t ret = send(fd, buf, msgtogo, MSG_NOSIGNAL);
        #else
                void (*oldsigpipe)(int);

                oldsigpipe = signal(SIGPIPE, SIG_IGN);
                ssize_t ret = send(fd, buf, msgtogo, 0);
                signal(SIGPIPE, oldsigpipe);
        #endif

                if (ret < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }

                    /* If we want to write blocking, but couldn't write anything,
                     select on the fd.  */
                    if (blocking && errno == EAGAIN)
                    {
                        int ready;

                        for (;;)
                        {
                            fd_set write_set;
                            FD_ZERO(&write_set);
                            FD_SET(fd, &write_set);
                            struct timeval tv;
                            tv.tv_sec = 20;
                            tv.tv_usec = 0;
                            ready = select(fd + 1, NULL, &write_set, NULL, &tv);

                            if (ready < 0 && errno == EINTR)
                            {
                                continue;
                            }

                            break;
                        }

                        /* socket ready now for writing ? */
                        if (ready > 0)
                        {
                            continue;
                        }

                        /* Timeout or real error --> error.  */
                    }

                    log_perror("flush_writebuf() failed");
                    error = true;
                    break;
                }
                else if (ret == 0)
                {
                    // EOF while writing --> error
                    error = true;
                    break;
                }

                msgtogo -= ret;
                buf += ret;
            }

            msgofs = buf - msgbuf;
            chop_output();
            return !error;
        }

        Channel &Channel::operator>>(uint32_t &buf)
        {
            if (inofs >= intogo + 4)
            {
                if (ptrdiff_t(inbuf + intogo) % 4)
                {
                    uint32_t t_buf[1];
                    memcpy(t_buf, inbuf + intogo, 4);
                    buf = t_buf[0];
                }
                else
                {
                    buf = *(uint32_t *) (inbuf + intogo);
                }

                intogo += 4;
                buf = ntohl(buf);
            }
            else
            {
                buf = 0;
            }

            return *this;
        }

        Channel &Channel::operator<<(uint32_t i)
        {
            i = htonl(i);
            writefull(&i, 4);
            return *this;
        }

        Channel &Channel::operator>>(string &s)
        {
            char *buf;
            // len is including the (also saved) 0 Byte
            uint32_t len;
            *this >> len;

            if (!len || len > inofs - intogo)
            {
                s = "";
            }
            else
            {
                buf = inbuf + intogo;
                intogo += len;
                s = buf;
            }

            return *this;
        }

        Channel &Channel::operator<<(const std::string &s)
        {
            uint32_t len = 1 + s.length();
            *this << len;
            writefull(s.c_str(), len);
            return *this;
        }

        Channel &Channel::operator>>(list<string> &l)
        {
            uint32_t len;
            l.clear();
            *this >> len;

            while (len--)
            {
                string s;
                *this >> s;
                l.push_back(s);

                if (inofs == intogo)
                {
                    break;
                }
            }

            return *this;
        }

        Channel &Channel::operator<<(const std::list<std::string> &l)
        {
            *this << static_cast<uint32_t>(l.size());

            for (list<string>::const_iterator it = l.begin(); it != l.end(); ++it)
            {
                *this << *it;
            }

            return *this;
        }

        void Channel::write_environments(const Environments &envs)
        {
            *this << envs.size();

            for (Environments::const_iterator it = envs.begin(); it != envs.end(); ++it)
            {
                *this << it->first;
                *this << it->second;
            }
        }

        void Channel::read_environments(Environments &envs)
        {
            envs.clear();
            uint32_t count;
            *this >> count;

            for (unsigned int i = 0; i < count; i++)
            {
                string plat;
                string vers;
                *this >> plat;
                *this >> vers;
                envs.push_back(make_pair(plat, vers));
            }
        }

        void Channel::readcompressed(unsigned char **uncompressed_buf, size_t &_uclen,
                size_t &_clen)
        {
            lzo_uint uncompressed_len;
            lzo_uint compressed_len;
            uint32_t tmp;
            *this >> tmp;
            uncompressed_len = tmp;
            *this >> tmp;
            compressed_len = tmp;

            /* If there was some input, but nothing compressed,
             or lengths are bigger than the whole chunk message
             or we don't have everything to uncompress, there was an error.  */
            if (uncompressed_len > MAX_MSG_SIZE || compressed_len > (inofs - intogo)
                    || (uncompressed_len && !compressed_len)
                    || inofs < intogo + compressed_len)
            {
                log_error() << "failure in readcompressed() length checking" << endl;
                *uncompressed_buf = 0;
                uncompressed_len = 0;
                _uclen = uncompressed_len;
                _clen = compressed_len;
                return;
            }

            *uncompressed_buf = new unsigned char[uncompressed_len];

            if (uncompressed_len && compressed_len)
            {
                const lzo_byte *compressed_buf = static_cast<lzo_byte*>(inbuf + intogo);
                lzo_voidp wrkmem = static_cast<lzo_voidp>(malloc(LZO1X_MEM_COMPRESS));
                int ret = lzo1x_decompress(compressed_buf, compressed_len,
                        *uncompressed_buf, &uncompressed_len, wrkmem);
                free(wrkmem);

                if (ret != LZO_E_OK)
                {
                    /* This should NEVER happen.
                     Remove the buffer, and indicate there is nothing in it,
                     but don't reset the compressed_len, so our caller know,
                     that there actually was something read in.  */
                    log_error() << "internal error - decompression of data from "
                            << dump().c_str() << " failed: " << ret << endl;
                    delete[] *uncompressed_buf;
                    *uncompressed_buf = 0;
                    uncompressed_len = 0;
                }
            }

            /* Read over everything used, _also_ if there was some error.
             If we couldn't decode it now, it won't get better in the future,
             so just ignore this hunk.  */
            intogo += compressed_len;
            _uclen = uncompressed_len;
            _clen = compressed_len;
        }

        void Channel::writecompressed(const unsigned char *in_buf, size_t _in_len,
                size_t &_out_len)
        {
            lzo_uint in_len = _in_len;
            lzo_uint out_len = _out_len;
            out_len = in_len + in_len / 64 + 16 + 3;
            *this << in_len;
            size_t msgtogo_old = msgtogo;
            *this << static_cast<uint32_t>(0);

            if (msgtogo + out_len >= msgbuflen)
            {
                /* Realloc to a multiple of 128.  */
                msgbuflen = (msgtogo + out_len + 127) & ~static_cast<size_t>(127);
                msgbuf = (char *) realloc(msgbuf, msgbuflen);
            }

            lzo_byte *out_buf = static_cast<lzo_byte*>(msgbuf + msgtogo);
            lzo_voidp wrkmem = static_cast<lzo_voidp>(malloc(LZO1X_MEM_COMPRESS));
            int ret = lzo1x_1_compress(in_buf, in_len, out_buf, &out_len, wrkmem);
            free(wrkmem);

            if (ret != LZO_E_OK)
            {
                /* this should NEVER happen */
                log_error() << "internal error - compression failed: " << ret << endl;
                out_len = 0;
            }

            uint32_t _olen = htonl(out_len);
            memcpy(msgbuf + msgtogo_old, &_olen, 4);
            msgtogo += out_len;
            _out_len = out_len;
        }

        void Channel::read_line(string &line)
        {
            /* XXX handle DOS and MAC line endings and null bytes as string endings.  */
            if (!text_based || inofs < intogo)
            {
                line = "";
            }
            else
            {
                line = string(inbuf + intogo, inmsglen);
                intogo += inmsglen;

                while (intogo < inofs && inbuf[intogo] < ' ')
                {
                    intogo++;
                }
            }
        }

        void Channel::write_line(const string &line)
        {
            size_t len = line.length();
            writefull(line.c_str(), len);

            if (line[len - 1] != '\n')
            {
                char c = '\n';
                writefull(&c, 1);
            }
        }
        bool Channel::eq_ip(const Channel &s) const
        {
            struct sockaddr_in *s1, *s2;
            s1 = (struct sockaddr_in *) addr;
            s2 = (struct sockaddr_in *) s.addr;
            return (addr_len == s.addr_len
                    && memcmp(&s1->sin_addr, &s2->sin_addr, sizeof(s1->sin_addr)) == 0);
        }

        string Channel::dump() const
        {
            return name + ": (" + char((int) instate + 'A') + " eof: " + char(eof + '0')
                    + ")";
        }

        /* Wait blocking until the protocol setup for this channel is complete.
         Returns false if an error occurred.  */
        bool Channel::wait_for_protocol()
        {
            /* protocol is 0 if we couldn't send our initial protocol version.  */
            if (protocol == 0)
            {
                return false;
            }

            while (instate == NEED_PROTO)
            {
                fd_set set;
                FD_ZERO(&set);
                FD_SET(fd, &set);
                struct timeval tv;
                tv.tv_sec = 5;
                tv.tv_usec = 0;
                int ret = select(fd + 1, &set, NULL, NULL, &tv);

                if (ret < 0 && errno == EINTR)
                {
                    continue;
                }

                if (ret == 0)
                {
                    log_error() << "no response from local daemon within timeout."
                            << endl;
                    return false; /* timeout. Consider it a fatal error. */
                }

                if (ret < 0)
                {
                    log_perror("select in wait_for_protocol()");
                    return false;
                }

                if (!read_a_bit() || eof)
                {
                    return false;
                }
            }

            return true;
        }

        void Channel::setBulkTransfer()
        {
            if (fd < 0)
            {
                return;
            }

            int i = 0;
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, static_cast<char>(&i), sizeof(i));

            // would be nice but not portable across non-linux
        #ifdef __linux__
            i = 1;
            setsockopt(fd, IPPROTO_TCP, TCP_CORK, static_cast<char*>(&i), sizeof(i));
        #endif
            i = 65536;
            setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &i, sizeof(i));
        }

        /* This waits indefinitely (well, TIMEOUT seconds) for a complete
         message to arrive.  Returns false if there was some error.  */
        bool Channel::wait_for_msg(int timeout)
        {
            if (has_msg())
            {
                return true;
            }

            if (!read_a_bit())
            {
                trace() << "!read_a_bit\n";
                return false;
            }

            if (timeout <= 0)
            {
                trace() << "timeout <= 0\n";
                return has_msg();
            }

            while (!has_msg())
            {
                fd_set read_set;
                FD_ZERO(&read_set);
                FD_SET(fd, &read_set);
                struct timeval tv;
                tv.tv_sec = timeout;
                tv.tv_usec = 0;

                if (select(fd + 1, &read_set, NULL, NULL, &tv) <= 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }

                    /* Either timeout or real error.  For this function also
                     a timeout is an error.  */
                    return false;
                }

                if (!read_a_bit())
                {
                    trace() << "!read_a_bit 2\n";
                    return false;
                }
            }

            return true;
        }

        Msg *Channel::get_msg(int timeout)
        {
            Msg *m = 0;
            MsgType type;
            uint32_t t;

            if (!wait_for_msg(timeout))
            {
                trace() << "!wait_for_msg()\n";
                return 0;
            }

            /* If we've seen the EOF, and we don't have a complete message,
             then we won't see it anymore.  Return that to the caller.
             Don't use has_msg() here, as it returns true for eof.  */
            if (at_eof())
            {
                trace() << "saw eof without complete msg! " << instate << endl;
                return 0;
            }

            if (!has_msg())
            {
                trace() << "saw eof without msg! " << eof << " " << instate << endl;
                return 0;
            }

            if (text_based)
            {
                type = MsgType::TEXT;
            }
            else
            {
                *this >> t;
                type = static_cast<MsgType>(t);
            }

            switch (type)
            {
            case MsgType::UNKNOWN:
                return 0;
            case MsgType::PING:
                m = new Ping;
                break;
            case MsgType::END:
                m = new End;
                break;
            case MsgType::GET_CS:
                m = new GetCS;
                break;
            case MsgType::USE_CS:
                m = new UseCS;
                break;
            case MsgType::COMPILE_FILE:
                m = new CompileFile(new CompileJob, true);
                break;
            case MsgType::FILE_CHUNK:
                m = new FileChunk;
                break;
            case MsgType::COMPILE_RESULT:
                m = new CompileResult;
                break;
            case MsgType::JOB_BEGIN:
                m = new JobBegin;
                break;
            case MsgType::JOB_DONE:
                m = new JobDone;
                break;
            case MsgType::LOGIN:
                m = new Login;
                break;
            case MsgType::STATS:
                m = new Stats;
                break;
            case MsgType::GET_NATIVE_ENV:
                m = new GetNativeEnv;
                break;
            case MsgType::NATIVE_ENV:
                m = new UseNativeEnv;
                break;
            case MsgType::MON_LOGIN:
                m = new MonLogin;
                break;
            case MsgType::MON_GET_CS:
                m = new MonGetCS;
                break;
            case MsgType::MON_JOB_BEGIN:
                m = new MonJobBegin;
                break;
            case MsgType::MON_JOB_DONE:
                m = new MonJobDone;
                break;
            case MsgType::MON_STATS:
                m = new MonStats;
                break;
            case MsgType::JOB_LOCAL_BEGIN:
                m = new JobLocalBegin;
                break;
            case MsgType::JOB_LOCAL_DONE:
                m = new JobLocalDone;
                break;
            case MsgType::MON_LOCAL_JOB_BEGIN:
                m = new MonLocalJobBegin;
                break;
            case MsgType::TRANFER_ENV:
                m = new EnvTransfer;
                break;
            case MsgType::TEXT:
                m = new Text;
                break;
            case MsgType::GET_INTERNALS:
                m = new GetInternalStatus;
                break;
            case MsgType::STATUS_TEXT:
                m = new StatusText;
                break;
            case MsgType::CS_CONF:
                m = new ConfCS;
                break;
            case MsgType::VERIFY_ENV:
                m = new VerifyEnv;
                break;
            case MsgType::VERIFY_ENV_RESULT:
                m = new VerifyEnvResult;
                break;
            case MsgType::BLACKLIST_HOST_ENV:
                m = new BlacklistHostEnv;
                break;
            case MsgType::TIMEOUT:
                break;
            }

            if (!m)
            {
                trace() << "no message type" << endl;
                return 0;
            }

            m->fill_from_channel(this);
            instate = NEED_LEN;
            update_state();

            return m;
        }

        bool Channel::send_msg(const Msg &m, SendFlags flags)
        {
            if (instate == NEED_PROTO && !wait_for_protocol())
            {
                return false;
            }

            chop_output();
            size_t msgtogo_old = msgtogo;

            if (text_based)
            {
                m.send_to_channel(this);
            }
            else
            {
                *this << static_cast<uint32_t>(0);
                m.send_to_channel(this);
                uint32_t len = htonl(msgtogo - msgtogo_old - 4);
                memcpy(msgbuf + msgtogo_old, &len, 4);
            }

            if ((flags & SendBulkOnly) && msgtogo < 4096)
            {
                return true;
            }

            return flush_writebuf((flags & SendBlocking));
        }
    } // services
} // icecream
