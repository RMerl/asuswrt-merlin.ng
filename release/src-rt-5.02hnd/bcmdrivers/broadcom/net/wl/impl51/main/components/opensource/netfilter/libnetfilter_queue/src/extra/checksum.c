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
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "internal.h"

uint16_t checksum(uint32_t sum, uint16_t *buf, int size)
{
	while (size > 1) {
		sum += *buf++;
		size -= sizeof(uint16_t);
	}
	if (size)
		sum += *(uint8_t *)buf;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >>16);

	return (uint16_t)(~sum);
}

uint16_t checksum_tcpudp_ipv4(struct iphdr *iph)
{
	uint32_t sum = 0;
	uint32_t iph_len = iph->ihl*4;
	uint32_t len = ntohs(iph->tot_len) - iph_len;
	uint8_t *payload = (uint8_t *)iph + iph_len;

	sum += (iph->saddr >> 16) & 0xFFFF;
	sum += (iph->saddr) & 0xFFFF;
	sum += (iph->daddr >> 16) & 0xFFFF;
	sum += (iph->daddr) & 0xFFFF;
	sum += htons(IPPROTO_TCP);
	sum += htons(len);

	return checksum(sum, (uint16_t *)payload, len);
}

uint16_t checksum_tcpudp_ipv6(struct ip6_hdr *ip6h, void *transport_hdr)
{
	uint32_t sum = 0;
	uint32_t hdr_len = (uint32_t *)transport_hdr - (uint32_t *)ip6h;
	uint32_t len = ip6h->ip6_plen - hdr_len;
	uint8_t *payload = (uint8_t *)ip6h + hdr_len;
	int i;

	for (i=0; i<8; i++) {
		sum += (ip6h->ip6_src.s6_addr16[i] >> 16) & 0xFFFF;
		sum += (ip6h->ip6_src.s6_addr16[i]) & 0xFFFF;
	}
	for (i=0; i<8; i++) {
		sum += (ip6h->ip6_dst.s6_addr16[i] >> 16) & 0xFFFF;
		sum += (ip6h->ip6_dst.s6_addr16[i]) & 0xFFFF;
	}
	sum += htons(IPPROTO_TCP);
	sum += htons(ip6h->ip6_plen);

	return checksum(sum, (uint16_t *)payload, len);
}

/**
 * @}
 */
