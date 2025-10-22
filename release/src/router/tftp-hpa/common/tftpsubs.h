/*
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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

/*
 * Prototypes for read-ahead/write-behind subroutines for tftp user and
 * server.
 */
#ifndef TFTPSUBS_H
#define TFTPSUBS_H

#include "config.h"

union sock_addr {
    struct sockaddr     sa;
    struct sockaddr_in  si;
#ifdef HAVE_IPV6
    struct sockaddr_in6 s6;
#endif
};

#define SOCKLEN(sock) \
    (((union sock_addr*)sock)->sa.sa_family == AF_INET ? \
    (sizeof(struct sockaddr_in)) : \
    (sizeof(union sock_addr)))

#ifdef HAVE_IPV6
#define SOCKPORT(sock) \
    (((union sock_addr*)sock)->sa.sa_family == AF_INET ? \
    ((union sock_addr*)sock)->si.sin_port : \
    ((union sock_addr*)sock)->s6.sin6_port)
#else
#define SOCKPORT(sock) \
    (((union sock_addr*)sock)->si.sin_port)
#endif

#ifdef HAVE_IPV6
#define SOCKADDR_P(sock) \
    (((union sock_addr*)sock)->sa.sa_family == AF_INET ? \
    (void *)&((union sock_addr*)sock)->si.sin_addr : \
    (void *)&((union sock_addr*)sock)->s6.sin6_addr)
#else
#define SOCKADDR_P(sock) \
    (void *)&((union sock_addr*)sock)->si.sin_addr
#endif

#ifdef HAVE_IPV6
int is_numeric_ipv6(const char *);
char *strip_address(char *);
#else
#define is_numeric_ipv6(a)      0
#define strip_address(a)	(a)
#endif

static inline int sa_set_port(union sock_addr *s, u_short port)
{
       switch (s->sa.sa_family) {
       case AF_INET:
               s->si.sin_port = port;
               break;
#ifdef HAVE_IPV6
       case AF_INET6:
               s->s6.sin6_port = port;
               break;
#endif
       default:
               return -1;
       }
       return 0;
}

int set_sock_addr(char *, union sock_addr *, char **);

struct tftphdr;

struct tftphdr *r_init(void);
void read_ahead(FILE *, int);
int readit(FILE *, struct tftphdr **, int);

int synchnet(int);

struct tftphdr *w_init(void);
int write_behind(FILE *, int);
int writeit(FILE *, struct tftphdr **, int, int);

extern int segsize;
#define MAX_SEGSIZE	65464

int pick_port_bind(int sockfd, union sock_addr *myaddr,
                   unsigned int from, unsigned int to);

#endif
