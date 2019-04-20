/*
 * Copyright (C) 2016 Google Inc.
 * Author: Kevin Cernekee <cernekee@chromium.org>
 *
 * This helper pokes a hole in the firewall for unicast mDNS replies
 * (RFC6762 section 5.1).  It is needed because the destination address of
 * the outgoing mDNS query (224.0.0.251) will not match the source address
 * of the incoming response (192.168.x.x or similar).  The helper is not used
 * for standard multicast queries/responses in which the sport and dport are
 * both 5353, because those can be handled by a standard filter/INPUT rule.
 *
 * Usage:
 *
 *     nfct add helper mdns inet udp
 *     iptables -t raw -A OUTPUT -p udp -d 224.0.0.251 '!' --sport 5353 \
 *         --dport 5353 -j CT --helper mdns
 *     iptables -t filter -A INPUT -p udp -d 224.0.0.251 --dport 5353 -j ACCEPT
 *     iptables -t filter -A INPUT -m state --state ESTABLISHED,RELATED \
 *         -j ACCEPT
 *
 * Requires Linux 3.12 or higher.  NAT is unsupported.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "conntrackd.h"
#include "helper.h"
#include "myct.h"
#include "log.h"

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <linux/netfilter.h>

static int mdns_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
			  struct myct *myct, uint32_t ctinfo)
{
	struct nf_expect *exp;
	int dir = CTINFO2DIR(ctinfo);
	union nfct_attr_grp_addr saddr;
	uint16_t sport, dport;

	exp = nfexp_new();
	if (exp == NULL) {
		pr_debug("conntrack_mdns: failed to allocate expectation\n");
		return NF_ACCEPT;
	}

	cthelper_get_addr_src(myct->ct, dir, &saddr);
	cthelper_get_port_src(myct->ct, dir, &sport);
	cthelper_get_port_src(myct->ct, !dir, &dport);

	if (cthelper_expect_init(exp,
				 myct->ct,
				 0 /* class */,
				 NULL /* saddr */,
				 &saddr /* daddr */,
				 IPPROTO_UDP,
				 &dport /* sport */,
				 &sport /* dport */,
				 NF_CT_EXPECT_PERMANENT)) {
		pr_debug("conntrack_mdns: failed to init expectation\n");
		nfexp_destroy(exp);
		return NF_ACCEPT;
	}

	myct->exp = exp;
	return NF_ACCEPT;
}

static struct ctd_helper mdns_helper = {
	.name		= "mdns",
	.l4proto	= IPPROTO_UDP,
	.priv_data_len	= 0,
	.cb		= mdns_helper_cb,
	.policy		= {
		[0] = {
			.name		= "mdns",
			.expect_max	= 8,
			.expect_timeout	= 30,
		},
	},
};

static void __attribute__ ((constructor)) mdns_init(void)
{
	helper_register(&mdns_helper);
}
