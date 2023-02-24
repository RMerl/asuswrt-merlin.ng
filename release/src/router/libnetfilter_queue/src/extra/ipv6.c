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
 * \param pktb: Pointer to user-space network packet buffer
 *
 * \returns pointer to IPv6 header if a valid header found, else NULL.
 */
EXPORT_SYMBOL
struct ip6_hdr *nfq_ip6_get_hdr(struct pkt_buff *pktb)
{
	struct ip6_hdr *ip6h;
	unsigned int pktlen = pktb_tail(pktb) - pktb->network_header;

	/* Not enough room for IPv6 header. */
	if (pktlen < sizeof(struct ip6_hdr))
		return NULL;

	ip6h = (struct ip6_hdr *)pktb->network_header;

	/* Not IPv6 packet. */
	if ((*(uint8_t *)ip6h & 0xf0) != 0x60)
		return NULL;

	return ip6h;
}

/**
 * nfq_ip6_set_transport_header - set transport header pointer for IPv6 packet
 * \param pktb: Pointer to user-space network packet buffer
 * \param ip6h: Pointer to IPv6 header
 * \param target: Protocol number to find transport header (ie. IPPROTO_*)
 *
 * \returns 1 if the protocol has been found and the transport
 * header has been set, else 0.
 */
EXPORT_SYMBOL
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
		if (pktb_tail(pktb) - cur < sizeof(struct ip6_ext)) {
			cur = NULL;
			break;
		}
		ip6_ext = (struct ip6_ext *)cur;

		if (nexthdr == IPPROTO_FRAGMENT) {
			uint16_t *frag_off;

			/* No room for full fragment header, bad packet. */
			if (pktb_tail(pktb) - cur < sizeof(struct ip6_frag)) {
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

/**
 * nfq_ip6_mangle - mangle IPv6 packet buffer
 * \param pktb: Pointer to user-space network packet buffer
 * \param dataoff: Offset to layer 4 header
 * \param match_offset: Offset to content that you want to mangle
 * \param match_len: Length of the existing content you want to mangle
 * \param rep_buffer: Pointer to data you want to use to replace current content
 * \param rep_len: Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv6 length (if necessary)
 */
EXPORT_SYMBOL
int nfq_ip6_mangle(struct pkt_buff *pktb, unsigned int dataoff,
		   unsigned int match_offset, unsigned int match_len,
		   const char *rep_buffer, unsigned int rep_len)
{
	struct ip6_hdr *ip6h = (struct ip6_hdr *)pktb->network_header;

	if (!pktb_mangle(pktb, dataoff, match_offset, match_len, rep_buffer,
			 rep_len))
		return 0;

	/* Fix IPv6 hdr length information */
	ip6h->ip6_plen =
		htons(pktb_tail(pktb) - pktb->network_header - sizeof *ip6h);

	return 1;
}

/**
 * nfq_ip6_snprintf - print IPv6 header into one buffer in iptables LOG format
 * \param buf: Pointer to buffer that is used to print the object
 * \param size: Size of the buffer (or remaining room in it).
 * \param ip6h: Pointer to a valid IPv6 header.
 * \returns same as snprintf
 * \sa **snprintf**(3)
 *
 */
EXPORT_SYMBOL
int nfq_ip6_snprintf(char *buf, size_t size, const struct ip6_hdr *ip6h)
{
	int ret;
	char src[INET6_ADDRSTRLEN];
	char dst[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &ip6h->ip6_src, src, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &ip6h->ip6_dst, dst, INET6_ADDRSTRLEN);

	ret = snprintf(buf, size, "SRC=%s DST=%s LEN=%zu TC=0x%X "
				  "HOPLIMIT=%u FLOWLBL=%u ",
			src, dst,
			ntohs(ip6h->ip6_plen) + sizeof(struct ip6_hdr),
			(ip6h->ip6_flow & 0x0ff00000) >> 20,
			ip6h->ip6_hlim,
			(ip6h->ip6_flow & 0x000fffff));

	return ret;
}

/**
 * @}
 */
