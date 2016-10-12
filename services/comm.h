/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2004 Michael Matz <matz@suse.de>
                  2004 Stephan Kulow <coolo@suse.de>

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


#ifndef ICECREAM_COMM_H
#define ICECREAM_COMM_H

#ifdef __linux__
#  include <stdint.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "job.h"

// if you increase the PROTOCOL_VERSION, add a macro below and use that
constexpr uint32_t PROTOCOL_VERSION = 35;
// if you increase the MIN_PROTOCOL_VERSION, comment out macros below and clean up the code
constexpr uint32_t MIN_PROTOCOL_VERSION = 21;

constexpr uint32_t MAX_SCHEDULER_PONG = 3;
// MAX_SCHEDULER_PING must be multiple of MAX_SCHEDULER_PONG
constexpr uint32_t MAX_SCHEDULER_PING = 12 * MAX_SCHEDULER_PONG;
// maximum amount of time in seconds a daemon can be busy installing
constexpr uint32_t MAX_BUSY_INSTALLING = 120;

#define IS_PROTOCOL_22(c) ((c)->protocol >= 22)
#define IS_PROTOCOL_23(c) ((c)->protocol >= 23)
#define IS_PROTOCOL_24(c) ((c)->protocol >= 24)
#define IS_PROTOCOL_25(c) ((c)->protocol >= 25)
#define IS_PROTOCOL_26(c) ((c)->protocol >= 26)
#define IS_PROTOCOL_27(c) ((c)->protocol >= 27)
#define IS_PROTOCOL_28(c) ((c)->protocol >= 28)
#define IS_PROTOCOL_29(c) ((c)->protocol >= 29)
#define IS_PROTOCOL_30(c) ((c)->protocol >= 30)
#define IS_PROTOCOL_31(c) ((c)->protocol >= 31)
#define IS_PROTOCOL_32(c) ((c)->protocol >= 32)
#define IS_PROTOCOL_33(c) ((c)->protocol >= 33)
#define IS_PROTOCOL_34(c) ((c)->protocol >= 34)
#define IS_PROTOCOL_35(c) ((c)->protocol >= 35)

enum class MsgType : uint32_t
{
    // so far unknown
    UNKNOWN = 'A',

    /* When the scheduler didn't get STATS from a CS
       for a specified time (e.g. 10m), then he sends a
       ping */
    PING,

    /* Either the end of file chunks or connection (A<->A) */
    END,

    TIMEOUT, // unused

    // C --> CS
    GET_NATIVE_ENV,
    // CS -> C
    NATIVE_ENV,

    // C --> S
    GET_CS,
    // S --> C
    USE_CS,  // = 'G'

    // C --> CS
    COMPILE_FILE, // = 'I'
    // generic file transfer
    FILE_CHUNK,
    // CS --> C
    COMPILE_RESULT,

    // CS --> S (after the C got the CS from the S, the CS tells the S when the C asks him)
    JOB_BEGIN,
    JOB_DONE,     // = 'M'

    // C --> CS, CS --> S (forwarded from C), _and_ CS -> C as start ping
    JOB_LOCAL_BEGIN, // = 'N'
    JOB_LOCAL_DONE,

    // CS --> S, first message sent
    LOGIN,
    // CS --> S (periodic)
    STATS,

    // messages between monitor and scheduler
    MON_LOGIN,
    MON_GET_CS,
    MON_JOB_BEGIN, // = 'T'
    MON_JOB_DONE,
    MON_LOCAL_JOB_BEGIN,
    MON_STATS,

    TRANFER_ENV, // = 'X'

    TEXT,
    STATUS_TEXT,
    GET_INTERNALS,

    // S --> CS, answered by LOGIN
    CS_CONF,

    // C --> CS, after installing an environment
    VERIFY_ENV,
    VERIFY_ENV_RESULT,
    // C --> CS, CS --> S (forwarded from C), to not use given host for given environment
    BLACKLIST_HOST_ENV
};

class Channel;

// a list of pairs of host platform, filename
typedef std::list<std::pair<std::string, std::string> > Environments;

class Msg
{
public:
    Msg(enum MsgType t)
        : type(t) {}
    virtual ~Msg() {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    enum MsgType type;
};

class Channel
{
public:
    enum SendFlags {
        SendBlocking = 1 << 0,
        SendNonBlocking = 1 << 1,
        SendBulkOnly = 1 << 2
    };

    virtual ~Channel();

    void setBulkTransfer();

    std::string dump() const;
    // NULL  <--> channel closed or timeout
    Msg *get_msg(int timeout = 10);

    // false <--> error (msg not send)
    bool send_msg(const Msg &, int SendFlags = SendBlocking);

    bool has_msg(void) const
    {
        return eof || instate == HAS_MSG;
    }

    bool read_a_bit(void);

    bool at_eof(void) const
    {
        return instate != HAS_MSG && eof;
    }

    bool is_text_based(void) const
    {
        return text_based;
    }

    void readcompressed(unsigned char **buf, size_t &_uclen, size_t &_clen);
    void writecompressed(const unsigned char *in_buf,
                         size_t _in_len, size_t &_out_len);
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

    enum {
        NEED_PROTO,
        NEED_LEN,
        FILL_BUF,
        HAS_MSG
    } instate;

    uint32_t inmsglen;
    bool eof;
    bool text_based;

private:
    friend class Service;

    // deep copied
    struct sockaddr *addr;
    socklen_t addr_len;
};

// just convenient functions to create Channels
class Service
{
public:
    static Channel *createChannel(const std::string &host, unsigned short p, int timeout);
    static Channel *createChannel(const std::string &domain_socket);
    static Channel *createChannel(int remote_fd, struct sockaddr *, socklen_t);
};

// --------------------------------------------------------------------------
// this class is also used by icecream-monitor
class DiscoverSched
{
public:
    /* Connect to a scheduler waiting max. TIMEOUT seconds.
       schedname can be the hostname of a box running a scheduler, to avoid
       broadcasting, port can be specified explicitly */
    DiscoverSched(const std::string &_netname = std::string(),
                  int _timeout = 2,
                  const std::string &_schedname = std::string(),
                  int port = 0);
    ~DiscoverSched();

    bool timed_out();

    int listen_fd() const
    {
        return schedname.empty() ? ask_fd : -1;
    }

    int connect_fd() const
    {
        return schedname.empty() ? -1 : ask_fd;
    }

    // compat for icecream monitor
    int get_fd() const
    {
        return listen_fd();
    }

    /* Attempt to get a conenction to the scheduler.
    
       Continue to call this while it returns NULL and timed_out() 
       returns false. If this returns NULL you should wait for either
       more data on listen_fd() (use select), or a timeout of your own.  
       */
    Channel *try_get_scheduler();

    // Returns the hostname of the scheduler - set by constructor or by try_get_scheduler
    std::string schedulerName() const
    {
        return schedname;
    }

    // Returns the network name of the scheduler - set by constructor or by try_get_scheduler
    std::string networkName() const
    {
        return netname;
    }

    /// Broadcasts the given data on the given port.
    static bool broadcastData(int port, const char* buf, int size);

private:
    struct sockaddr_in remote_addr;
    std::string netname;
    std::string schedname;
    int timeout;
    int ask_fd;
    time_t time0;
    unsigned int sport;
    int best_version;
    time_t best_start_time;
    bool multiple;

    void attempt_scheduler_connect();
};
// --------------------------------------------------------------------------

/* Return a list of all reachable netnames.  We wait max. WAITTIME
   milliseconds for answers.  */
std::list<std::string> get_netnames(int waittime = 2000, int port = 8765);

class Ping : public Msg
{
public:
    Ping()
        : Msg(MsgType::PING) {}
};

class End : public Msg
{
public:
    End()
        : Msg(MsgType::END) {}
};

class GetCS : public Msg
{
public:
    GetCS()
        : Msg(MsgType::GET_CS)
        , count(1)
        , arg_flags(0)
        , client_id(0) {}

    GetCS(const Environments &envs, const std::string &f,
             CompileJob::Language _lang, unsigned int _count,
             std::string _target, unsigned int _arg_flags,
             const std::string &host, int _minimal_host_version)
        : Msg(MsgType::GET_CS)
        , versions(envs)
        , filename(f)
        , lang(_lang)
        , count(_count)
        , target(_target)
        , arg_flags(_arg_flags)
        , client_id(0)
        , preferred_host(host)
        , minimal_host_version(_minimal_host_version) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    Environments versions;
    std::string filename;
    CompileJob::Language lang;
    uint32_t count; // the number of UseCS messages to answer with - usually 1
    std::string target;
    uint32_t arg_flags;
    uint32_t client_id;
    std::string preferred_host;
    int minimal_host_version;
};

class UseCS : public Msg
{
public:
    UseCS()
        : Msg(MsgType::USE_CS) {}
    UseCS(std::string platform, std::string host, unsigned int p, unsigned int id, bool gotit,
             unsigned int _client_id, unsigned int matched_host_jobs)
        : Msg(MsgType::USE_CS),
          job_id(id),
          hostname(host),
          port(p),
          host_platform(platform),
          got_env(gotit),
          client_id(_client_id),
          matched_job_id(matched_host_jobs) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
    std::string hostname;
    uint32_t port;
    std::string host_platform;
    uint32_t got_env;
    uint32_t client_id;
    uint32_t matched_job_id;
};

class GetNativeEnv : public Msg
{
public:
    GetNativeEnv()
        : Msg(MsgType::GET_NATIVE_ENV) {}

    GetNativeEnv(const std::string &c, const std::list<std::string> &e)
        : Msg(MsgType::GET_NATIVE_ENV)
        , compiler(c)
        , extrafiles(e) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string compiler; // "gcc" or "clang" right now
    std::list<std::string> extrafiles;
};

class UseNativeEnv : public Msg
{
public:
    UseNativeEnv()
        : Msg(MsgType::NATIVE_ENV) {}

    UseNativeEnv(std::string _native)
        : Msg(MsgType::NATIVE_ENV)
        , nativeVersion(_native) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string nativeVersion;
};

class CompileFile : public Msg
{
public:
    CompileFile(CompileJob *j, bool delete_job = false)
        : Msg(MsgType::COMPILE_FILE)
        , deleteit(delete_job)
        , job(j) {}

    ~CompileFile()
    {
        if (deleteit) {
            delete job;
        }
    }

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;
    CompileJob *takeJob();

private:
    std::string remote_compiler_name() const;

    bool deleteit;
    CompileJob *job;
};

class FileChunk : public Msg
{
public:
    FileChunk(unsigned char *_buffer, size_t _len)
        : Msg(MsgType::FILE_CHUNK)
        , buffer(_buffer)
        , len(_len)
        , del_buf(false) {}

    FileChunk()
        : Msg(MsgType::FILE_CHUNK)
        , buffer(0)
        , len(0)
        , del_buf(true) {}

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

class CompileResult : public Msg
{
public:
    CompileResult()
        : Msg(MsgType::COMPILE_RESULT)
        , status(0)
        , was_out_of_memory(false)
        , have_dwo_file(false) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    int status;
    std::string out;
    std::string err;
    bool was_out_of_memory;
    bool have_dwo_file;
};

class JobBegin : public Msg
{
public:
    JobBegin()
        : Msg(MsgType::JOB_BEGIN) {}

    JobBegin(unsigned int j)
        : Msg(MsgType::JOB_BEGIN)
        , job_id(j)
        , stime(time(0)) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
    uint32_t stime;
};

enum SpecialExits {
    CLIENT_WAS_WAITING_FOR_CS = 200
};

class JobDone : public Msg
{
public:
    /* FROM_SERVER: this message was generated by the daemon responsible
          for remotely compiling the job (i.e. job->server).
       FROM_SUBMITTER: this message was generated by the daemon connected
          to the submitting client.  */
    enum from_type {
        FROM_SERVER = 0,
        FROM_SUBMITTER = 1
    };

    JobDone(int job_id = 0, int exitcode = -1, unsigned int flags = FROM_SERVER);

    void set_from(from_type from)
    {
        flags |= (uint32_t)from;
    }

    bool is_from_server()
    {
        return (flags & FROM_SUBMITTER) == 0;
    }

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t real_msec; /* real time it used */
    uint32_t user_msec; /* user time used */
    uint32_t sys_msec; /* system time used */
    uint32_t pfaults; /* page faults */

    int exitcode; /* exit code */

    uint32_t flags;

    uint32_t in_compressed;
    uint32_t in_uncompressed;
    uint32_t out_compressed;
    uint32_t out_uncompressed;

    uint32_t job_id;
};

class JobLocalBegin : public Msg
{
public:
    JobLocalBegin(int job_id = 0, const std::string &file = "")
        : Msg(MsgType::JOB_LOCAL_BEGIN)
        , outfile(file)
        , stime(time(0))
        , id(job_id) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string outfile;
    uint32_t stime;
    uint32_t id;
};

class JobLocalDone : public Msg
{
public:
    JobLocalDone(unsigned int id = 0)
        : Msg(MsgType::JOB_LOCAL_DONE)
        , job_id(id) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
};

class Login : public Msg
{
public:
    Login(unsigned int myport, const std::string &_nodename, const std::string _host_platform);
    Login()
        : Msg(MsgType::LOGIN)
        , port(0) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t port;
    Environments envs;
    uint32_t max_kids;
    bool noremote;
    bool chroot_possible;
    std::string nodename;
    std::string host_platform;
};

class ConfCS : public Msg
{
public:
    ConfCS()
        : Msg(MsgType::CS_CONF)
        , max_scheduler_pong(MAX_SCHEDULER_PONG)
        , max_scheduler_ping(MAX_SCHEDULER_PING) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t max_scheduler_pong;
    uint32_t max_scheduler_ping;
};

class Stats : public Msg
{
public:
    Stats()
        : Msg(MsgType::STATS)
    {
        load = 0;
    }

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    /**
     * For now the only load measure we have is the
     * load from 0-1000.
     * This is defined to be a daemon defined value
     * on how busy the machine is. The higher the load
     * is, the slower a given job will compile (preferably
     * linear scale). Load of 1000 means to not schedule
     * another job under no circumstances.
     */
    uint32_t load;

    uint32_t loadAvg1;
    uint32_t loadAvg5;
    uint32_t loadAvg10;
    uint32_t freeMem;
};

class EnvTransfer : public Msg
{
public:
    EnvTransfer()
        : Msg(MsgType::TRANFER_ENV) {}

    EnvTransfer(const std::string &_target, const std::string &_name)
        : Msg(MsgType::TRANFER_ENV)
        , name(_name)
        , target(_target) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string name;
    std::string target;
};

class GetInternalStatus : public Msg
{
public:
    GetInternalStatus()
        : Msg(MsgType::GET_INTERNALS) {}

    GetInternalStatus(const GetInternalStatus &)
        : Msg(MsgType::GET_INTERNALS) {}
};

class MonLogin : public Msg
{
public:
    MonLogin()
        : Msg(MsgType::MON_LOGIN) {}
};

class MonGetCS : public GetCS
{
public:
    MonGetCS()
        : GetCS()
    { // overwrite
        type = MsgType::MON_GET_CS;
        clientid = job_id = 0;
    }

    MonGetCS(int jobid, int hostid, GetCS *m)
        : GetCS(Environments(), m->filename, m->lang, 1, m->target, 0, std::string(), false)
        , job_id(jobid)
        , clientid(hostid)
    {
        type = MsgType::MON_GET_CS;
    }

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
    uint32_t clientid;
};

class MonJobBegin : public Msg
{
public:
    MonJobBegin()
        : Msg(MsgType::MON_JOB_BEGIN)
        , job_id(0)
        , stime(0)
        , hostid(0) {}

    MonJobBegin(unsigned int id, unsigned int time, int _hostid)
        : Msg(MsgType::MON_JOB_BEGIN)
        , job_id(id)
        , stime(time)
        , hostid(_hostid) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
    uint32_t stime;
    uint32_t hostid;
};

class MonJobDone : public JobDone
{
public:
    MonJobDone()
        : JobDone()
    {
        type = MsgType::MON_JOB_DONE;
    }

    MonJobDone(const JobDone &o)
        : JobDone(o)
    {
        type = MsgType::MON_JOB_DONE;
    }
};

class MonLocalJobBegin : public Msg
{
public:
    MonLocalJobBegin()
        : Msg(MsgType::MON_LOCAL_JOB_BEGIN) {}

    MonLocalJobBegin(unsigned int id, const std::string &_file, unsigned int time, int _hostid)
        : Msg(MsgType::MON_LOCAL_JOB_BEGIN)
        , job_id(id)
        , stime(time)
        , hostid(_hostid)
        , file(_file) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t job_id;
    uint32_t stime;
    uint32_t hostid;
    std::string file;
};

class MonStats : public Msg
{
public:
    MonStats()
        : Msg(MsgType::MON_STATS) {}

    MonStats(int id, const std::string &msg_stat)
        : Msg(MsgType::MON_STATS)
        , hostid(id)
        , statmsg(msg_stat) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    uint32_t hostid;
    std::string statmsg;
};

class Text : public Msg
{
public:
    Text()
        : Msg(MsgType::TEXT) {}

    Text(const std::string &_text)
        : Msg(MsgType::TEXT)
        , text(_text) {}

    Text(const Text &m)
        : Msg(MsgType::TEXT)
        , text(m.text) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string text;
};

class StatusText : public Msg
{
public:
    StatusText()
        : Msg(MsgType::STATUS_TEXT) {}

    StatusText(const std::string &_text)
        : Msg(MsgType::STATUS_TEXT)
        , text(_text) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string text;
};

class VerifyEnv : public Msg
{
public:
    VerifyEnv()
        : Msg(MsgType::VERIFY_ENV) {}

    VerifyEnv(const std::string &_target, const std::string &_environment)
        : Msg(MsgType::VERIFY_ENV)
        , environment(_environment)
        , target(_target) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string environment;
    std::string target;
};

class VerifyEnvResult : public Msg
{
public:
    VerifyEnvResult()
        : Msg(MsgType::VERIFY_ENV_RESULT) {}

    VerifyEnvResult(bool _ok)
        : Msg(MsgType::VERIFY_ENV_RESULT)
        , ok(_ok) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    bool ok;
};

class BlacklistHostEnv : public Msg
{
public:
    BlacklistHostEnv()
        : Msg(MsgType::BLACKLIST_HOST_ENV) {}

    BlacklistHostEnv(const std::string &_target, const std::string &_environment, const std::string &_hostname)
        : Msg(MsgType::BLACKLIST_HOST_ENV)
        , environment(_environment)
        , target(_target)
        , hostname(_hostname) {}

    virtual void fill_from_channel(Channel *c);
    virtual void send_to_channel(Channel *c) const;

    std::string environment;
    std::string target;
    std::string hostname;
};

#endif
