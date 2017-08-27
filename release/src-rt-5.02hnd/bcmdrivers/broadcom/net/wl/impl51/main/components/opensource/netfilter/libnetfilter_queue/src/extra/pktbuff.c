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

#include <stdlib.h>
#include <string.h> /* for memcpy */
#include <stdbool.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "internal.h"

/**
 * \defgroup pktbuff User-space network packet buffer
 *
 * This library provides the user-space network packet buffer. This abstraction
 * is strongly inspired by Linux kernel network buffer, the so-called sk_buff.
 *
 * @{
 */

/**
 * pktb_alloc - allocate a new packet buffer
 * \param family Indicate what family, eg. AF_BRIDGE, AF_INET, AF_INET6, ...
 * \param data Pointer to packet data
 * \param len Packet length
 * \param extra Extra memory in the tail to be allocated (for mangling)
 *
 * This function returns a packet buffer that contains the packet data and
 * some extra memory room in the tail (in case of requested).
 *
 * \return a pointer to a new queue handle or NULL on failure.
 */
struct pkt_buff *
pktb_alloc(int family, void *data, size_t len, size_t extra)
{
	struct pkt_buff *pktb;
	void *pkt_data;

	pktb = calloc(1, sizeof(struct pkt_buff) + len + extra);
	if (pktb == NULL)
		return NULL;

	/* Better make sure alignment is correct. */
	pkt_data = (uint8_t *)pktb + sizeof(struct pkt_buff);
	memcpy(pkt_data, data, len);

	pktb->len = len;
	pktb->data_len = len + extra;

	pktb->head = pkt_data;
	pktb->data = pkt_data;
	pktb->tail = pktb->head + len;

	switch(family) {
	case AF_INET:
		pktb->network_header = pktb->data;
		break;
	case AF_BRIDGE: {
		struct ethhdr *ethhdr = (struct ethhdr *)pktb->data;

		pktb->mac_header = pktb->data;

		switch(ethhdr->h_proto) {
		case ETH_P_IP:
			pktb->network_header = pktb->data + ETH_HLEN;
			break;
		default:
			/* This protocol is unsupported. */
			free(pktb);
			return NULL;
		}
		break;
	}
	}
	return pktb;
}

/**
 * pktb_data - return pointer to the beginning of the packet buffer
 * \param pktb Pointer to packet buffer
 */
uint8_t *pktb_data(struct pkt_buff *pktb)
{
	return pktb->data;
}

/**
 * pktb_len - return length of the packet buffer
 * \param pktb Pointer to packet buffer
 */
uint32_t pktb_len(struct pkt_buff *pktb)
{
	return pktb->len;
}

/**
 * pktb_free - release packet buffer
 * \param pktb Pointer to packet buffer
 */
void pktb_free(struct pkt_buff *pktb)
{
	free(pktb);
}

/**
 * pktb_push - update pointer to the beginning of the packet buffer
 * \param pktb Pointer to packet buffer
 */
void pktb_push(struct pkt_buff *pktb, unsigned int len)
{
	pktb->data -= len;
	pktb->len += len;
}

/**
 * pktb_pull - update pointer to the beginning of the packet buffer
 * \param pktb Pointer to packet buffer
 */
void pktb_pull(struct pkt_buff *pktb, unsigned int len)
{
	pktb->data += len;
	pktb->len -= len;
}

/**
 * pktb_put - add extra bytes to the tail of the packet buffer
 * \param pktb Pointer to packet buffer
 */
void pktb_put(struct pkt_buff *pktb, unsigned int len)
{
	pktb->tail += len;
	pktb->len += len;
}

/**
 * pktb_trim - set new length for this packet buffer
 * \param pktb Pointer to packet buffer
 */
void pktb_trim(struct pkt_buff *pktb, unsigned int len)
{
	pktb->len = len;
}

/**
 * pktb_tailroom - get room in bytes in the tail of the packet buffer
 * \param pktb Pointer to packet buffer
 */
unsigned int pktb_tailroom(struct pkt_buff *pktb)
{
	return pktb->data_len - pktb->len;
}

/**
 * pktb_mac_header - return pointer to layer 2 header (if any)
 * \param pktb Pointer to packet buffer
 */
uint8_t *pktb_mac_header(struct pkt_buff *pktb)
{
	return pktb->mac_header;
}

/**
 * pktb_network_header - return pointer to layer 3 header
 * \param pktb Pointer to packet buffer
 */
uint8_t *pktb_network_header(struct pkt_buff *pktb)
{
	return pktb->network_header;
}

/**
 * pktb_transport_header - return pointer to layer 4 header (if any)
 * \param pktb Pointer to packet buffer
 */
uint8_t *pktb_transport_header(struct pkt_buff *pktb)
{
	return pktb->transport_header;
}

static int pktb_expand_tail(struct pkt_buff *pkt, int extra)
{
	/* No room in packet, cannot mangle it. We don't support dynamic
	 * reallocation. Instead, increase the size of the extra room in
	 * the tail in pktb_alloc.
	 */
	if (pkt->len + extra > pkt->data_len)
		return 0;

	pkt->len += extra;
	pkt->tail = pkt->tail + extra;
	return 1;
}

static int enlarge_pkt(struct pkt_buff *pkt, unsigned int extra)
{
	if (pkt->len + extra > 65535)
		return 0;

	if (!pktb_expand_tail(pkt, extra - pktb_tailroom(pkt)))
		return 0;

	return 1;
}

int pktb_mangle(struct pkt_buff *pkt,
		 unsigned int dataoff,
		 unsigned int match_offset,
		 unsigned int match_len,
		 const char *rep_buffer,
		 unsigned int rep_len)
{
	unsigned char *data;

	if (rep_len > match_len &&
	    rep_len - match_len > pktb_tailroom(pkt) &&
	    !enlarge_pkt(pkt, rep_len - match_len))
		return 0;

	data = pkt->network_header + dataoff;

	/* move post-replacement */
	memmove(data + match_offset + rep_len,
		data + match_offset + match_len,
		pkt->tail - (pkt->network_header + dataoff +
			     match_offset + match_len));

	/* insert data from buffer */
	memcpy(data + match_offset, rep_buffer, rep_len);

	/* update pkt info */
	if (rep_len > match_len)
		pktb_put(pkt, rep_len - match_len);
	else
		pktb_trim(pkt, pkt->len + rep_len - match_len);

	pkt->mangled = true;
	return 1;
}
EXPORT_SYMBOL(pktb_mangle);

/**
 * pktb_mangled - return true if packet has been mangled
 * \param pktb Pointer to packet buffer
 */
bool pktb_mangled(const struct pkt_buff *pkt)
{
	return pkt->mangled;
}
EXPORT_SYMBOL(pktb_mangled);

/**
 * @}
 */
