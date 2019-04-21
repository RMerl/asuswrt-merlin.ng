/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Adapted from:
 *
 * Amanda extension for IP connection tracking
 *
 * (C) 2002 by Brian J. Murrell <netfilter@interlinx.bc.ca>
 * based on HW's ip_conntrack_irc.c as well as other modules
 * (C) 2006 Patrick McHardy <kaber@trash.net>
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

static int nat_amanda(struct pkt_buff *pkt, uint32_t ctinfo,
		      unsigned int matchoff, unsigned int matchlen,
		      struct nf_expect *exp)
{
	char buffer[sizeof("65535")];
	uint16_t port, initial_port;
	unsigned int ret;
	const struct nf_conntrack *expected;
	struct nf_conntrack *nat_tuple;

	nat_tuple = nfct_new();
	if (nat_tuple == NULL)
		return NF_ACCEPT;

	expected = nfexp_get_attr(exp, ATTR_EXP_EXPECTED);

	/* Connection comes from client. */
	initial_port = nfct_get_attr_u16(expected, ATTR_PORT_DST);
	nfexp_set_attr_u32(exp, ATTR_EXP_NAT_DIR, IP_CT_DIR_ORIGINAL);

	/* libnetfilter_conntrack needs this */
	nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_SRC, 0);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_DST, 0);
	nfct_set_attr_u8(nat_tuple, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_DST, 0);

	/* When you see the packet, we need to NAT it the same as the
	 * this one (ie. same IP: it will be TCP and master is UDP). */
	nfexp_set_attr(exp, ATTR_EXP_FN, "nat-follow-master");

	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(initial_port); port != 0; port++) {
		int res;

		nfct_set_attr_u16(nat_tuple, ATTR_PORT_SRC, htons(port));
		nfexp_set_attr(exp, ATTR_EXP_NAT_TUPLE, nat_tuple);

		res = cthelper_add_expect(exp);
		if (res == 0)
			break;
		else if (res != -EBUSY) {
			port = 0;
			break;
		}
	}
	nfct_destroy(nat_tuple);

	if (port == 0) {
		pr_debug("all ports in use\n");
		return NF_DROP;
	}

	sprintf(buffer, "%u", port);
	ret = nfq_udp_mangle_ipv4(pkt, matchoff, matchlen, buffer,
				  strlen(buffer));
	if (ret != NF_ACCEPT) {
		pr_debug("cannot mangle packet\n");
		cthelper_del_expect(exp);
	}
	return ret;
}

static char amanda_buffer[65536];
static unsigned int master_timeout = 300;

enum amanda_strings {
	SEARCH_CONNECT,
	SEARCH_NEWLINE,
	SEARCH_DATA,
	SEARCH_MESG,
	SEARCH_INDEX,
};

static const char *conns[] = { "DATA ", "MESG ", "INDEX " };

static int
amanda_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
		 struct myct *myct, uint32_t ctinfo)
{
	struct nf_expect *exp;
	char *data, *data_limit, *tmp;
	unsigned int dataoff, i;
	uint16_t port, len;
	int ret = NF_ACCEPT;
	struct iphdr *iph;
	union nfct_attr_grp_addr saddr, daddr;

	/* Only look at packets from the Amanda server */
	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	/* increase the UDP timeout of the master connection as replies from
	 * Amanda clients to the server can be quite delayed */
	nfct_set_attr_u32(myct->ct, ATTR_TIMEOUT, master_timeout);

	/* No data? */
	iph = (struct iphdr *)pktb_network_header(pkt);
	dataoff = iph->ihl*4 + sizeof(struct udphdr);
	if (dataoff >= pktb_len(pkt)) {
		pr_debug("amanda_help: pktlen = %u\n", pktb_len(pkt));
		return NF_ACCEPT;
	}

	memcpy(amanda_buffer, pktb_network_header(pkt) + dataoff,
	       pktb_len(pkt) - dataoff);
	data = amanda_buffer;
	data_limit = amanda_buffer + pktb_len(pkt) - dataoff;
	*data_limit = '\0';

	/* Search for the CONNECT string */
	data = strstr(data, "CONNECT ");
	if (!data)
		goto out;
	data += strlen("CONNECT ");

	/* Only search first line. */
	if ((tmp = strchr(data, '\n')))
		*tmp = '\0';

	for (i = 0; i < ARRAY_SIZE(conns); i++) {
		char *match = strstr(data, conns[i]);
		if (!match)
			continue;
		tmp = data = match + strlen(conns[i]);
		port = strtoul(data, &data, 10);
		len = data - tmp;
		if (port == 0 || len > 5)
			break;

		exp = nfexp_new();
		if (exp == NULL)
			return NF_DROP;

		cthelper_get_addr_src(myct->ct, MYCT_DIR_ORIG, &saddr);
		cthelper_get_addr_dst(myct->ct, MYCT_DIR_ORIG, &daddr);
		cthelper_get_port_src(myct->ct, MYCT_DIR_ORIG, &port);

		if (cthelper_expect_init(exp, myct->ct, 0, &saddr, &daddr,
					 IPPROTO_TCP, NULL, &port, 0)) {
			nfexp_destroy(exp);
			return NF_DROP;
		}

		if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_NAT_MASK) {
			ret = nat_amanda(pkt, ctinfo, tmp - amanda_buffer,
					 len, exp);
		} else
			myct->exp = exp;
	}
out:
	return ret;
}

static struct ctd_helper amanda_helper = {
	.name		= "amanda",
	.l4proto	= IPPROTO_UDP,
	.cb		= amanda_helper_cb,
	.policy		= {
		[0] = {
			.name			= "amanda",
			.expect_max		= ARRAY_SIZE(conns),
			.expect_timeout		= 180,
		},
	},
};

void __attribute__ ((constructor)) amanda_init(void);

void amanda_init(void)
{
	helper_register(&amanda_helper);
}
