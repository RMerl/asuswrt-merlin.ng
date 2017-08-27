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
#include <netinet/udp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_udp.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <libnetfilter_queue/pktbuff.h>

#include "internal.h"

/**
 * \defgroup udp UDP helper functions
 * @{
 */

/**
 * nfq_udp_get_hdr - get the UDP header.
 * \param head: pointer to the beginning of the packet
 * \param tail: pointer to the tail of the packet
 *
 * This function returns NULL if invalid UDP header is found. On success,
 * it returns the UDP header.
 */
struct udphdr *nfq_udp_get_hdr(struct pkt_buff *pktb)
{
	if (pktb->transport_header == NULL)
		return NULL;

	/* No room for the UDP header. */
	if (pktb->tail - pktb->transport_header < sizeof(struct udphdr))
		return NULL;

	return (struct udphdr *)pktb->transport_header;
}
EXPORT_SYMBOL(nfq_udp_get_hdr);

/**
 * nfq_udp_get_payload - get the UDP packet payload.
 * \param udph: the pointer to the UDP header.
 * \param tail: pointer to the tail of the packet
 */
void *nfq_udp_get_payload(struct udphdr *udph, struct pkt_buff *pktb)
{
	unsigned int doff = udph->len;

	/* malformed UDP data offset. */
	if (pktb->transport_header + doff > pktb->tail)
		return NULL;

	return pktb->transport_header + doff;
}
EXPORT_SYMBOL(nfq_udp_get_payload);

/**
 * nfq_udp_get_payload_len - get the udp packet payload.
 * \param udp: the pointer to the udp header.
 */
unsigned int nfq_udp_get_payload_len(struct udphdr *udph, struct pkt_buff *pktb)
{
	return pktb->tail - pktb->transport_header;
}
EXPORT_SYMBOL(nfq_udp_get_payload_len);

/**
 * nfq_udp_set_checksum_ipv4 - computes a IPv4/TCP packet's segment
 * \param iphdrp: pointer to the ip header
 * \param ippayload: payload of the ip packet
 *
 * \returns the checksum of the udp segment.
 *
 * \see nfq_pkt_compute_ip_checksum
 * \see nfq_pkt_compute_udp_checksum
 */
void
nfq_udp_compute_checksum_ipv4(struct udphdr *udph, struct iphdr *iph)
{
	/* checksum field in header needs to be zero for calculation. */
	udph->check = 0;
	udph->check = checksum_tcpudp_ipv4(iph);
}
EXPORT_SYMBOL(nfq_udp_compute_checksum_ipv4);

/**
 * nfq_udp_set_checksum_ipv6 - computes a IPv6/TCP packet's segment
 * \param iphdrp: pointer to the ip header
 * \param ippayload: payload of the ip packet
 *
 * \returns the checksum of the udp segment.
 *
 * \see nfq_pkt_compute_ip_checksum
 * \see nfq_pkt_compute_udp_checksum
 */
void
nfq_udp_compute_checksum_ipv6(struct udphdr *udph, struct ip6_hdr *ip6h)
{
	/* checksum field in header needs to be zero for calculation. */
	udph->check = 0;
	udph->check = checksum_tcpudp_ipv6(ip6h, udph);
}
EXPORT_SYMBOL(nfq_udp_compute_checksum_ipv6);

/**
 * nfq_tcp_mangle_ipv4 - mangle TCP/IPv4 packet buffer
 * \param pktb: pointer to network packet buffer
 * \param match_offset: offset to content that you want to mangle
 * \param match_len: length of the existing content you want to mangle
 * \param rep_buffer: pointer to data you want to use to replace current content 
 * \param rep_len: length of data you want to use to replace current content
 *
 * \note This function recalculates the IPv4 and TCP checksums for you.
 */
int
nfq_udp_mangle_ipv4(struct pkt_buff *pkt,
		    unsigned int match_offset, unsigned int match_len,
		    const char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph;
	struct udphdr *udph;

	iph = (struct iphdr *)pkt->network_header;
	udph = (struct udphdr *)(pkt->network_header + iph->ihl*4);

	if (!nfq_ip_mangle(pkt, iph->ihl*4 + sizeof(struct udphdr),
				match_offset, match_len, rep_buffer, rep_len))
		return 0;

	nfq_udp_compute_checksum_ipv4(udph, iph);

	return 1;
}
EXPORT_SYMBOL(nfq_udp_mangle_ipv4);

/**
 * nfq_pkt_snprintf_udp_hdr - print udp header into one buffer in a humnan
 * readable way
 * \param buf: pointer to buffer that is used to print the object
 * \param size: size of the buffer (or remaining room in it).
 * \param udp: pointer to a valid udp header.
 *
 */
int nfq_udp_snprintf(char *buf, size_t size, const struct udphdr *udph)
{
	return snprintf(buf, size, "SPT=%u DPT=%u ",
			htons(udph->source), htons(udph->dest));
}
EXPORT_SYMBOL(nfq_udp_snprintf);

/**
 * @}
 */
