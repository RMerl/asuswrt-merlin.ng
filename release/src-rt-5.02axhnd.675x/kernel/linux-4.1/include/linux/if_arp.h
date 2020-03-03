/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Global definitions for the ARP (RFC 826) protocol.
 *
 * Version:	@(#)if_arp.h	1.0.1	04/16/93
 *
 * Authors:	Original taken from Berkeley UNIX 4.3, (c) UCB 1986-1988
 *		Portions taken from the KA9Q/NOS (v2.00m PA0GRI) source.
 *		Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Florian La Roche,
 *		Jonathan Layes <layes@loran.com>
 *		Arnaldo Carvalho de Melo <acme@conectiva.com.br> ARPHRD_HWX25
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_IF_ARP_H
#define _LINUX_IF_ARP_H

#include <linux/skbuff.h>
#include <uapi/linux/if_arp.h>

static inline struct arphdr *arp_hdr(const struct sk_buff *skb)
{
	return (struct arphdr *)skb_network_header(skb);
}

static inline int arp_hdr_len(struct net_device *dev)
{
	switch (dev->type) {
#if IS_ENABLED(CONFIG_FIREWIRE_NET)
	case ARPHRD_IEEE1394:
		/* ARP header, device address and 2 IP addresses */
		return sizeof(struct arphdr) + dev->addr_len + sizeof(u32) * 2;
#endif
	default:
		/* ARP header, plus 2 device addresses, plus 2 IP addresses. */
		return sizeof(struct arphdr) + (dev->addr_len + sizeof(u32)) * 2;
	}
}
#endif	/* _LINUX_IF_ARP_H */
