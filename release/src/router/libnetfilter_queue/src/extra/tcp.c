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
#include <string.h> /* for memcpy */
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#define _GNU_SOURCE
#include <netinet/tcp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_tcp.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv6.h>
#include <libnetfilter_queue/pktbuff.h>

#include "internal.h"

/**
 * \defgroup tcp TCP helper functions
 * @{
 */

/**
 * nfq_tcp_get_hdr - get the TCP header
 * \param pktb: pointer to user-space network packet buffer
 * \returns validated pointer to the TCP header or NULL if the TCP header was
 * not set or if a minimal length check fails.
 * \note You have to call nfq_ip_set_transport_header() or
 * nfq_ip6_set_transport_header() first to set the TCP header.
 */
EXPORT_SYMBOL
struct tcphdr *nfq_tcp_get_hdr(struct pkt_buff *pktb)
{
	if (pktb->transport_header == NULL)
		return NULL;

	/* No room for the TCP header. */
	if (pktb_tail(pktb) - pktb->transport_header < sizeof(struct tcphdr))
		return NULL;

	return (struct tcphdr *)pktb->transport_header;
}

/**
 * nfq_tcp_get_payload - get the TCP packet payload
 * \param tcph: pointer to the TCP header
 * \param pktb: pointer to user-space network packet buffer
 * \returns Pointer to the TCP payload, or NULL if malformed TCP packet.
 */
EXPORT_SYMBOL
void *nfq_tcp_get_payload(struct tcphdr *tcph, struct pkt_buff *pktb)
{
	unsigned int len = tcph->doff * 4;

	/* TCP packet is too short */
	if (len < sizeof(struct tcphdr))
		return NULL;

	/* malformed TCP data offset. */
	if (pktb->transport_header + len > pktb_tail(pktb))
		return NULL;

	return pktb->transport_header + len;
}

/**
 * nfq_tcp_get_payload_len - get the tcp packet payload
 * \param tcph: pointer to the TCP header
 * \param pktb: pointer to user-space network packet buffer
 * \returns Length of TCP payload (user data)
 */
EXPORT_SYMBOL
unsigned int nfq_tcp_get_payload_len(struct tcphdr *tcph, struct pkt_buff *pktb)
{
	return pktb_tail(pktb) - pktb->transport_header - (tcph->doff * 4);
}

/**
 * \defgroup tcp_internals Internal TCP functions
 *
 * Most user-space programs will never need these.
 *
 * @{
 */

/**
 * nfq_tcp_compute_checksum_ipv4 - computes IPv4/TCP packet checksum
 * \param tcph: pointer to the TCP header
 * \param iph: pointer to the IPv4 header
 * \note
 * nfq_tcp_mangle_ipv4() invokes this function.
 * As long as developers always use __nfq_tcp_mangle_ipv4__ when changing the
 * content of a TCP message, there is no need to call
 * __nfq_tcp_compute_checksum_ipv4__.
 */
EXPORT_SYMBOL
void nfq_tcp_compute_checksum_ipv4(struct tcphdr *tcph, struct iphdr *iph)
{
	/* checksum field in header needs to be zero for calculation. */
	tcph->check = 0;
	tcph->check = nfq_checksum_tcpudp_ipv4(iph, IPPROTO_TCP);
}

/**
 * nfq_tcp_compute_checksum_ipv6 - computes IPv6/TCP packet checksum
 * \param tcph: pointer to the TCP header
 * \param ip6h: pointer to the IPv6 header
 * \note
 * nfq_tcp_mangle_ipv6() invokes this function.
 * As long as developers always use __nfq_tcp_mangle_ipv6__ when changing the
 * content of a TCP message, there is no need to call
 * __nfq_tcp_compute_checksum_ipv6__.
 */
EXPORT_SYMBOL
void nfq_tcp_compute_checksum_ipv6(struct tcphdr *tcph, struct ip6_hdr *ip6h)
{
	/* checksum field in header needs to be zero for calculation. */
	tcph->check = 0;
	tcph->check = nfq_checksum_tcpudp_ipv6(ip6h, tcph, IPPROTO_TCP);
}

/**
 * @}
 */

/*
 *	The union cast uses a gcc extension to avoid aliasing problems
 *  (union is compatible to any of its members)
 *  This means this part of the code is -fstrict-aliasing safe now.
 */
union tcp_word_hdr {
	struct tcphdr hdr;
	uint32_t  words[5];
};

#define tcp_flag_word(tp) ( ((union tcp_word_hdr *)(tp))->words[3])

/**
 * nfq_pkt_snprintf_tcp_hdr - print tcp header into one buffer in a humnan
 * readable way
 * \param buf: pointer to buffer that is used to print the object
 * \param size: size of the buffer (or remaining room in it).
 * \param tcph: pointer to a valid tcp header.
 * \returns Same as \b snprintf
 * \sa __snprintf__(3)
 *
 */
EXPORT_SYMBOL
int nfq_tcp_snprintf(char *buf, size_t size, const struct tcphdr *tcph)
{
	int ret, len = 0;

#define TCP_RESERVED_BITS htonl(0x0F000000)

	ret = snprintf(buf, size, "SPT=%u DPT=%u SEQ=%u ACK=%u "
				   "WINDOW=%u RES=0x%02x ",
			ntohs(tcph->source), ntohs(tcph->dest),
			ntohl(tcph->seq), ntohl(tcph->ack_seq),
			ntohs(tcph->window),
			(uint8_t)
			(ntohl(tcp_flag_word(tcph) & TCP_RESERVED_BITS) >> 22));
	len += ret;

	if (tcph->urg) {
		ret = snprintf(buf+len, size-len, "URG ");
		len += ret;
	}
	if (tcph->ack) {
		ret = snprintf(buf+len, size-len, "ACK ");
		len += ret;
	}
	if (tcph->psh) {
		ret = snprintf(buf+len, size-len, "PSH ");
		len += ret;
	}
	if (tcph->rst) {
		ret = snprintf(buf+len, size-len, "RST ");
		len += ret;
	}
	if (tcph->syn) {
		ret = snprintf(buf+len, size-len, "SYN ");
		len += ret;
	}
	if (tcph->fin) {
		ret = snprintf(buf+len, size-len, "FIN ");
		len += ret;
	}
	/* XXX: Not TCP options implemented yet, sorry. */

	return ret;
}

/**
 * nfq_tcp_mangle_ipv4 - mangle TCP/IPv4 packet buffer
 * \param pktb: pointer to network packet buffer
 * \param match_offset: offset to content that you want to mangle
 * \param match_len: length of the existing content you want to mangle
 * \param rep_buffer: pointer to data you want to use to replace current content
 * \param rep_len: length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv4 length and recalculates the IPv4 & TCP
 * checksums for you.
 * \warning After changing the length of a TCP message, the application will
 * need to mangle sequence numbers in both directions until another change
 * puts them in sync again
 */
EXPORT_SYMBOL
int nfq_tcp_mangle_ipv4(struct pkt_buff *pktb,
			unsigned int match_offset, unsigned int match_len,
			const char *rep_buffer, unsigned int rep_len)
{
	struct iphdr *iph;
	struct tcphdr *tcph;

	iph = (struct iphdr *)pktb->network_header;
	tcph = (struct tcphdr *)(pktb->network_header + iph->ihl*4);

	if (!nfq_ip_mangle(pktb, iph->ihl*4 + tcph->doff*4,
				match_offset, match_len, rep_buffer, rep_len))
		return 0;

	nfq_tcp_compute_checksum_ipv4(tcph, iph);

	return 1;
}

/**
 * nfq_tcp_mangle_ipv6 - Mangle TCP/IPv6 packet buffer
 * \param pktb: Pointer to network packet buffer
 * \param match_offset: Offset from start of TCP data of content that you want
 * to mangle
 * \param match_len: Length of the existing content you want to mangle
 * \param rep_buffer: Pointer to data you want to use to replace current content
 * \param rep_len: Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. See pktb_mangle() for failure case
 * \note This function updates the IPv6 length and recalculates the TCP
 * checksum for you.
 * \warning After changing the length of a TCP message, the application will
 * need to mangle sequence numbers in both directions until another change
 * puts them in sync again
 */
EXPORT_SYMBOL
int nfq_tcp_mangle_ipv6(struct pkt_buff *pktb,
			unsigned int match_offset, unsigned int match_len,
			const char *rep_buffer, unsigned int rep_len)
{
	struct ip6_hdr *ip6h;
	struct tcphdr *tcph;

	ip6h = (struct ip6_hdr *)pktb->network_header;
	tcph = (struct tcphdr *)(pktb->transport_header);
	if (!tcph)
		return 0;

	if (!nfq_ip6_mangle(pktb,
			   pktb->transport_header - pktb->network_header +
			   tcph->doff * 4,
			   match_offset, match_len, rep_buffer, rep_len))
		return 0;

	nfq_tcp_compute_checksum_ipv6(tcph, ip6h);

	return 1;
}

/**
 * @}
 */
