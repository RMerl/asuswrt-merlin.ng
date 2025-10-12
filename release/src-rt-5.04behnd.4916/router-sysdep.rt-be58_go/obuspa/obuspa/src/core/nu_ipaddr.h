/*
 *
 * Copyright (C) 2019-2020, Broadband Forum
 * Copyright (C) 2007-2019  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file nu_ipaddr.h
 *
 * Implements a class that wraps IPv4/v6 address functionality
 *
 */

#ifndef NU_IPADDR_H
#define NU_IPADDR_H 1

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>

#define IPV6_NUIPADDR 1

#if IPV6_NUIPADDR
#define NU_IPADDRSTRLEN INET6_ADDRSTRLEN
typedef struct in6_addr nu_ipaddr_t;
#else /* IPV6_NUIPADDR */
#define NU_IPADDRSTRLEN INET_ADDRSTRLEN
typedef struct in_addr nu_ipaddr_t;
#endif /* !IPV6_NUIPADDR */

#if BYTE_ORDER == BIG_ENDIAN
#define IP4NETADDR_PRINTF_FMT(_addr)					\
	(unsigned int)(((_addr) >> 24) & 0xff),				\
		(unsigned int)(((_addr) >> 16) & 0xff),			\
		(unsigned int)(((_addr) >> 8) & 0xff),			\
		(unsigned int)((_addr) & 0xff)
#else
#define IP4NETADDR_PRINTF_FMT(_addr)					\
	(unsigned int)((_addr) & 0xff),					\
		(unsigned int)(((_addr) >> 8) & 0xff),			\
		(unsigned int)(((_addr) >> 16) & 0xff),			\
		(unsigned int)(((_addr) >> 24) & 0xff)
#endif

/* interface for converting nu_ipaddr_t to standard values */
int nu_ipaddr_get_family(const nu_ipaddr_t *addr, sa_family_t *familyp);
int nu_ipaddr_to_inaddr(const nu_ipaddr_t *addr, struct in_addr *p);
int nu_ipaddr_to_in6addr(const nu_ipaddr_t *addr, struct in6_addr *p);
int nu_ipaddr_to_sockaddr(const nu_ipaddr_t *addr, int port, struct sockaddr_storage *sa, socklen_t *len_p);

int nu_ipaddr_to_str(const nu_ipaddr_t *addr, char *buf, int buflen);
char *nu_ipaddr_str(const nu_ipaddr_t *addr, char *buf, int buflen);
int nu_ipaddr_from_str(const char *str, nu_ipaddr_t *addr);
int nu_ipaddr_from_sockaddr_storage(const struct sockaddr_storage *p, nu_ipaddr_t *addr, uint16_t *port);
int nu_ipaddr_from_inaddr(const struct in_addr *p, nu_ipaddr_t *addr);
int nu_ipaddr_from_in6addr(const struct in6_addr *p, nu_ipaddr_t *addr);

int nu_ipaddr_equal(const nu_ipaddr_t *a1, const nu_ipaddr_t *a2, bool *equalp);
int nu_ipaddr_copy(nu_ipaddr_t *dest, const nu_ipaddr_t *src);
int nu_ipaddr_set_zero(nu_ipaddr_t *addr);
bool nu_ipaddr_is_zero(const nu_ipaddr_t *addrp);

int nu_ipaddr_get_interface_addr_from_dest_addr(nu_ipaddr_t *dest, nu_ipaddr_t *if_addr);
int nu_ipaddr_get_interface_addr_from_sock_fd(int sock_fd, char *buf, int bufsiz);
int nu_ipaddr_get_interface_name_from_src_addr(char *src_addr, char *name, int name_len);
int nu_ipaddr_has_interface_addr_changed(char *dev, char *expected_addr, bool *has_addr);
int nu_ipaddr_get_ip_supported_families(bool *ipv4_supported, bool *ipv6_supported);
bool nu_ipaddr_is_valid_interface(const char *dev);
char *tw_ulib_diags_family_to_protocol_version(int address_family);

int tw_ulib_diags_lookup_host(const char *host, int acs_family_pref, bool prefer_ipv6, nu_ipaddr_t *acs_ipaddr_to_bind_to, nu_ipaddr_t *dst);
int tw_ulib_get_dev_ipaddr(const char *dev, char *addr, size_t asiz, bool prefer_ipv6);

#ifdef CONNECT_ONLY_OVER_WAN_INTERFACE
int tw_ulib_dev_get_live_wan_address(char *buf, size_t bufsiz);
#endif

#endif
