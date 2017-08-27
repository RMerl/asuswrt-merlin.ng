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
#include <stddef.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv6.h>
#include <libnetfilter_queue/pktbuff.h>

#include "internal.h"

/**
 * \defgroup ipv6 IPv6 helper functions
 * @{
 */

/**
 * nfq_ip6_get_hdr - get IPv6 header
 * \param pktb: pointer to user-space network packet buffer
 *
 * This funcion returns NULL if an invalid header is found. On sucess, it
 * returns a valid pointer to the header.
 */
struct ip6_hdr *nfq_ip6_get_hdr(struct pkt_buff *pktb)
{
	struct ip6_hdr *ip6h;
	unsigned int pktlen = pktb->tail - pktb->network_header;

	/* Not enough room for IPv4 header. */
	if (pktlen < sizeof(struct ip6_hdr))
		return NULL;

	ip6h = (struct ip6_hdr *)pktb->network_header;

	/* Not IPv6 packet. */
	if (ip6h->ip6_flow != 0x60)
		return NULL;

	return ip6h;
}
EXPORT_SYMBOL(nfq_ip6_get_hdr);

/**
 * nfq_ip6_set_transport_header - set transport header pointer for IPv6 packet
 * \param pktb: pointer to user-space network packet buffer
 * \param ip6h: pointer to IPv6 header
 * \param target: protocol number to find transport header (ie. IPPROTO_*)
 *
 * This function returns 1 if the protocol has been found and the transport
 * header has been set. Otherwise, it returns 0.
 */
int nfq_ip6_set_transport_header(struct pkt_buff *pktb, struct ip6_hdr *ip6h,
				 uint8_t target)
{
	uint8_t nexthdr = ip6h->ip6_nxt;
	uint8_t *cur = (uint8_t *)ip6h + sizeof(struct ip6_hdr);

	while (nexthdr != target) {
		struct ip6_ext *ip6_ext;
		uint32_t hdrlen;

		/* No more extensions, we're done. */
		if (nexthdr == IPPROTO_NONE) {
			cur = NULL;
			break;
		}
		/* No room for extension, bad packet. */
		if (pktb->tail - cur < sizeof(struct ip6_ext)) {
			cur = NULL;
			break;
		}
		ip6_ext = (struct ip6_ext *)cur;

		if (nexthdr == IPPROTO_FRAGMENT) {
			uint16_t *frag_off;

			/* No room for full fragment header, bad packet. */
			if (pktb->tail - cur < sizeof(struct ip6_frag)) {
				cur = NULL;
				break;
			}

			frag_off = (uint16_t *)cur +
					offsetof(struct ip6_frag, ip6f_offlg);

			/* Fragment offset is only 13 bits long. */
			if (htons(*frag_off & ~0x7)) {
				/* Not the first fragment, it does not contain
				 * any headers.
				 */
				cur = NULL;
				break;
			}
			hdrlen = sizeof(struct ip6_frag);
		} else if (nexthdr == IPPROTO_AH)
			hdrlen = (ip6_ext->ip6e_len + 2) << 2;
		else
			hdrlen = ip6_ext->ip6e_len;

		nexthdr = ip6_ext->ip6e_nxt;
		cur += hdrlen;
	}
	pktb->transport_header = cur;
	return cur ? 1 : 0;
}
EXPORT_SYMBOL(nfq_ip6_set_transport_header);

/**
 * nfq_ip6_snprintf - print IPv6 header into one buffer in iptables LOG format
 * \param buf: pointer to buffer that is used to print the object
 * \param size: size of the buffer (or remaining room in it).
 * \param ip6_hdr: pointer to a valid IPv6 header.
 *
 */
int nfq_ip6_snprintf(char *buf, size_t size, const struct ip6_hdr *ip6h)
{
	int ret;
	char src[INET6_ADDRSTRLEN];
	char dst[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &ip6h->ip6_src, src, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &ip6h->ip6_dst, dst, INET6_ADDRSTRLEN);

	ret = snprintf(buf, size, "SRC=%s DST=%s LEN=%Zu TC=0x%X "
				  "HOPLIMIT=%u FLOWLBL=%u ",
			src, dst,
			ntohs(ip6h->ip6_plen) + sizeof(struct ip6_hdr),
			(ip6h->ip6_flow & 0x0ff00000) >> 20,
			ip6h->ip6_hlim,
			(ip6h->ip6_flow & 0x000fffff));

	return ret;
}
EXPORT_SYMBOL(nfq_ip6_snprintf);

/**
 * @}
 */
