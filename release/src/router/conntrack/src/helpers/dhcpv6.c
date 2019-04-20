/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Adapted from:
 *
 * DHCPv6 multicast connection tracking helper.
 *
 * (c) 2012 Google Inc.
 *
 * Original author: Darren Willis <djw@google.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include "conntrackd.h"
#include "helper.h"
#include "myct.h"
#include "log.h"
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_udp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <linux/netfilter.h>

#define DHCPV6_CLIENT_PORT 546

static uint16_t dhcpv6_port;

/* Timeouts for DHCPv6 replies, in seconds, indexed by message type. */
static const int dhcpv6_timeouts[] = {
	0,	/* No message has type 0. */
	120,	/* Solicit. */
	0,	/* Advertise. */
	30,	/* Request. */
	4,	/* Confirm. */
	600,	/* Renew. */
	600,	/* Rebind. */
	0,	/* Reply. */
	1,	/* Release. */
	1,	/* Decline. */
	0,	/* Reconfigure. */
	120,	/* Information Request. */
	0,	/* Relay-forward. */
	0	/* Relay-reply. */
};

static inline int ipv6_addr_is_multicast(const struct in6_addr *addr)
{
	return (addr->s6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000);
}

static int
dhcpv6_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
		 struct myct *myct, uint32_t ctinfo)
{
	struct iphdr *iph = (struct iphdr *)pktb_network_header(pkt);
	struct ip6_hdr *ip6h = (struct ip6_hdr *)pktb_network_header(pkt);
	int dir = CTINFO2DIR(ctinfo);
	union nfct_attr_grp_addr addr;
	struct nf_expect *exp;
	uint8_t *dhcpv6_msg_type;

	if (iph->version != 6 || !ipv6_addr_is_multicast(&ip6h->ip6_dst))
		return NF_ACCEPT;

	dhcpv6_msg_type = pktb_network_header(pkt) + protoff + sizeof(struct udphdr);
	if (*dhcpv6_msg_type > ARRAY_SIZE(dhcpv6_timeouts)) {
		printf("Dropping DHCPv6 message with bad type %u\n",
			*dhcpv6_msg_type);
		return NF_DROP;
	}

	exp = nfexp_new();
	if (exp == NULL)
		return NF_ACCEPT;

	cthelper_get_addr_src(myct->ct, dir, &addr);

	if (cthelper_expect_init(exp, myct->ct, 0, NULL, &addr,
				 IPPROTO_UDP, NULL, &dhcpv6_port,
				 NF_CT_EXPECT_PERMANENT)) {
		nfexp_destroy(exp);
		return NF_DROP;
	}

	myct->exp = exp;

	if (dhcpv6_timeouts[*dhcpv6_msg_type] > 0) {
		nfct_set_attr_u32(myct->ct, ATTR_TIMEOUT,
				  dhcpv6_timeouts[*dhcpv6_msg_type]);
	}

	return NF_ACCEPT;
}

static struct ctd_helper dhcpv6_helper = {
	.name		= "dhcpv6",
	.l4proto	= IPPROTO_UDP,
	.cb		= dhcpv6_helper_cb,
	.policy		= {
		[0] = {
			.name			= "dhcpv6",
			.expect_max		= 1,
			.expect_timeout		= 300,
		},
	},
};

void __attribute__ ((constructor)) dhcpv6_init(void);

void dhcpv6_init(void)
{
	dhcpv6_port = htons(DHCPV6_CLIENT_PORT);
	helper_register(&dhcpv6_helper);
}
