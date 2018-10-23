/*
 * (C) 2006-2008 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include "cidr.h"

/* returns the netmask in host byte order */
uint32_t ipv4_cidr2mask_host(uint8_t cidr)
{
	return 0xFFFFFFFF << (32 - cidr);
}

/* returns the netmask in network byte order */
uint32_t ipv4_cidr2mask_net(uint8_t cidr)
{
	return htonl(ipv4_cidr2mask_host(cidr));
}

void ipv6_cidr2mask_host(uint8_t cidr, uint32_t *res)
{
	int i, j;

	memset(res, 0, sizeof(uint32_t)*4);
	for (i = 0;  i < 4 && cidr > 32; i++) {
		res[i] = 0xFFFFFFFF;
		cidr -= 32;
	}
	res[i] = 0xFFFFFFFF << (32 - cidr);
	for (j = i+1; j < 4; j++) {
		res[j] = 0;
	}
}

void ipv6_cidr2mask_net(uint8_t cidr, uint32_t *res)
{
	int i;

	ipv6_cidr2mask_host(cidr, res);
	for (i=0; i<4; i++)
		res[i] = htonl(res[i]);
}

/* I need this function because I initially defined an IPv6 address as
 * uint32 u[4]. Using char u[16] instead would allow to remove this. */
void ipv6_addr2addr_host(uint32_t *addr, uint32_t *res)
{
	int i;

	memset(res, 0, sizeof(uint32_t)*4);
	for (i = 0;  i < 4; i++) {
		res[i] = ntohl(addr[i]);
	}
}
