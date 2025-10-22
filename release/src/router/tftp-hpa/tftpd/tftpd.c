/*
 * Copyright (c) 1983 Regents of the University of California.
 * Copyright (c) 1999-2009 H. Peter Anvin
 * Copyright (c) 2011 Intel Corporation; author: H. Peter Anvin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"             /* Must be included first */
#include "tftpd.h"

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#include <sys/ioctl.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <limits.h>
#include <syslog.h>

#include "common/tftpsubs.h"
#include "recvfrom.h"
#include "remap.h"

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>          /* Necessary for FIONBIO on Solaris */
#endif

#ifdef HAVE_TCPWRAPPERS
#include <tcpd.h>

int deny_severity = LOG_WARNING;
int allow_severity = -1;        /* Don't log at all */

static struct request_info wrap_request;
#endif

#ifdef HAVE_IPV6
static int ai_fam = AF_UNSPEC;
#else
static int ai_fam = AF_INET;
#endif

#define	TIMEOUT 1000000         /* Default timeout (us) */
#define TRIES   6               /* Number of attempts to send each packet */
#define TIMEOUT_LIMIT ((1 << TRIES)-1)

const char *__progname;
static int peer;
static unsigned long timeout  = TIMEOUT;        /* Current timeout value */
static unsigned long rexmtval = TIMEOUT;       /* Basic timeout value */
static unsigned long maxtimeout = TIMEOUT_LIMIT * TIMEOUT;
static int timeout_quit = 0;
static sigjmp_buf timeoutbuf;
static uint16_t rollover_val = 0;

#define	PKTSIZE	MAX_SEGSIZE+4
static char buf[PKTSIZE];
static char ackbuf[PKTSIZE];
static unsigned int max_blksize = MAX_SEGSIZE;

static char tmpbuf[INET6_ADDRSTRLEN], *tmp_p;

static union sock_addr from;
static socklen_t fromlen;
static off_t tsize;
static int tsize_ok;

static int ndirs;
static const char **dirs;

static int secure = 0;
int cancreate = 0;
int unixperms = 0;
int portrange = 0;
unsigned int portrange_from, portrange_to;
int verbosity = 0;

struct formats;
#ifdef WITH_REGEX
static struct rule *rewrite_rules = NULL;
#endif

int tftp(struct tftphdr *, int);
static void nak(int, const char *);
static void timer(int);
static void do_opt(const char *, const char *, char **);

static int set_blksize(uintmax_t *);
static int set_blksize2(uintmax_t *);
static int set_tsize(uintmax_t *);
static int set_timeout(uintmax_t *);
static int set_utimeout(uintmax_t *);
static int set_rollover(uintmax_t *);

struct options {
    const char *o_opt;
    int (*o_fnc)(uintmax_t *);
} options[] = {
    {"blksize",  set_blksize},
    {"blksize2", set_blksize2},
    {"tsize",    set_tsize},
    {"timeout",  set_timeout},
    {"utimeout", set_utimeout},
    {"rollover", set_rollover},
    {NULL, NULL}
};

/* Simple handler for SIGHUP */
static volatile sig_atomic_t caught_sighup = 0;
static void handle_sighup(int sig)
{
    (void)sig;                  /* Suppress unused warning */
    caught_sighup = 1;
}

/* Handle exit requests by SIGTERM and SIGINT */
static volatile sig_atomic_t exit_signal = 0;
static void handle_exit(int sig)
{
    exit_signal = sig;
}

/* Handle timeout signal or timeout event */
void timer(int sig)
{
    (void)sig;                  /* Suppress unused warning */
    timeout <<= 1;
    if (timeout >= maxtimeout || timeout_quit)
        exit(0);
    siglongjmp(timeoutbuf, 1);
}

#ifdef WITH_REGEX
static struct rule *read_remap_rules(const char *file)
{
    FILE *f;
    struct rule *rulep;

    f = fopen(file, "rt");
    if (!f) {
        syslog(LOG_ERR, "Cannot open map file: %s: %m", file);
        exit(EX_NOINPUT);
    }
    rulep = parserulefile(f);
    fclose(f);

    return rulep;
}
#endif

/*
 * Rules for locking files; return 0 on success, -1 on failure
 */
static int lock_file(int fd, int lock_write)
{
#if defined(HAVE_FCNTL) && defined(HAVE_F_SETLK_DEFINITION)
  struct flock fl;

  fl.l_type   = lock_write ? F_WRLCK : F_RDLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 0;		/* Whole file */
  return fcntl(fd, F_SETLK, &fl);
#elif defined(HAVE_LOCK_SH_DEFINITION)
  return flock(fd, lock_write ? LOCK_EX|LOCK_NB : LOCK_SH|LOCK_NB);
#else
  return 0;			/* Hope & pray... */
#endif
}

static void set_socket_nonblock(int fd, int flag)
{
    int err;
    int flags;
#if defined(HAVE_FCNTL) && defined(HAVE_O_NONBLOCK_DEFINITION)
    /* Posixly correct */
    err = ((flags = fcntl(fd, F_GETFL, 0)) < 0) ||
        (fcntl
         (fd, F_SETFL,
          flag ? flags | O_NONBLOCK : flags & ~O_NONBLOCK) < 0);
#else
    flags = flag ? 1 : 0;
    err = (ioctl(fd, FIONBIO, &flags) < 0);
#endif
    if (err) {
        syslog(LOG_ERR, "Cannot set nonblock flag on socket: %m");
        exit(EX_OSERR);
    }
}

static void pmtu_discovery_off(int fd)
{
#if defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)
    int pmtu = IP_PMTUDISC_DONT;

    setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &pmtu, sizeof(pmtu));
#endif
}

/*
 * Receive packet with synchronous timeout; timeout is adjusted
 * to account for time spent waiting.
 */
static int recv_time(int s, void *rbuf, int len, unsigned int flags,
                     unsigned long *timeout_us_p)
{
    fd_set fdset;
    struct timeval tmv, t0, t1;
    int rv, err;
    unsigned long timeout_us = *timeout_us_p;
    unsigned long timeout_left, dt;

    gettimeofday(&t0, NULL);
    timeout_left = timeout_us;

    for (;;) {
        FD_ZERO(&fdset);
        FD_SET(s, &fdset);

        do {
            tmv.tv_sec = timeout_left / 1000000;
            tmv.tv_usec = timeout_left % 1000000;

            rv = select(s + 1, &fdset, NULL, NULL, &tmv);
            err = errno;

            gettimeofday(&t1, NULL);

            dt = (t1.tv_sec - t0.tv_sec) * 1000000 +
		 (t1.tv_usec - t0.tv_usec);
            *timeout_us_p = timeout_left =
                (dt >= timeout_us) ? 1 : (timeout_us - dt);
        } while (rv == -1 && err == EINTR);

        if (rv == 0) {
            timer(0);           /* Should not return */
            return -1;
        }

        set_socket_nonblock(s, 1);
        rv = recv(s, rbuf, len, flags);
        err = errno;
        set_socket_nonblock(s, 0);

        if (rv < 0) {
            if (E_WOULD_BLOCK(err) || err == EINTR) {
                continue;       /* Once again, with feeling... */
            } else {
                errno = err;
                return rv;
            }
        } else {
            return rv;
        }
    }
}

static int split_port(char **ap, char **pp)
{
    char *a, *p;
    int  ret = AF_UNSPEC;

    a = *ap;
#ifdef HAVE_IPV6
    if (is_numeric_ipv6(a)) {
        if (*a++ != '[')
            return -1;
        *ap = a;
        p = strrchr(a, ']');
        if (!p)
            return -1;
        *p++ = 0;
        a = p;
        ret = AF_INET6;
        p = strrchr(a, ':');
        if (p)
            *p++ = 0;
    } else
#endif
    {
        struct in_addr in;

        p = strrchr(a, ':');
        if (p)
            *p++ = 0;
        if (inet_aton(a, &in))
            ret = AF_INET;
    }
    *pp = p;
    return ret;
}

enum long_only_options {
    OPT_VERBOSITY	= 256,
};
    
static struct option long_options[] = {
    { "ipv4",        0, NULL, '4' },
    { "ipv6",        0, NULL, '6' },
    { "create",      0, NULL, 'c' },
    { "secure",      0, NULL, 's' },
    { "permissive",  0, NULL, 'p' },
    { "verbose",     0, NULL, 'v' },
    { "verbosity",   1, NULL, OPT_VERBOSITY },
    { "version",     0, NULL, 'V' },
    { "listen",      0, NULL, 'l' },
    { "foreground",  0, NULL, 'L' },
    { "address",     1, NULL, 'a' },
    { "blocksize",   1, NULL, 'B' },
    { "user",        1, NULL, 'u' },
    { "umask",       1, NULL, 'U' },
    { "refuse",      1, NULL, 'r' },
    { "timeout",     1, NULL, 't' },
    { "retransmit",  1, NULL, 'T' },
    { "port-range",  1, NULL, 'R' },
    { "map-file",    1, NULL, 'm' },
    { "pidfile",     1, NULL, 'P' },
    { NULL, 0, NULL, 0 }
};
static const char short_options[] = "46cspvVlLa:B:u:U:r:t:T:R:m:P:";

int main(int argc, char **argv)
{
    struct tftphdr *tp;
    struct passwd *pw;
    struct options *opt;
    union sock_addr myaddr;
    struct sockaddr_in bindaddr4;
#ifdef HAVE_IPV6
    struct sockaddr_in6 bindaddr6;
    int force_ipv6 = 0;
#endif
    int n;
    int fd = -1;
    int fd4 = -1;
    int fd6 = -1;
    int fdmax = 0;
    int standalone = 0;         /* Standalone (listen) mode */
    int nodaemon = 0;           /* Do not detach process */
    char *address = NULL;       /* Address to listen to */
    pid_t pid;
    mode_t my_umask = 0;
    int spec_umask = 0;
    int c;
    int setrv;
    int waittime = 900;         /* Default time to wait for a connect */
    const char *user = "nobody";        /* Default user */
    char *p, *ep;
#ifdef WITH_REGEX
    char *rewrite_file = NULL;
#endif
    const char *pidfile = NULL;
    u_short tp_opcode;

    /* basename() is way too much of a pain from a portability standpoint */

    p = strrchr(argv[0], '/');
    __progname = (p && p[1]) ? p + 1 : argv[0];

    openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

    srand(time(NULL) ^ getpid());

    while ((c = getopt_long(argc, argv, short_options, long_options, NULL))
           != -1)
        switch (c) {
        case '4':
            ai_fam = AF_INET;
            break;
#ifdef HAVE_IPV6
        case '6':
            ai_fam = AF_INET6;
            force_ipv6 = 1;
            break;
#endif
        case 'c':
            cancreate = 1;
            break;
        case 's':
            secure = 1;
            break;
        case 'p':
            unixperms = 1;
            break;
        case 'l':
            standalone = 1;
            break;
        case 'L':
            standalone = 1;
            nodaemon = 1;
            break;
        case 'a':
            address = optarg;
            break;
        case 't':
            waittime = atoi(optarg);
            break;
        case 'B':
            {
                char *vp;
                max_blksize = (unsigned int)strtoul(optarg, &vp, 10);
                if (max_blksize < 512 || max_blksize > MAX_SEGSIZE || *vp) {
                    syslog(LOG_ERR,
                           "Bad maximum blocksize value (range 512-%d): %s",
                           MAX_SEGSIZE, optarg);
                    exit(EX_USAGE);
                }
            }
            break;
        case 'T':
            {
                char *vp;
                unsigned long tov = strtoul(optarg, &vp, 10);
                if (tov < 10000UL || tov > 255000000UL || *vp) {
                    syslog(LOG_ERR, "Bad timeout value: %s", optarg);
                    exit(EX_USAGE);
                }
                rexmtval = timeout = tov;
                maxtimeout = rexmtval * TIMEOUT_LIMIT;
            }
            break;
        case 'R':
            {
                if (sscanf(optarg, "%u:%u", &portrange_from, &portrange_to)
                    != 2 || portrange_from > portrange_to
                    || portrange_to >= 65535) {
                    syslog(LOG_ERR, "Bad port range: %s", optarg);
                    exit(EX_USAGE);
                }
                portrange = 1;
            }
            break;
        case 'u':
            user = optarg;
            break;
        case 'U':
            my_umask = strtoul(optarg, &ep, 8);
            if (*ep) {
                syslog(LOG_ERR, "Invalid umask: %s", optarg);
                exit(EX_USAGE);
            }
            spec_umask = 1;
            break;
        case 'r':
            for (opt = options; opt->o_opt; opt++) {
                if (!strcasecmp(optarg, opt->o_opt)) {
                    opt->o_opt = "";    /* Don't support this option */
                    break;
                }
            }
            if (!opt->o_opt) {
                syslog(LOG_ERR, "Unknown option: %s", optarg);
                exit(EX_USAGE);
            }
            break;
#ifdef WITH_REGEX
        case 'm':
            if (rewrite_file) {
                syslog(LOG_ERR, "Multiple -m options");
                exit(EX_USAGE);
            }
            rewrite_file = optarg;
            break;
#endif
        case 'v':
            verbosity++;
            break;
        case OPT_VERBOSITY:
            verbosity = atoi(optarg);
            break;
        case 'V':
            /* Print configuration to stdout and exit */
            printf("%s\n", TFTPD_CONFIG_STR);
            exit(0);
            break;
        case 'P':
            pidfile = optarg;
            break;
        default:
            syslog(LOG_ERR, "Unknown option: '%c'", optopt);
            break;
        }

    dirs = xmalloc((argc - optind + 1) * sizeof(char *));
    for (ndirs = 0; optind != argc; optind++)
        dirs[ndirs++] = argv[optind];

    dirs[ndirs] = NULL;

    if (secure) {
        if (ndirs == 0) {
            syslog(LOG_ERR, "no -s directory");
            exit(EX_USAGE);
        }
        if (ndirs > 1) {
            syslog(LOG_ERR, "too many -s directories");
            exit(EX_USAGE);
        }
        if (chdir(dirs[0])) {
            syslog(LOG_ERR, "%s: %m", dirs[0]);
            exit(EX_NOINPUT);
        }
    }

    pw = getpwnam(user);
    if (!pw) {
        syslog(LOG_ERR, "no user %s: %m", user);
        exit(EX_NOUSER);
    }

#ifdef WITH_REGEX
    if (rewrite_file)
        rewrite_rules = read_remap_rules(rewrite_file);
#endif

    if (pidfile && !standalone) {
        syslog(LOG_WARNING, "not in standalone mode, ignoring pid file");
        pidfile = NULL;
    }

    /* If we're running standalone, set up the input port */
    if (standalone) {
        FILE *pf;
#ifdef HAVE_IPV6
        if (ai_fam != AF_INET6) {
#endif
            fd4 = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd4 < 0) {
                syslog(LOG_ERR, "cannot open IPv4 socket: %m");
                exit(EX_OSERR);
            }
#ifndef __CYGWIN__
            set_socket_nonblock(fd4, 1);
#endif
            memset(&bindaddr4, 0, sizeof bindaddr4);
            bindaddr4.sin_family = AF_INET;
            bindaddr4.sin_addr.s_addr = INADDR_ANY;
            bindaddr4.sin_port = htons(IPPORT_TFTP);
#ifdef HAVE_IPV6
        }
        if (ai_fam != AF_INET) {
            fd6 = socket(AF_INET6, SOCK_DGRAM, 0);
            if (fd6 < 0) {
                if (fd4 < 0) {
                    syslog(LOG_ERR, "cannot open IPv6 socket: %m");
                    exit(EX_OSERR);
                } else {
                    syslog(LOG_ERR,
                           "cannot open IPv6 socket, disable IPv6: %m");
                }
            }
#ifndef __CYGWIN__
            set_socket_nonblock(fd6, 1);
#endif
            memset(&bindaddr6, 0, sizeof bindaddr6);
            bindaddr6.sin6_family = AF_INET6;
            bindaddr6.sin6_port = htons(IPPORT_TFTP);
        }
#endif
        if (address) {
            char *portptr = NULL, *eportptr;
            int err;
            struct servent *servent;
            unsigned long port;

            address = tfstrdup(address);
            err = split_port(&address, &portptr);
            switch (err) {
            case AF_INET:
#ifdef HAVE_IPV6
                if (fd6 >= 0) {
                    close(fd6);
                    fd6 = -1;
                    if (ai_fam == AF_INET6) {
                        syslog(LOG_ERR,
                               "Address %s is not in address family AF_INET6",
                               address);
                        exit(EX_USAGE);
                    }
                    ai_fam = AF_INET;
                }
                break;
            case AF_INET6:
                if (fd4 >= 0) {
                    close(fd4);
                    fd4 = -1;
                    if (ai_fam == AF_INET) {
                        syslog(LOG_ERR,
                               "Address %s is not in address family AF_INET",
                               address);
                        exit(EX_USAGE);
                    }
                    ai_fam = AF_INET6;
                }
                break;
#endif
            case AF_UNSPEC:
                break;
            default:
                syslog(LOG_ERR,
                       "Numeric IPv6 addresses need to be enclosed in []");
                exit(EX_USAGE);
            }
            if (!portptr)
                portptr = (char *)"tftp";
            if (*address) {
                if (fd4 >= 0) {
                    bindaddr4.sin_family = AF_INET;
                    err = set_sock_addr(address,
                                        (union sock_addr *)&bindaddr4, NULL);
                    if (err) {
                        syslog(LOG_ERR,
                               "cannot resolve local IPv4 bind address: %s, %s",
                               address, gai_strerror(err));
                        exit(EX_NOINPUT);
                    }
                }
#ifdef HAVE_IPV6
                if (fd6 >= 0) {
                    bindaddr6.sin6_family = AF_INET6;
                    err = set_sock_addr(address,
                                        (union sock_addr *)&bindaddr6, NULL);
                    if (err) {
                        if (fd4 >= 0) {
                            syslog(LOG_ERR,
                                   "cannot resolve local IPv6 bind address: %s"
                                   "(%s); using IPv4 only",
                                   address, gai_strerror(err));
                            close(fd6);
                            fd6 = -1;
                        } else {
                            syslog(LOG_ERR,
                                   "cannot resolve local IPv6 bind address: %s"
                                   "(%s)", address, gai_strerror(err));
                            exit(EX_NOINPUT);
                        }
                    }
                }
#endif
            } else {
                /* Default to using INADDR_ANY */
            }

            if (portptr && *portptr) {
                servent = getservbyname(portptr, "udp");
                if (servent) {
                    if (fd4 >= 0)
                        bindaddr4.sin_port = servent->s_port;
#ifdef HAVE_IPV6
                    if (fd6 >= 0)
                        bindaddr6.sin6_port = servent->s_port;
#endif
                } else if ((port = strtoul(portptr, &eportptr, 0))
                           && !*eportptr) {
                    if (fd4 >= 0)
                        bindaddr4.sin_port = htons(port);
#ifdef HAVE_IPV6
                    if (fd6 >= 0)
                        bindaddr6.sin6_port = htons(port);
#endif
                } else if (!strcmp(portptr, "tftp")) {
                    /* It's TFTP, we're OK */
                } else {
                    syslog(LOG_ERR, "cannot resolve local bind port: %s",
                           portptr);
                    exit(EX_NOINPUT);
                }
            }
        }

        if (fd4 >= 0) {
            if (bind(fd4, (struct sockaddr *)&bindaddr4,
              sizeof(bindaddr4)) < 0) {
                syslog(LOG_ERR, "cannot bind to local IPv4 socket: %m");
                exit(EX_OSERR);
            }
        }
#ifdef HAVE_IPV6
        if (fd6 >= 0) {
#if defined(IPV6_V6ONLY)
            int on = 1;
            if (fd4 >= 0 || force_ipv6)
                if (setsockopt(fd6, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&on,
                  sizeof(on)))
                    syslog(LOG_ERR, "cannot setsockopt IPV6_V6ONLY %m");
#endif
            if (bind(fd6, (struct sockaddr *)&bindaddr6,
              sizeof(bindaddr6)) < 0) {
                if (fd4 >= 0) {
                    syslog(LOG_ERR,
                           "cannot bind to local IPv6 socket,"
                           "IPv6 disabled: %m");
                    close(fd6);
                    fd6 = -1;
                } else {
                    syslog(LOG_ERR, "cannot bind to local IPv6 socket: %m");
                    exit(EX_OSERR);
                }
            }
        }
#endif
        /* Daemonize this process */
        /* Note: when running in secure mode (-s), we must not chdir, since
           we are already in the proper directory. */
        if (!nodaemon && daemon(secure, 0) < 0) {
            syslog(LOG_ERR, "cannot daemonize: %m");
            exit(EX_OSERR);
        }
        set_signal(SIGTERM, handle_exit, 0);
        set_signal(SIGINT,  handle_exit, 0);
        if (pidfile) {
            pf = fopen (pidfile, "w");
            if (!pf) {
                syslog(LOG_ERR, "cannot open pid file '%s' for writing: %m", pidfile);
                pidfile = NULL;
            } else {
                if (fprintf(pf, "%d\n", getpid()) < 0)
                    syslog(LOG_ERR, "error writing pid file '%s': %m", pidfile);
                if (fclose(pf))
                    syslog(LOG_ERR, "error closing pid file '%s': %m", pidfile);
            }
        }
        if (fd6 > fd4)
            fdmax = fd6;
        else
            fdmax = fd4;
    } else {
        /* 0 is our socket descriptor */
        close(1);
        close(2);
        fd = 0;
        fdmax = 0;
        /* Note: on Cygwin, select() on a nonblocking socket becomes
           a nonblocking select. */
#ifndef __CYGWIN__
        set_socket_nonblock(fd, 1);
#endif
    }

    /* Disable path MTU discovery */
    pmtu_discovery_off(fd);

    /* This means we don't want to wait() for children */
#ifdef SA_NOCLDWAIT
    set_signal(SIGCHLD, SIG_IGN, SA_NOCLDSTOP | SA_NOCLDWAIT);
#else
    set_signal(SIGCHLD, SIG_IGN, SA_NOCLDSTOP);
#endif

    /* Take SIGHUP and use it to set a variable.  This
       is polled synchronously to make sure we don't
       lose packets as a result. */
    set_signal(SIGHUP, handle_sighup, 0);

    if (spec_umask || !unixperms)
        umask(my_umask);

    while (1) {
        fd_set readset;
        struct timeval tv_waittime;
        int rv;

        if (exit_signal) { /* happens in standalone mode only */
            if (pidfile && unlink(pidfile)) {
                syslog(LOG_WARNING, "error removing pid file '%s': %m", pidfile);
                exit(EX_OSERR);
            } else {
                exit(0);
            }
	}

        if (caught_sighup) {
            caught_sighup = 0;
            if (standalone) {
#ifdef WITH_REGEX
                if (rewrite_file) {
                    freerules(rewrite_rules);
                    rewrite_rules = read_remap_rules(rewrite_file);
                }
#endif
            } else {
                /* Return to inetd for respawn */
                exit(0);
            }
        }

        FD_ZERO(&readset);
        if (standalone) {
            if (fd4 >= 0) {
                FD_SET(fd4, &readset);
#ifdef __CYGWIN__
                /* On Cygwin, select() on a nonblocking socket returns
                   immediately, with a rv of 0! */
                set_socket_nonblock(fd4, 0);
#endif
            }
            if (fd6 >= 0) {
                FD_SET(fd6, &readset);
#ifdef __CYGWIN__
                /* On Cygwin, select() on a nonblocking socket returns
                   immediately, with a rv of 0! */
                set_socket_nonblock(fd6, 0);
#endif
            }
        } else { /* fd always 0 */
            fd = 0;
#ifdef __CYGWIN__
            /* On Cygwin, select() on a nonblocking socket returns
               immediately, with a rv of 0! */
            set_socket_nonblock(fd, 0);
#endif
            FD_SET(fd, &readset);
        }
        tv_waittime.tv_sec = waittime;
        tv_waittime.tv_usec = 0;


        /* Never time out if we're in standalone mode */
        rv = select(fdmax + 1, &readset, NULL, NULL,
                    standalone ? NULL : &tv_waittime);
        if (rv == -1 && errno == EINTR)
            continue;           /* Signal caught, reloop */

        if (rv == -1) {
            syslog(LOG_ERR, "select loop: %m");
            exit(EX_IOERR);
        } else if (rv == 0) {
            exit(0);            /* Timeout, return to inetd */
        }

        if (standalone) {
            if ((fd4 >= 0) && FD_ISSET(fd4, &readset))
                fd = fd4;
            else if ((fd6 >= 0) && FD_ISSET(fd6, &readset))
                fd = fd6;
            else /* not in set ??? */
                continue;
        }
#ifdef __CYGWIN__
        /* On Cygwin, select() on a nonblocking socket returns
           immediately, with a rv of 0! */
        set_socket_nonblock(fd, 0);
#endif

        fromlen = sizeof(from);
        n = myrecvfrom(fd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&from, &fromlen, &myaddr);

        if (n < 0) {
            if (E_WOULD_BLOCK(errno) || errno == EINTR) {
                continue;       /* Again, from the top */
            } else {
                syslog(LOG_ERR, "recvfrom: %m");
                exit(EX_IOERR);
            }
        }
#ifdef HAVE_IPV6
        if ((from.sa.sa_family != AF_INET) && (from.sa.sa_family != AF_INET6)) {
            syslog(LOG_ERR, "received address was not AF_INET/AF_INET6,"
                   " please check your inetd config");
#else
        if (from.sa.sa_family != AF_INET) {
            syslog(LOG_ERR, "received address was not AF_INET,"
                   " please check your inetd config");
#endif
            exit(EX_PROTOCOL);
        }

        if (standalone) {
            if ((from.sa.sa_family == AF_INET) &&
              (myaddr.si.sin_addr.s_addr == INADDR_ANY)) {
                /* myrecvfrom() didn't capture the source address; but we might
                   have bound to a specific address, if so we should use it */
                memcpy(SOCKADDR_P(&myaddr), &bindaddr4.sin_addr,
                       sizeof(bindaddr4.sin_addr));
#ifdef HAVE_IPV6
            } else if ((from.sa.sa_family == AF_INET6) &&
		       IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *)
					       SOCKADDR_P(&myaddr))) {
		memcpy(SOCKADDR_P(&myaddr), &bindaddr6.sin6_addr,
		       sizeof(bindaddr6.sin6_addr));
#endif
            }
        }

        /*
         * Now that we have read the request packet from the UDP
         * socket, we fork and go back to listening to the socket.
         */
        pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "fork: %m");
            exit(EX_OSERR);     /* Return to inetd, just in case */
        } else if (pid == 0)
            break;              /* Child exit, parent loop */
    }

    /* Child process: handle the actual request here */

    /* Ignore SIGHUP */
    set_signal(SIGHUP, SIG_IGN, 0);

    /* Make sure the log socket is still connected.  This has to be
       done before the chroot, while /dev/log is still accessible.
       When not running standalone, there is little chance that the
       syslog daemon gets restarted by the time we get here. */
    if (secure && standalone) {
        closelog();
        openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
    }

#ifdef HAVE_TCPWRAPPERS
    /* Verify if this was a legal request for us.  This has to be
       done before the chroot, while /etc is still accessible. */
    request_init(&wrap_request,
                 RQ_DAEMON, __progname,
                 RQ_FILE, fd,
                 RQ_CLIENT_SIN, &from, RQ_SERVER_SIN, &myaddr, 0);
    sock_methods(&wrap_request);

    tmp_p = (char *)inet_ntop(myaddr.sa.sa_family, SOCKADDR_P(&myaddr),
                              tmpbuf, INET6_ADDRSTRLEN);
    if (!tmp_p) {
        tmp_p = tmpbuf;
        strcpy(tmpbuf, "???");
    }
    if (hosts_access(&wrap_request) == 0) {
        if (deny_severity != -1)
            syslog(deny_severity, "connection refused from %s", tmp_p);
        exit(EX_NOPERM);        /* Access denied */
    } else if (allow_severity != -1) {
        syslog(allow_severity, "connect from %s", tmp_p);
    }
#endif

    /* Close file descriptors we don't need */
    close(fd);

    /* Get a socket.  This has to be done before the chroot(), since
       some systems require access to /dev to create a socket. */

    peer = socket(myaddr.sa.sa_family, SOCK_DGRAM, 0);
    if (peer < 0) {
        syslog(LOG_ERR, "socket: %m");
        exit(EX_IOERR);
    }

    /* Set up the supplementary group access list if possible */
    /* /etc/group still need to be accessible at this point */
#ifdef HAVE_INITGROUPS
    setrv = initgroups(user, pw->pw_gid);
    if (setrv) {
        syslog(LOG_ERR, "cannot set groups for user %s", user);
        exit(EX_OSERR);
    }
#else
#ifdef HAVE_SETGROUPS
    if (setgroups(0, NULL)) {
        syslog(LOG_ERR, "cannot clear group list");
    }
#endif
#endif

    /* Chroot and drop privileges */
    if (secure) {
        if (chroot(".")) {
            syslog(LOG_ERR, "chroot: %m");
            exit(EX_OSERR);
        }
#ifdef __CYGWIN__
        chdir("/");             /* Cygwin chroot() bug workaround */
#endif
    }
#ifdef HAVE_SETREGID
    setrv = setregid(pw->pw_gid, pw->pw_gid);
#else
    setrv = setegid(pw->pw_gid) || setgid(pw->pw_gid);
#endif

#ifdef HAVE_SETREUID
    setrv = setrv || setreuid(pw->pw_uid, pw->pw_uid);
#else
    /* Important: setuid() must come first */
    setrv = setrv || setuid(pw->pw_uid) ||
        (geteuid() != pw->pw_uid && seteuid(pw->pw_uid));
#endif

    if (setrv) {
        syslog(LOG_ERR, "cannot drop privileges: %m");
        exit(EX_OSERR);
    }

    /* Process the request... */
    if (pick_port_bind(peer, &myaddr, portrange_from, portrange_to) < 0) {
        syslog(LOG_ERR, "bind: %m");
        exit(EX_IOERR);
    }

    if (connect(peer, &from.sa, SOCKLEN(&from)) < 0) {
        syslog(LOG_ERR, "connect: %m");
        exit(EX_IOERR);
    }

    /* Disable path MTU discovery */
    pmtu_discovery_off(peer);

    tp = (struct tftphdr *)buf;
    tp_opcode = ntohs(tp->th_opcode);
    if (tp_opcode == RRQ || tp_opcode == WRQ)
        tftp(tp, n);
    exit(0);
}

static char *rewrite_access(char *, int, const char **);
static int validate_access(char *, int, const struct formats *, const char **);
static void tftp_sendfile(const struct formats *, struct tftphdr *, int);
static void tftp_recvfile(const struct formats *, struct tftphdr *, int);

struct formats {
    const char *f_mode;
    char *(*f_rewrite) (char *, int, const char **);
    int (*f_validate) (char *, int, const struct formats *, const char **);
    void (*f_send) (const struct formats *, struct tftphdr *, int);
    void (*f_recv) (const struct formats *, struct tftphdr *, int);
    int f_convert;
};
static const struct formats formats[] = {
    {
    "netascii", rewrite_access, validate_access, tftp_sendfile,
            tftp_recvfile, 1}, {
    "octet", rewrite_access, validate_access, tftp_sendfile,
            tftp_recvfile, 0}, {
    NULL, NULL, NULL, NULL, NULL, 0}
};

/*
 * Handle initial connection protocol.
 */
int tftp(struct tftphdr *tp, int size)
{
    char *cp, *end;
    int argn, ecode;
    const struct formats *pf = NULL;
    char *origfilename;
    char *filename, *mode = NULL;
    const char *errmsgptr;
    u_short tp_opcode = ntohs(tp->th_opcode);

    char *val = NULL, *opt = NULL;
    char *ap = ackbuf + 2;

    ((struct tftphdr *)ackbuf)->th_opcode = htons(OACK);

    origfilename = cp = (char *)&(tp->th_stuff);
    argn = 0;

    end = (char *)tp + size;

    while (cp < end && *cp) {
        do {
            cp++;
        } while (cp < end && *cp);

        if (*cp) {
            nak(EBADOP, "Request not null-terminated");
            exit(0);
        }

        argn++;
        if (argn == 1) {
            mode = ++cp;
        } else if (argn == 2) {
            for (cp = mode; *cp; cp++)
                *cp = tolower(*cp);
            for (pf = formats; pf->f_mode; pf++) {
                if (!strcmp(pf->f_mode, mode))
                    break;
            }
            if (!pf->f_mode) {
                nak(EBADOP, "Unknown mode");
                exit(0);
            }
            if (!(filename =
                  (*pf->f_rewrite) (origfilename, tp_opcode,
                                    &errmsgptr))) {
                nak(EACCESS, errmsgptr);        /* File denied by mapping rule */
                exit(0);
            }
            if (verbosity >= 1) {
                tmp_p = (char *)inet_ntop(from.sa.sa_family, SOCKADDR_P(&from),
                                          tmpbuf, INET6_ADDRSTRLEN);
                if (!tmp_p) {
                    tmp_p = tmpbuf;
                    strcpy(tmpbuf, "???");
                }
                if (filename == origfilename
                    || !strcmp(filename, origfilename))
                    syslog(LOG_NOTICE, "%s from %s filename %s\n",
                           tp_opcode == WRQ ? "WRQ" : "RRQ",
                           tmp_p, filename);
                else
                    syslog(LOG_NOTICE,
                           "%s from %s filename %s remapped to %s\n",
                           tp_opcode == WRQ ? "WRQ" : "RRQ",
                           tmp_p, origfilename,
                           filename);
            }
            ecode =
                (*pf->f_validate) (filename, tp_opcode, pf, &errmsgptr);
            if (ecode) {
                nak(ecode, errmsgptr);
                exit(0);
            }
            opt = ++cp;
        } else if (argn & 1) {
            val = ++cp;
        } else {
            do_opt(opt, val, &ap);
            opt = ++cp;
        }
    }

    if (!pf) {
        nak(EBADOP, "Missing mode");
        exit(0);
    }

    if (ap != (ackbuf + 2)) {
        if (tp_opcode == WRQ)
            (*pf->f_recv) (pf, (struct tftphdr *)ackbuf, ap - ackbuf);
        else
            (*pf->f_send) (pf, (struct tftphdr *)ackbuf, ap - ackbuf);
    } else {
        if (tp_opcode == WRQ)
            (*pf->f_recv) (pf, NULL, 0);
        else
            (*pf->f_send) (pf, NULL, 0);
    }
    exit(0);                    /* Request completed */
}

static int blksize_set;

/*
 * Set a non-standard block size (c.f. RFC2348)
 */
static int set_blksize(uintmax_t *vp)
{
    uintmax_t sz = *vp;

    if (blksize_set)
        return 0;

    if (sz < 8)
        return 0;
    else if (sz > max_blksize)
        sz = max_blksize;

    *vp = segsize = sz;
    blksize_set = 1;
    return 1;
}

/*
 * Set a power-of-two block size (nonstandard)
 */
static int set_blksize2(uintmax_t *vp)
{
    uintmax_t sz = *vp;

    if (blksize_set)
        return 0;

    if (sz < 8)
        return (0);
    else if (sz > max_blksize)
        sz = max_blksize;
    else

    /* Convert to a power of two */
    if (sz & (sz - 1)) {
        unsigned int sz1 = 1;
        /* Not a power of two - need to convert */
        while (sz >>= 1)
            sz1 <<= 1;
        sz = sz1;
    }

    *vp = segsize = sz;
    blksize_set = 1;
    return 1;
}

/*
 * Set the block number rollover value
 */
static int set_rollover(uintmax_t *vp)
{
    uintmax_t ro = *vp;
    
    if (ro > 65535)
	return 0;

    rollover_val = (uint16_t)ro;
    return 1;
}

/*
 * Return a file size (c.f. RFC2349)
 * For netascii mode, we don't know the size ahead of time;
 * so reject the option.
 */
static int set_tsize(uintmax_t *vp)
{
    uintmax_t sz = *vp;

    if (!tsize_ok)
        return 0;

    if (sz == 0)
        sz = tsize;

    *vp = sz;
    return 1;
}

/*
 * Set the timeout (c.f. RFC2349).  This is supposed
 * to be the (default) retransmission timeout, but being an
 * integer in seconds it seems a bit limited.
 */
static int set_timeout(uintmax_t *vp)
{
    uintmax_t to = *vp;

    if (to < 1 || to > 255)
        return 0;

    rexmtval = timeout = to * 1000000UL;
    maxtimeout = rexmtval * TIMEOUT_LIMIT;

    return 1;
}

/* Similar, but in microseconds.  We allow down to 10 ms. */
static int set_utimeout(uintmax_t *vp)
{
    uintmax_t to = *vp;

    if (to < 10000UL || to > 255000000UL)
        return 0;

    rexmtval = timeout = to;
    maxtimeout = rexmtval * TIMEOUT_LIMIT;

    return 1;
}

/*
 * Conservative calculation for the size of a buffer which can hold an
 * arbitrary integer
 */
#define OPTBUFSIZE	(sizeof(uintmax_t) * CHAR_BIT / 3 + 3)

/*
 * Parse RFC2347 style options; we limit the arguments to positive
 * integers which matches all our current options.
 */
static void do_opt(const char *opt, const char *val, char **ap)
{
    struct options *po;
    char retbuf[OPTBUFSIZE];
    char *p = *ap;
    size_t optlen, retlen;
    char *vend;
    uintmax_t v;

    /* Global option-parsing variables initialization */
    blksize_set = 0;

    if (!*opt || !*val)
        return;

    errno = 0;
    v = strtoumax(val, &vend, 10);
    if (*vend || errno == ERANGE)
	return;

    for (po = options; po->o_opt; po++)
        if (!strcasecmp(po->o_opt, opt)) {
            if (po->o_fnc(&v)) {
		optlen = strlen(opt);
		retlen = sprintf(retbuf, "%"PRIuMAX, v);

                if (p + optlen + retlen + 2 >= ackbuf + sizeof(ackbuf)) {
                    nak(EOPTNEG, "Insufficient space for options");
                    exit(0);
                }
		
		memcpy(p, opt, optlen+1);
		p += optlen+1;
		memcpy(p, retbuf, retlen+1);
		p += retlen+1;
            } else {
                nak(EOPTNEG, "Unsupported option(s) requested");
                exit(0);
            }
            break;
        }

    *ap = p;
}

#ifdef WITH_REGEX

/*
 * This is called by the remap engine when it encounters macros such
 * as \i.  It should write the output in "output" if non-NULL, and
 * return the length of the output (generated or not).
 *
 * Return -1 on failure.
 */
static int rewrite_macros(char macro, char *output)
{
    char *p, tb[INET6_ADDRSTRLEN];
    int l=0;

    switch (macro) {
    case 'i':
        p = (char *)inet_ntop(from.sa.sa_family, SOCKADDR_P(&from),
                              tb, INET6_ADDRSTRLEN);
        if (output && p)
            strcpy(output, p);
        if (!p)
            return 0;
        else
            return strlen(p);

    case 'x':
        if (output) {
            if (from.sa.sa_family == AF_INET) {
                sprintf(output, "%08lX",
                    (unsigned long)ntohl(from.si.sin_addr.s_addr));
                l = 8;
#ifdef HAVE_IPV6
            } else {
                unsigned char *c = (unsigned char *)SOCKADDR_P(&from);
                p = tb;
                for (l = 0; l < 16; l++) {
                    sprintf(p, "%02X", *c);
                    c++;
                    p += 2;
                }
                strcpy(output, tb);
                l = strlen(tb);
#endif
            }
        }
        return l;

    default:
        return -1;
    }
}

/*
 * Modify the filename, if applicable.  If it returns NULL, deny the access.
 */
static char *rewrite_access(char *filename, int mode, const char **msg)
{
    if (rewrite_rules) {
        char *newname =
            rewrite_string(filename, rewrite_rules,
			   mode != RRQ ? 'P' : 'G',
                           rewrite_macros, msg);
        filename = newname;
    }
    return filename;
}

#else
static char *rewrite_access(char *filename, int mode, const char **msg)
{
    (void)mode;                 /* Avoid warning */
    (void)msg;
    return filename;
}
#endif

static FILE *file;
/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable, unless -p specified.
 * If we were invoked with arguments
 * from inetd then the file must also be
 * in one of the given directory prefixes.
 * Note also, full path name must be
 * given as we have no login directory.
 */
static int validate_access(char *filename, int mode,
			   const struct formats *pf, const char **errmsg)
{
    struct stat stbuf;
    int i, len;
    int fd, wmode, rmode;
    char *cp;
    const char **dirp;
    char stdio_mode[3];

    tsize_ok = 0;
    *errmsg = NULL;

    if (!secure) {
        if (*filename != '/') {
            *errmsg = "Only absolute filenames allowed";
            return (EACCESS);
        }

        /*
         * prevent tricksters from getting around the directory
         * restrictions
         */
        len = strlen(filename);
        for (i = 1; i < len - 3; i++) {
            cp = filename + i;
            if (*cp == '.' && memcmp(cp - 1, "/../", 4) == 0) {
                *errmsg = "Reverse path not allowed";
                return (EACCESS);
            }
        }

        for (dirp = dirs; *dirp; dirp++)
            if (strncmp(filename, *dirp, strlen(*dirp)) == 0)
                break;
        if (*dirp == 0 && dirp != dirs) {
            *errmsg = "Forbidden directory";
            return (EACCESS);
        }
    }

    /*
     * We use different a different permissions scheme if `cancreate' is
     * set.
     */
    wmode = O_WRONLY | (cancreate ? O_CREAT : 0) | (pf->f_convert ? O_TEXT : O_BINARY);
    rmode = O_RDONLY | (pf->f_convert ? O_TEXT : O_BINARY);

#ifndef HAVE_FTRUNCATE
    wmode |= O_TRUNC;		/* This really sucks on a dupe */
#endif

    fd = open(filename, mode == RRQ ? rmode : wmode, 0666);
    if (fd < 0) {
        switch (errno) {
        case ENOENT:
        case ENOTDIR:
            return ENOTFOUND;
        case ENOSPC:
            return ENOSPACE;
        case EEXIST:
            return EEXISTS;
        default:
            return errno + 100;
        }
    }

    if (fstat(fd, &stbuf) < 0)
        exit(EX_OSERR);         /* This shouldn't happen */

    /* A duplicate RRQ or (worse!) WRQ packet could really cause havoc... */
    if (lock_file(fd, mode != RRQ))
	exit(0);

    if (mode == RRQ) {
        if (!unixperms && (stbuf.st_mode & (S_IREAD >> 6)) == 0) {
            *errmsg = "File must have global read permissions";
            return (EACCESS);
        }
        tsize = stbuf.st_size;
        /* We don't know the tsize if conversion is needed */
        tsize_ok = !pf->f_convert;
    } else {
        if (!unixperms) {
            if ((stbuf.st_mode & (S_IWRITE >> 6)) == 0) {
                *errmsg = "File must have global write permissions";
                return (EACCESS);
            }
        }

#ifdef HAVE_FTRUNCATE
	/* We didn't get to truncate the file at open() time */
	if (ftruncate(fd, (off_t) 0)) {
	  *errmsg = "Cannot reset file size";
	  return (EACCESS);
	}
#endif
        tsize = 0;
        tsize_ok = 1;
    }

    stdio_mode[0] = (mode == RRQ) ? 'r' : 'w';
    stdio_mode[1] = (pf->f_convert) ? 't' : 'b';
    stdio_mode[2] = '\0';

    file = fdopen(fd, stdio_mode);
    if (file == NULL)
        exit(EX_OSERR);         /* Internal error */

    return (0);
}

/*
 * Send the requested file.
 */
static void tftp_sendfile(const struct formats *pf, struct tftphdr *oap, int oacklen)
{
    struct tftphdr *dp;
    struct tftphdr *ap;         /* ack packet */
    static u_short block = 1;   /* Static to avoid longjmp funnies */
    u_short ap_opcode, ap_block;
    unsigned long r_timeout;
    int size, n;

    if (oap) {
        timeout = rexmtval;
        (void)sigsetjmp(timeoutbuf, 1);
      oack:
        r_timeout = timeout;
        if (send(peer, oap, oacklen, 0) != oacklen) {
            syslog(LOG_WARNING, "tftpd: oack: %m\n");
            goto abort;
        }
        for (;;) {
            n = recv_time(peer, ackbuf, sizeof(ackbuf), 0, &r_timeout);
            if (n < 0) {
                syslog(LOG_WARNING, "tftpd: read: %m\n");
                goto abort;
            }
            ap = (struct tftphdr *)ackbuf;
            ap_opcode = ntohs((u_short) ap->th_opcode);
            ap_block = ntohs((u_short) ap->th_block);

            if (ap_opcode == ERROR) {
                syslog(LOG_WARNING,
                       "tftp: client does not accept options\n");
                goto abort;
            }
            if (ap_opcode == ACK) {
                if (ap_block == 0)
                    break;
                /* Resynchronize with the other side */
                (void)synchnet(peer);
                goto oack;
            }
        }
    }

    dp = r_init();
    do {
        size = readit(file, &dp, pf->f_convert);
        if (size < 0) {
            nak(errno + 100, NULL);
            goto abort;
        }
        dp->th_opcode = htons((u_short) DATA);
        dp->th_block = htons((u_short) block);
        timeout = rexmtval;
        (void)sigsetjmp(timeoutbuf, 1);

        r_timeout = timeout;
        if (send(peer, dp, size + 4, 0) != size + 4) {
            syslog(LOG_WARNING, "tftpd: write: %m");
            goto abort;
        }
        read_ahead(file, pf->f_convert);
        for (;;) {
            n = recv_time(peer, ackbuf, sizeof(ackbuf), 0, &r_timeout);
            if (n < 0) {
                syslog(LOG_WARNING, "tftpd: read(ack): %m");
                goto abort;
            }
            ap = (struct tftphdr *)ackbuf;
            ap_opcode = ntohs((u_short) ap->th_opcode);
            ap_block = ntohs((u_short) ap->th_block);

            if (ap_opcode == ERROR)
                goto abort;

            if (ap_opcode == ACK) {
                if (ap_block == block) {
                    break;
                }
                /* Re-synchronize with the other side */
                (void)synchnet(peer);
                /*
                 * RFC1129/RFC1350: We MUST NOT re-send the DATA
                 * packet in response to an invalid ACK.  Doing so
                 * would cause the Sorcerer's Apprentice bug.
                 */
            }

        }
	if (!++block)
	  block = rollover_val;
    } while (size == segsize);
  abort:
    (void)fclose(file);
}

/*
 * Receive a file.
 */
static void tftp_recvfile(const struct formats *pf, struct tftphdr *oap, int oacklen)
{
    struct tftphdr *dp;
    int n, size;
    /* These are "static" to avoid longjmp funnies */
    static struct tftphdr *ap;  /* ack buffer */
    static u_short block = 0;
    static int acksize;
    u_short dp_opcode, dp_block;
    unsigned long r_timeout;

    dp = w_init();
    do {
        timeout = rexmtval;

        if (!block && oap) {
            ap = (struct tftphdr *)ackbuf;
            acksize = oacklen;
        } else {
            ap = (struct tftphdr *)ackbuf;
            ap->th_opcode = htons((u_short) ACK);
            ap->th_block = htons((u_short) block);
            acksize = 4;
            /* If we're sending a regular ACK, that means we have successfully
             * sent the OACK. Clear oap so that we won't try to send another
             * OACK when the block number wraps back to 0. */
            oap = NULL;
        }
        if (!++block)
	  block = rollover_val;
        (void)sigsetjmp(timeoutbuf, 1);
      send_ack:
        r_timeout = timeout;
        if (send(peer, ackbuf, acksize, 0) != acksize) {
            syslog(LOG_WARNING, "tftpd: write(ack): %m");
            goto abort;
        }
        write_behind(file, pf->f_convert);
        for (;;) {
            n = recv_time(peer, dp, PKTSIZE, 0, &r_timeout);
            if (n < 0) {        /* really? */
                syslog(LOG_WARNING, "tftpd: read: %m");
                goto abort;
            }
            dp_opcode = ntohs((u_short) dp->th_opcode);
            dp_block = ntohs((u_short) dp->th_block);
            if (dp_opcode == ERROR)
                goto abort;
            if (dp_opcode == DATA) {
                if (dp_block == block) {
                    break;      /* normal */
                }
                /* Re-synchronize with the other side */
                (void)synchnet(peer);
                if (dp_block == (block - 1))
                    goto send_ack;      /* rexmit */
            }
        }
        /*  size = write(file, dp->th_data, n - 4); */
        size = writeit(file, &dp, n - 4, pf->f_convert);
        if (size != (n - 4)) {  /* ahem */
            if (size < 0)
                nak(errno + 100, NULL);
            else
                nak(ENOSPACE, NULL);
            goto abort;
        }
    } while (size == segsize);
    write_behind(file, pf->f_convert);
    (void)fclose(file);         /* close data file */

    ap->th_opcode = htons((u_short) ACK);       /* send the "final" ack */
    ap->th_block = htons((u_short) (block));
    (void)send(peer, ackbuf, 4, 0);

    timeout_quit = 1;           /* just quit on timeout */
    n = recv_time(peer, buf, sizeof(buf), 0, &timeout); /* normally times out and quits */
    timeout_quit = 0;

    if (n >= 4 &&               /* if read some data */
        dp_opcode == DATA &&    /* and got a data block */
        block == dp_block) {    /* then my last ack was lost */
        (void)send(peer, ackbuf, 4, 0); /* resend final ack */
    }
  abort:
    return;
}

static const char *const errmsgs[] = {
    "Undefined error code",     /* 0 - EUNDEF */
    "File not found",           /* 1 - ENOTFOUND */
    "Access denied",            /* 2 - EACCESS */
    "Disk full or allocation exceeded", /* 3 - ENOSPACE */
    "Illegal TFTP operation",   /* 4 - EBADOP */
    "Unknown transfer ID",      /* 5 - EBADID */
    "File already exists",      /* 6 - EEXISTS */
    "No such user",             /* 7 - ENOUSER */
    "Failure to negotiate RFC2347 options"      /* 8 - EOPTNEG */
};

#define ERR_CNT (sizeof(errmsgs)/sizeof(const char *))

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
static void nak(int error, const char *msg)
{
    struct tftphdr *tp;
    int length;

    tp = (struct tftphdr *)buf;
    tp->th_opcode = htons((u_short) ERROR);

    if (error >= 100) {
        /* This is a Unix errno+100 */
        if (!msg)
            msg = strerror(error - 100);
        error = EUNDEF;
    } else {
        if ((unsigned)error >= ERR_CNT)
            error = EUNDEF;

        if (!msg)
            msg = errmsgs[error];
    }

    tp->th_code = htons((u_short) error);

    length = strlen(msg) + 1;
    memcpy(tp->th_msg, msg, length);
    length += 4;                /* Add space for header */

    if (verbosity >= 2) {
        tmp_p = (char *)inet_ntop(from.sa.sa_family, SOCKADDR_P(&from),
                                  tmpbuf, INET6_ADDRSTRLEN);
        if (!tmp_p) {
            tmp_p = tmpbuf;
            strcpy(tmpbuf, "???");
        }
        syslog(LOG_INFO, "sending NAK (%d, %s) to %s",
               error, tp->th_msg, tmp_p);
    }

    if (send(peer, buf, length, 0) != length)
        syslog(LOG_WARNING, "nak: %m");
}
