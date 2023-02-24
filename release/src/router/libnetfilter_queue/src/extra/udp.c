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
#define _GNU_SOURCE
#include <netinet/udp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_udp.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv6.h>
#include <libnetfilter_queue/pktbuff.h>

#include "internal.h"

/**
 * \defgroup udp UDP helper functions
 * @{
 */

/**
 * nfq_udp_get_hdr - get the UDP header.
 * \param pktb: Pointer to userspace network packet buffer
 *
 * \returns validated pointer to the UDP header or NULL if the UDP header was
 * not set or if a minimal length check fails.
 * \note You have to call nfq_ip_set_transport_header() or
 * nfq_ip6_set_transport_header() first to set the UDP header.
 */
EXPORT_SYMBOL
struct udphdr *nfq_udp_get_hdr(struct pkt_buff *pktb)
{
	if (pktb->transport_header == NULL)
		return NULL;

	/* No room for the UDP header. */
	if (pktb_tail(pktb) - pktb->transport_header < sizeof(struct udphdr))
		return NULL;

	return (struct udphdr *)pktb->transport_header;
}

/**
 * nfq_udp_get_payload - get the UDP packet payload.
 * \param udph: Pointer to UDP header
 * \param pktb: Pointer to userspace network packet buffer
 * \returns Pointer to the UDP payload, or NULL if malformed UDP packet.
 */
EXPORT_SYMBOL
void *nfq_udp_get_payload(struct udphdr *udph, struct pkt_buff *pktb)
{
	uint16_t len = ntohs(udph->len);

	/* the UDP packet is too short. */
	if (len < sizeof(struct udphdr))
		return NULL;

	/* malformed UDP packet. */
	if (pktb->transport_header + len > pktb_tail(pktb))
		return NULL;

	return pktb->transport_header + sizeof(struct udphdr);
}

/**
 * nfq_udp_get_payload_len - get the udp packet payload.
 * \param udph: Pointer to UDP header
 * \param pktb: Pointer to userspace network packet buffer
 * \returns Length of UDP payload (user data)
 */
EXPORT_SYMBOL
unsigned int nfq_udp_get_payload_len(struct udphdr *udph, struct pkt_buff *pktb)
{
	return pktb_tail(pktb) - pktb->transport_header - sizeof(struct udphdr);
}

/**
 * \defgroup udp_internals Internal UDP functions
 *
 * Most user-space programs will never need these.
 *
 * @{
 */

/**
 * nfq_udp_compute_checksum_ipv4 - sets up the UDP checksum in a UDP/IPv4 packet
 * \param udph: pointer to the UDP header
 * \param iph: pointer to the IPv4 header
 * \note
 * nfq_udp_mangle_ipv4() invokes this function.
 * As long as developers always use __nfq_udp_mangle_ipv4__ when changing the
 * content of a UDP message, there is no need to call
 * __nfq_udp_compute_checksum_ipv4__.
 */
EXPORT_SYMBOL
void nfq_udp_compute_checksum_ipv4(struct udphdr *udph, struct iphdr *iph)
{
	/* checksum field in header needs to be zero for calculation. */
	udph->check = 0;
	udph->check = nfq_checksum_tcpudp_ipv4(iph, IPPROTO_UDP);
}

/**
 * nfq_udp_compute_checksum_ipv6 - sets up the UDP checksum in a UDP/IPv6 packet
 * \param udph: pointer to the UDP header
 * \param ip6h: pointer to the IPv6 header
 * \note
 * nfq_udp_mangle_ipv6() invokes this function.
 * As long as developers always use __nfq_udp_mangle_ipv6__ when changing the
 * content of a UDP message, there is no need to call
 * __nfq_udp_compute_checksum_ipv6__.
 */
EXPORT_SYMBOL
void nfq_udp_compute_checksum_ipv6(struct udphdr *udph, struct ip6_hdr *ip6h)
{
	/* checksum field in header needs to be zero for calculation. */
	udph->check = 0;
	udph->check = nfq_checksum_tcpudp_ipv6(ip6h, udph, IPPROTO_UDP);
}

/**
 * @}
 */

/**
 * nfq_udp_mangle_ipv4 - Mangle UDP/IPv4 packet buffer
 * \param pktb: Pointer to network packet buffer
 * \param match_offset: Offset from start of UDP data of content that you want
 * to mangle
 * \param match_len: Length of the existing content you want to mangle
 * \param rep_buffer: Pointer to data you want to use to replace current content
 * \param rep_len: Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv4 and UDP lengths and recalculates their
 * checksums for you.
 */
EXPORT_SYMBOL
int nfq_udp_mangle_ipv4(struct pkt_buff *pktb,
			unsigned int match_offset, unsigned int match_len,
			const char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph;
	struct udphdr *udph;

	iph = (struct iphdr *)pktb->network_header;
	udph = (struct udphdr *)(pktb->network_header + iph->ihl*4);

	udph->len = htons(ntohs(udph->len) + rep_len - match_len);

	if (!nfq_ip_mangle(pktb, iph->ihl*4 + sizeof(struct udphdr),
				match_offset, match_len, rep_buffer, rep_len))
		return 0;

	nfq_udp_compute_checksum_ipv4(udph, iph);

	return 1;
}

/**
 * nfq_udp_mangle_ipv6 - Mangle UDP/IPv6 packet buffer
 * \param pktb: Pointer to network packet buffer
 * \param match_offset: Offset from start of UDP data of content that you want
 * to mangle
 * \param match_len: Length of the existing content you want to mangle
 * \param rep_buffer: Pointer to data you want to use to replace current content
 * \param rep_len: Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv6 and UDP lengths and recalculates the UDP
 * checksum for you.
 */
EXPORT_SYMBOL
int nfq_udp_mangle_ipv6(struct pkt_buff *pktb,
			unsigned int match_offset, unsigned int match_len,
			const char *rep_buffer, unsigned int rep_len)
{
	struct ip6_hdr *ip6h;
	struct udphdr *udph;

	ip6h = (struct ip6_hdr *)pktb->network_header;
	udph = (struct udphdr *)(pktb->transport_header);
	if (!udph)
		return 0;

	udph->len = htons(ntohs(udph->len) + rep_len - match_len);

	if (!nfq_ip6_mangle(pktb,
			    pktb->transport_header - pktb->network_header +
			    sizeof(struct udphdr),
			    match_offset, match_len, rep_buffer, rep_len))
		return 0;

	nfq_udp_compute_checksum_ipv6(udph, ip6h);

	return 1;
}

/**
 * nfq_pkt_snprintf_udp_hdr - print udp header into one buffer in a humnan
 * readable way
 * \param buf: pointer to buffer that is used to print the object
 * \param size: size of the buffer (or remaining room in it).
 * \param udph: pointer to a valid udp header.
 * \returns The number of characters notionally written (excluding trailing NUL)
 * \sa __snprintf__(3)
 *
 */
EXPORT_SYMBOL
int nfq_udp_snprintf(char *buf, size_t size, const struct udphdr *udph)
{
	return snprintf(buf, size, "SPT=%u DPT=%u ",
			htons(udph->source), htons(udph->dest));
}

/**
 * @}
 */
