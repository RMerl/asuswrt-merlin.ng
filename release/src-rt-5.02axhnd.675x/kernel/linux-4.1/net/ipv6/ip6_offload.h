/*
 *	IPV6 GSO/GRO offload support
 *	Linux INET6 implementation
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#ifndef __ip6_offload_h
#define __ip6_offload_h

int ipv6_exthdrs_offload_init(void);
int udp_offload_init(void);
int tcpv6_offload_init(void);

#endif
