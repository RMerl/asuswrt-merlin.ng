/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "parser.h"

void arp_dump(int level, struct frame *frm)
{
	int i;
	char buf[20];
	struct sockaddr_in sai;
	struct ether_arp *arp = (struct ether_arp *) frm->ptr;

	printf("Src ");
	for (i = 0; i < 5; i++)
		printf("%02x:", arp->arp_sha[i]);
	printf("%02x", arp->arp_sha[5]);
	sai.sin_family = AF_INET;
	memcpy(&sai.sin_addr, &arp->arp_spa, sizeof(sai.sin_addr));
	getnameinfo((struct sockaddr *) &sai, sizeof(sai), buf, sizeof(buf),
		    NULL, 0, NI_NUMERICHOST);
	printf("(%s) ", buf);
	printf("Tgt ");
	for (i = 0; i < 5; i++)
		printf("%02x:", arp->arp_tha[i]);
	printf("%02x", arp->arp_tha[5]);
	memcpy(&sai.sin_addr, &arp->arp_tpa, sizeof(sai.sin_addr));
	getnameinfo((struct sockaddr *) &sai, sizeof(sai), buf, sizeof(buf),
		    NULL, 0, NI_NUMERICHOST);
	printf("(%s)\n", buf);
	frm->ptr += sizeof(struct ether_arp);
	frm->len -= sizeof(struct ether_arp);
	raw_dump(level, frm);		// not needed.
}

void ip_dump(int level, struct frame *frm)
{
	char src[50], dst[50];
	struct ip *ip = (struct ip *) (frm->ptr);
	uint8_t proto;
	int len;

	if (ip->ip_v == 4) {
		struct sockaddr_in sai;
		proto = ip->ip_p;
		len = ip->ip_hl << 2;
		memset(&sai, 0, sizeof(sai));
		sai.sin_family = AF_INET;
		memcpy(&sai.sin_addr, &ip->ip_src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *) &sai, sizeof(sai),
			    src, sizeof(src), NULL, 0, NI_NUMERICHOST);
		memcpy(&sai.sin_addr, &ip->ip_dst, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *) &sai, sizeof(sai),
			    dst, sizeof(dst), NULL, 0, NI_NUMERICHOST);
	} else if (ip->ip_v == 6) {
		struct sockaddr_in6 sai6;
		struct ip6_hdr *ip6 = (struct ip6_hdr *) ip;
		proto = ip6->ip6_nxt;
		len = sizeof(struct ip6_hdr);
		memset(&sai6, 0, sizeof(sai6));
		sai6.sin6_family = AF_INET6;
		memcpy(&sai6.sin6_addr, &ip6->ip6_src, sizeof(struct in6_addr));
		getnameinfo((struct sockaddr *) &sai6, sizeof(sai6),
			    src, sizeof(src), NULL, 0, NI_NUMERICHOST);
		memcpy(&sai6.sin6_addr, &ip6->ip6_dst, sizeof(struct in6_addr));
		getnameinfo((struct sockaddr *) &sai6, sizeof(sai6),
			    dst, sizeof(dst), NULL, 0, NI_NUMERICHOST);
	} else {
		raw_dump(level, frm);
		return;
	}

	printf("src %s ", src);
	printf("dst %s\n", dst);

	frm->ptr += len;
	frm->len -= len;
	p_indent(++level, frm);

	switch (proto) {
	case IPPROTO_TCP:
		printf("TCP:\n");
		break;

	case IPPROTO_UDP:
		printf("UDP:\n");
		break;

	case IPPROTO_ICMP:
		printf("ICMP:\n");
		break;

	case IPPROTO_ICMPV6:
		printf("ICMPv6:\n");
		break;

	default:
		printf("Unknown Protocol: 0x%02x\n", ip->ip_p);
		break;
	}

	raw_dump(level, frm);
}
