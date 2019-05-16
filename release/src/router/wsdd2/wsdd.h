/*
   WSDD - Web Service Dynamic Discovery protocol server

        Copyright (c) 2016 NETGEAR
        Copyright (c) 2016 Hiro Sugawara
  
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WSDD_H_
#define _WSDD_H_

#define _GNU_SOURCE

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <stdbool.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>

#include <time.h>
#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/rtnetlink.h>

extern int debug_L, debug_W;
#define DEBUG(x, y, ...)	\
	do {	\
		if (debug_##y >= (x)) {	\
			fprintf(stderr, __VA_ARGS__);	\
			putc('\n', stderr);		\
			syslog(LOG_USER|LOG_ERR, __VA_ARGS__);	\
		}		\
	} while (0)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX	255
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))
#endif

#ifndef max
#define max(a, b)	((a)>(b)?(a):(b))
#endif

#define _ADDRSTRLEN	max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)

typedef union {
	struct sockaddr_in	in;
	struct sockaddr_in6	in6;
	struct sockaddr_nl	nl;
	struct sockaddr_storage	ss;
} _saddr_t;

#define _SIN_ADDR(x)	(((x)->ss.ss_family == AF_INET) \
				? (void *)&(x)->in.sin_addr \
				: (void *)&(x)->in6.sin6_addr)

#define _SIN_PORT(x)	ntohs(((x)->ss.ss_family == AF_INET) \
				? (x)->in.sin_port \
				: (x)->in6.sin6_port)

struct endpoint {
	char ifname[IFNAMSIZ];
	struct endpoint *next;
	struct service *service;
	int family, type, protocol;
	in_port_t port;
	int sock;
	char *errstr;
	int _errno;
	size_t mlen, llen;
	_saddr_t mcast, local;
	union {
		struct ip_mreq ip_mreq;
		/* struct ip_mreqn ip_mreq; */
		struct ipv6_mreq ipv6_mreq;
	} mreq;
	size_t mreqlen;
};

struct service {
	const char *name;
	const int family, type, protocol;
	const char *port_name;
	const in_port_t port_num;
	const char *mcast_addr;

	const uint32_t nl_groups;

	int (*init)(struct endpoint *);
	int (*recv)(struct endpoint *);
	int (*timer)(struct endpoint *);
	void (*exit)(struct endpoint *);
	time_t interval;
};

extern int wsd_init(struct endpoint *);
extern int wsd_recv(struct endpoint *);
extern void wsd_exit(struct endpoint *);

extern int wsd_http(struct endpoint *);

extern int llmnr_init(struct endpoint *);
extern int llmnr_recv(struct endpoint *);
extern int llmnr_timer(struct endpoint *);
extern void llmnr_exit(struct endpoint *);

extern int connected_if(const _saddr_t *, _saddr_t *);
extern char *ip2uri(const char *);

extern int set_getresp(const char *, const char **);
extern void printBootInfoKeys(FILE *, int);

#endif
