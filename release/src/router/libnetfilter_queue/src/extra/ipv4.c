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
 * nfq_ip_get_hdr - get the IPv4 header
 * \param pktb: Pointer to user-space network packet buffer
 * \returns validated pointer to the IPv4 header or NULL if IP is malformed or
 * not version 4
 *
 * Many programs will not need to call this function. A possible use is to
 * determine the layer 4 protocol. The validation is that the buffer is big
 * enough for the declared lengths in the header, i.e. an extra check for packet
 * truncation.
 */
EXPORT_SYMBOL
struct iphdr *nfq_ip_get_hdr(struct pkt_buff *pktb)
{
	struct iphdr *iph;
	unsigned int pktlen = pktb_tail(pktb) - pktb->network_header;

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

/**
 * nfq_ip_set_transport_header - set the \b transport_header field in \b pktb
 * \param pktb: Pointer to user-space network packet buffer
 * \param iph: Pointer to the IPv4 header
 * \returns 0 on success or -1 if a minimal validation check fails
 * \note
 * Most programs should call __nfq_ip_set_transport_header__ as soon as
 * possible, since most layer 4 helper functions assume the
 * \b transport_header field is valid.
 */
EXPORT_SYMBOL
int nfq_ip_set_transport_header(struct pkt_buff *pktb, struct iphdr *iph)
{
	int doff = iph->ihl * 4;

	/* Wrong offset to IPv4 payload. */
	if ((int)pktb->len - doff <= 0)
		return -1;

	pktb->transport_header = pktb->network_header + doff;
	return 0;
}

/**
 * \defgroup ip_internals Internal IP functions
 *
 * Most user-space programs will never need these.
 *
 * @{
 */

/**
 * nfq_ip_set_checksum - set IPv4 checksum
 * \param iph: Pointer to the IPv4 header
 * \note
 * nfq_ip_mangle() invokes this function.
 * As long as developers always use the appropriate mangler for the layer being
 * mangled, there is no need to call __nfq_ip_set_checksum__.
 */
EXPORT_SYMBOL
void nfq_ip_set_checksum(struct iphdr *iph)
{
	uint32_t iph_len = iph->ihl * 4;

	iph->check = 0;
	iph->check = nfq_checksum(0, (uint16_t *)iph, iph_len);
}

/**
 * @}
 */

/**
 * nfq_ip_mangle - mangle IPv4 packet buffer
 * \param pktb: Pointer to user-space network packet buffer
 * \param dataoff: Offset to layer 4 header, or zero to mangle IP header
 * \param match_offset: Offset to content that you want to mangle
 * \param match_len: Length of the existing content you want to mangle
 * \param rep_buffer: Pointer to data you want to use to replace current content
 * \param rep_len: Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv4 length if necessary and recalculates the
 * IPv4 checksum.
 */
EXPORT_SYMBOL
int nfq_ip_mangle(struct pkt_buff *pktb, unsigned int dataoff,
		  unsigned int match_offset, unsigned int match_len,
		  const char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph = (struct iphdr *) pktb->network_header;

	if (!pktb_mangle(pktb, dataoff, match_offset, match_len, rep_buffer,
			 rep_len))
		return 0;

	/* fix IP hdr checksum information */
	iph->tot_len = htons(pktb_tail(pktb) - pktb->network_header);
	nfq_ip_set_checksum(iph);

	return 1;
}

/**
 * nfq_pkt_snprintf_ip - print IPv4 header into buffer in iptables LOG format
 * \param buf: Pointer to buffer that will be used to print the header
 * \param size: Size of the buffer (or remaining room in it)
 * \param iph: Pointer to a valid IPv4 header
 * \returns same as snprintf
 * \sa **snprintf**(3)
 */
EXPORT_SYMBOL
int nfq_ip_snprintf(char *buf, size_t size, const struct iphdr *iph)
{
	int ret;
	struct in_addr src = { iph->saddr };
	struct in_addr dst = { iph->daddr };

	char src_str[INET_ADDRSTRLEN];
	char dst_str[INET_ADDRSTRLEN];

	ret = snprintf(buf, size, "SRC=%s DST=%s LEN=%u TOS=0x%X "
				  "PREC=0x%X TTL=%u ID=%u PROTO=%u ",
			inet_ntop(AF_INET, &src, src_str, INET_ADDRSTRLEN),
			inet_ntop(AF_INET, &dst, dst_str, INET_ADDRSTRLEN),
			ntohs(iph->tot_len), IPTOS_TOS(iph->tos),
			IPTOS_PREC(iph->tos), iph->ttl, ntohs(iph->id),
			iph->protocol);

	return ret;
}

/**
 * @}
 */
