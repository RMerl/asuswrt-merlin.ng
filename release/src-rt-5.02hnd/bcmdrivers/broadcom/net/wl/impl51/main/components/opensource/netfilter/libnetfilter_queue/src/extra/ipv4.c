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

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <libnetfilter_queue/pktbuff.h>

#include "internal.h"

/**
 * \defgroup ipv4 IPv4 helper functions
 * @{
 */

/**
 * nfq_ip_get_hdr - get IPv4 header
 * \param pktb: pointer to network packet buffer
 *
 * This funcion returns NULL if the IPv4 is malformed or the protocol version
 * is not 4. On success, it returns a valid pointer to the IPv4 header.
 */
struct iphdr *nfq_ip_get_hdr(struct pkt_buff *pktb)
{
	struct iphdr *iph;
	unsigned int pktlen = pktb->tail - pktb->network_header;

	/* Not enough room for IPv4 header. */
	if (pktlen < sizeof(struct iphdr))
		return NULL;

	iph = (struct iphdr *)pktb->network_header;

	/* Not IPv4 packet. */
	if (iph->version != 4)
		return NULL;

	/* Malformed IPv4 total length field. */
	if (ntohs(iph->tot_len) > pktlen)
		return NULL;

	return iph;
}
EXPORT_SYMBOL(nfq_ip_get_hdr);

/**
 * nfq_ip_set_transport_header - set transport header
 * \param pktb: pointer to network packet buffer
 * \param iph: pointer to the IPv4 header
 */
int nfq_ip_set_transport_header(struct pkt_buff *pktb, struct iphdr *iph)
{
	int doff = iph->ihl * 4;

	/* Wrong offset to IPv4 payload. */
	if ((int)pktb->len - doff <= 0)
		return -1;

	pktb->transport_header = pktb->network_header + doff;
	return 0;
}
EXPORT_SYMBOL(nfq_ip_set_transport_header);

/**
 * nfq_ip_set_checksum - set IPv4 checksum
 * \param iph: pointer to the IPv4 header
 *
 * \note Call to this function if you modified the IPv4 header to update the
 * checksum.
 */
void nfq_ip_set_checksum(struct iphdr *iph)
{
	uint32_t iph_len = iph->ihl * 4;

	iph->check = 0;
	iph->check = checksum(0, (uint16_t *)iph, iph_len);
}
EXPORT_SYMBOL(nfq_ip_set_checksum);

/**
 * nfq_ip_mangle - mangle IPv4 packet buffer
 * \param pktb: pointer to network packet buffer
 * \param dataoff: offset to layer 4 header
 * \param match_offset: offset to content that you want to mangle
 * \param match_len: length of the existing content you want to mangle
 * \param rep_buffer: pointer to data you want to use to replace current content
 * \param rep_len: length of data you want to use to replace current content
 *
 * \note This function recalculates the IPv4 checksum (if needed).
 */
int nfq_ip_mangle(struct pkt_buff *pkt, unsigned int dataoff,
		  unsigned int match_offset, unsigned int match_len,
		  const char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph = (struct iphdr *) pkt->network_header;

	if (!pktb_mangle(pkt, dataoff, match_offset, match_len,
						rep_buffer, rep_len))
		return 0;

	/* fix IP hdr checksum information */
	iph->tot_len = htons(pkt->len);
	nfq_ip_set_checksum(iph);

	return 1;
}
EXPORT_SYMBOL(nfq_ip_mangle);

/**
 * nfq_pkt_snprintf_ip - print IPv4 header into buffer in iptables LOG format
 * \param buf: pointer to buffer that will be used to print the header
 * \param size: size of the buffer (or remaining room in it)
 * \param ip: pointer to a valid IPv4 header
 *
 * This function returns the number of bytes that would have been written in
 * case that there is enough room in the buffer. Read snprintf manpage for more
 * information to know more about this strange behaviour.
 */
int nfq_ip_snprintf(char *buf, size_t size, const struct iphdr *iph)
{
	int ret;
	struct in_addr src = { iph->saddr };
	struct in_addr dst = { iph->daddr };

	ret = snprintf(buf, size, "SRC=%s DST=%s LEN=%u TOS=0x%X "
				  "PREC=0x%X TTL=%u ID=%u PROTO=%u ",
			inet_ntoa(src), inet_ntoa(dst),
			ntohs(iph->tot_len), IPTOS_TOS(iph->tos),
			IPTOS_PREC(iph->tos), iph->ttl, ntohs(iph->id),
			iph->protocol);

	return ret;
}
EXPORT_SYMBOL(nfq_ip_snprintf);

/**
 * @}
 */
