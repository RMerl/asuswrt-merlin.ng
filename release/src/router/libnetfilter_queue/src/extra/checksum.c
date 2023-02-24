/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include <stdio.h>
#include <stdbool.h>
#include <endian.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "internal.h"

uint16_t nfq_checksum(uint32_t sum, uint16_t *buf, int size)
{
	while (size > 1) {
		sum += *buf++;
		size -= sizeof(uint16_t);
	}
	if (size) {
#if __BYTE_ORDER == __BIG_ENDIAN
		sum += (uint16_t)*(uint8_t *)buf << 8;
#else
		sum += (uint16_t)*(uint8_t *)buf;
#endif
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >>16);

	return (uint16_t)(~sum);
}

uint16_t nfq_checksum_tcpudp_ipv4(struct iphdr *iph, uint16_t protonum)
{
	uint32_t sum = 0;
	uint32_t iph_len = iph->ihl*4;
	uint32_t len = ntohs(iph->tot_len) - iph_len;
	uint8_t *payload = (uint8_t *)iph + iph_len;

	sum += (iph->saddr >> 16) & 0xFFFF;
	sum += (iph->saddr) & 0xFFFF;
	sum += (iph->daddr >> 16) & 0xFFFF;
	sum += (iph->daddr) & 0xFFFF;
	sum += htons(protonum);
	sum += htons(len);

	return nfq_checksum(sum, (uint16_t *)payload, len);
}

uint16_t nfq_checksum_tcpudp_ipv6(struct ip6_hdr *ip6h, void *transport_hdr,
				  uint16_t protonum)
{
	uint32_t sum = 0;
	uint32_t hdr_len = (uint8_t *)transport_hdr - (uint8_t *)ip6h;
	/* Allow for extra headers before the UDP header */
	/* TODO: Deal with routing headers */
	uint32_t len = ntohs(ip6h->ip6_plen) - (hdr_len - sizeof *ip6h);
	uint8_t *payload = (uint8_t *)ip6h + hdr_len;
	int i;

	for (i=0; i<8; i++) {
		sum += (ip6h->ip6_src.s6_addr16[i]);
	}
	for (i=0; i<8; i++) {
		sum += (ip6h->ip6_dst.s6_addr16[i]);
	}
	sum += htons(protonum);
	sum += htons(len);

	return nfq_checksum(sum, (uint16_t *)payload, len);
}

/**
 * @}
 */
