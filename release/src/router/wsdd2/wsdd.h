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

#include <stdbool.h> // bool
#include <stdio.h> // FILE, fopen(), fprintf()
#include <syslog.h> // syslog()
#include <net/if.h> // IFNAMSIZ
#include <arpa/inet.h> // ntohs()
#include <netinet/in.h> // struct sockaddr_in, struct ip_mreq
#include <linux/in.h> // struct ip_mreqn
#include <linux/netlink.h> // struct sockaddr_nl
#include <time.h> // time_t, time()

/* wsdd2.c */
extern char *hostname, *hostaliases, *netbiosname, *netbiosaliases, *workgroup;
extern int debug_L, debug_W;
extern bool is_daemon;

#define LOG(level, ...)						\
	do {							\
		if (is_daemon) {				\
			syslog(LOG_USER | (level), __VA_ARGS__);\
		} else {					\
			fprintf(stderr, __VA_ARGS__);		\
			putc('\n', stderr);			\
		}						\
	} while (0)

#define DEBUG(x, y, ...)						\
	do {								\
		if (debug_##y >= (x)) {					\
			if (is_daemon) {				\
				syslog(LOG_USER | LOG_DEBUG, __VA_ARGS__);\
			} else {					\
				fprintf(stderr, __VA_ARGS__);		\
				putc('\n', stderr);			\
			}						\
		}							\
	} while (0)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX	255
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
#endif

#ifndef max
#define max(a, b)	((a)>(b)?(a):(b))
#endif

#define _ADDRSTRLEN	max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)

typedef union {
	struct sockaddr		sa;
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
	size_t mlen, llen, mreqlen;
	_saddr_t mcast, local;
	union {
#ifdef USE_ip_mreq
		struct ip_mreq ip_mreq;
#else
		struct ip_mreqn ip_mreq;
#endif
		struct ipv6_mreq ipv6_mreq;
	} mreq;
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

// wsd.c
int wsd_init(struct endpoint *);
int wsd_recv(struct endpoint *);
void wsd_exit(struct endpoint *);

void init_getresp(void);
const char *get_getresp(const char *key);
int set_getresp(const char *key, const char **endp);
void printBootInfoKeys(FILE *fp, int indent);

// llmnr.c
int llmnr_init(struct endpoint *);
int llmnr_recv(struct endpoint *);
void llmnr_exit(struct endpoint *);

// wsdd2.c
int connected_if(const _saddr_t *, _saddr_t *);
char *ip2uri(const char *);

// nl_debug.c
int nl_debug(void *buf, int len);
void dump(const void *p, size_t len, unsigned long start, const char *prefix);
void dump_str(const void *p, size_t len);
void dump_hex(const void *p, size_t len);

#endif
