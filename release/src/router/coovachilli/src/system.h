/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "../config.h"
#ifdef ENABLE_CONFIG
#include ENABLE_CONFIG
#endif

/*
 *   I do not like this here, but otherwise
 *   __u64 is not defined. Set by -ansi
#undef __STRICT_ANSI__
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#ifdef HAVE_SYS_MSG_H
#include <sys/msg.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_POLL_H
#include <poll.h>
#endif

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#if defined(__linux__)
#include <asm/types.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#ifndef HAVE_SYS_UN_H
#include <linux/un.h>
#endif

#elif defined (__FreeBSD__)  || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__) 
#include <net/if.h>
#include <net/bpf.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <ifaddrs.h>
#endif

#ifndef EIDRM
#define EIDRM   EINVAL
#endif
#ifndef ENOMSG
#define ENOMSG  EAGAIN
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if defined(HAVE_NET_IF_H) && !defined(__linux__)
#include <net/if.h>
#endif

#ifdef HAVE_NET_IF_TUN_H
#include <net/if_tun.h>
#endif

//#ifdef HAVE_NET_ETHERNET_H
//#include <net/ethernet.h>
//#endif

#ifdef HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif

#ifdef HAVE_NET_ROUTE_H
#include <net/route.h>
#endif

#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif

#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif

#ifdef MTRACE
#include <mcheck.h> 
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#undef LITTLE_ENDIAN
#undef BIG_ENDIAN

#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define LITTLE_ENDIAN 1
# define BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN) || \
      (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define LITTLE_ENDIAN 0
# define BIG_ENDIAN 1
#else
# define LITTLE_ENDIAN 0
# define BIG_ENDIAN 0
#endif

#ifndef SI_LOAD_SHIFT
#define SI_LOAD_SHIFT 16
#endif

#include <unistd.h>
#include <errno.h>

#define safe_snprintf portable_snprintf
char *safe_strncpy(char *dst, const char *src, size_t size);

int safe_accept(int fd, struct sockaddr *sa, socklen_t *lenptr);
int safe_select(int nfds, fd_set *readfds, fd_set *writefds,
		fd_set *exceptfds, struct timeval *timeout);
#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
int safe_epoll_wait(int epfd, struct epoll_event *events,
		    int maxevents, int timeout);
#else
int safe_poll(struct pollfd *fds, nfds_t nfds, int timeout);
#endif
#endif
int safe_connect(int s, struct sockaddr *sock, size_t len);
int safe_write(int s, void *b, size_t blen);
int safe_read(int s, void *b, size_t blen);
int safe_recv(int s, void *b, size_t blen, int flags);
int safe_send(int s, void *b, size_t blen, int flags);
int safe_recvmsg(int sockfd, struct msghdr *msg, int flags);
int safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
		  struct sockaddr *src_addr, socklen_t *addrlen);
int safe_sendto(int s, const void *b, size_t blen, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);
int safe_close (int fd);
pid_t safe_fork();

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
    ({ \
        long int _result; \
        do _result = (long int) (expression); \
        while (_result == -1L && errno == EINTR); \
        _result; \
    })
#endif

#define SET_SA_FAMILY(addr, family)			\
    memset ((char *) &(addr), '\0', sizeof(addr));	\
    addr.sa_family = (family);

void copy_mac6(uint8_t *, uint8_t *);

#ifndef USING_IPC_MSG
#define USING_IPC_UNIX
#endif

#if defined(HAVE_OPENSSL) || defined(HAVE_MATRIXSSL) || defined(HAVE_CYASSL)
#define HAVE_SSL 1
#endif

#ifdef HAVE_PATRICIA
#include "patricia.h"
#define HAVE_SEARCH 1
#endif

#ifdef HAVE_AVL
#include "avl/avl.h"
#define HAVE_SEARCH 1
#endif

#endif
