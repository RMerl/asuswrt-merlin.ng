/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Adapted from:
 *
 * (C) 2001-2002 Magnus Boden <mb@ozaba.mine.nu>
 * (C) 2006-2012 Patrick McHardy <kaber@trash.net>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

struct tftphdr {
	uint16_t opcode;
};

#define TFTP_OPCODE_READ	1
#define TFTP_OPCODE_WRITE	2
#define TFTP_OPCODE_DATA	3
#define TFTP_OPCODE_ACK		4
#define TFTP_OPCODE_ERROR	5

static unsigned int nat_tftp(struct pkt_buff *pkt, uint32_t ctinfo,
			     struct nf_conntrack *ct, struct nf_expect *exp)
{
	struct nf_conntrack *nat_tuple;
	static uint32_t zero[4] = { 0, 0, 0, 0 };

	nat_tuple = nfct_new();
	if (nat_tuple == NULL)
		return NF_ACCEPT;

	switch (nfct_get_attr_u8(ct, ATTR_L3PROTO)) {
	case AF_INET:
		nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET);
		nfct_set_attr_u32(nat_tuple, ATTR_IPV4_SRC, 0);
		nfct_set_attr_u32(nat_tuple, ATTR_IPV4_DST, 0);
		break;
	case AF_INET6:
		nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET6);
		nfct_set_attr(nat_tuple, ATTR_IPV6_SRC, &zero);
		nfct_set_attr(nat_tuple, ATTR_IPV6_DST, &zero);
		break;
	}
	nfct_set_attr_u8(nat_tuple, ATTR_L4PROTO, IPPROTO_UDP);
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_SRC,
			  nfct_get_attr_u16(ct, ATTR_PORT_SRC));
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_DST, 0);

	nfexp_set_attr_u32(exp, ATTR_EXP_NAT_DIR, MYCT_DIR_REPL);
	nfexp_set_attr(exp, ATTR_EXP_FN, "nat-follow-master");
	nfexp_set_attr(exp, ATTR_EXP_NAT_TUPLE, nat_tuple);
	nfct_destroy(nat_tuple);

	return NF_ACCEPT;
}

static int
tftp_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
	       struct myct *myct, uint32_t ctinfo)
{
	const struct tftphdr *tfh;
	struct nf_expect *exp;
	unsigned int ret = NF_ACCEPT;
	union nfct_attr_grp_addr saddr, daddr;
	uint16_t dport;

	tfh = (struct tftphdr *)(pktb_network_header(pkt) + protoff + sizeof(struct udphdr));

	switch (ntohs(tfh->opcode)) {
	case TFTP_OPCODE_READ:
	case TFTP_OPCODE_WRITE:
		/* RRQ and WRQ works the same way */
		exp = nfexp_new();
		if (exp == NULL) {
			pr_debug("cannot alloc expectation\n");
			return NF_DROP;
		}

		cthelper_get_addr_src(myct->ct, MYCT_DIR_REPL, &saddr);
		cthelper_get_addr_dst(myct->ct, MYCT_DIR_REPL, &daddr);
		cthelper_get_port_dst(myct->ct, MYCT_DIR_REPL, &dport);

		if (cthelper_expect_init(exp, myct->ct, 0, &saddr, &daddr,
					 IPPROTO_UDP, NULL, &dport, 0)) {
			nfexp_destroy(exp);
			return NF_DROP;
		}

		if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_NAT_MASK)
			ret = nat_tftp(pkt, ctinfo, myct->ct, exp);

		myct->exp = exp;
		break;
	case TFTP_OPCODE_DATA:
	case TFTP_OPCODE_ACK:
		pr_debug("Data/ACK opcode\n");
		break;
	case TFTP_OPCODE_ERROR:
		pr_debug("Error opcode\n");
		break;
	default:
		pr_debug("Unknown opcode\n");
	}
	return ret;
}

static struct ctd_helper tftp_helper = {
	.name		= "tftp",
	.l4proto	= IPPROTO_UDP,
	.cb		= tftp_helper_cb,
	.policy		= {
		[0] = {
			.name			= "tftp",
			.expect_max		= 1,
			.expect_timeout		= 5 * 60,
		},
	},
};

static void __attribute__ ((constructor)) tftp_init(void)
{
	helper_register(&tftp_helper);
}
