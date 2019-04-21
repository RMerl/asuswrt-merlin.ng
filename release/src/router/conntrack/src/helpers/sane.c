/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Port this helper to userspace.
 */

/* SANE connection tracking helper
 * (SANE = Scanner Access Now Easy)
 * For documentation about the SANE network protocol see
 * http://www.sane-project.org/html/doc015.html
 */

/*
 * Copyright (C) 2007 Red Hat, Inc.
 * Author: Michal Schmidt <mschmidt@redhat.com>
 * Based on the FTP conntrack helper (net/netfilter/nf_conntrack_ftp.c):
 *  (C) 1999-2001 Paul `Rusty' Russell
 *  (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *  (C) 2003,2004 USAGI/WIDE Project <http://www.linux-ipv6.org>
 *  (C) 2003 Yasuyuki Kozakai @USAGI <yasuyuki.kozakai@toshiba.co.jp>
 *
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
#define _GNU_SOURCE
#include <netinet/tcp.h>
#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_tcp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <linux/netfilter.h>

enum sane_state {
	SANE_STATE_NORMAL,
	SANE_STATE_START_REQUESTED,
};

struct sane_request {
	uint32_t RPC_code;
#define SANE_NET_START      7   /* RPC code */

	uint32_t handle;
};

struct sane_reply_net_start {
	uint32_t status;
#define SANE_STATUS_SUCCESS 0

	uint16_t zero;
	uint16_t port;
	/* other fields aren't interesting for conntrack */
};

struct nf_ct_sane_master {
	enum sane_state state;
};

static int
sane_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
		 struct myct *myct, uint32_t ctinfo)
{
	unsigned int dataoff, datalen;
	const struct tcphdr *th;
	void *sb_ptr;
	int ret = NF_ACCEPT;
	int dir = CTINFO2DIR(ctinfo);
	struct nf_ct_sane_master *ct_sane_info = myct->priv_data;
	struct nf_expect *exp;
	struct sane_request *req;
	struct sane_reply_net_start *reply;
	union nfct_attr_grp_addr saddr;
	union nfct_attr_grp_addr daddr;

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED &&
	    ctinfo != IP_CT_ESTABLISHED_REPLY)
		return NF_ACCEPT;

	th = (struct tcphdr *)(pktb_network_header(pkt) + protoff);

	/* No data? */
	dataoff = protoff + th->doff * 4;
	if (dataoff >= pktb_len(pkt))
		return NF_ACCEPT;

	datalen = pktb_len(pkt) - dataoff;

	sb_ptr = pktb_network_header(pkt) + dataoff;

	if (dir == MYCT_DIR_ORIG) {
		if (datalen != sizeof(struct sane_request))
			goto out;

		req = sb_ptr;
		if (req->RPC_code != htonl(SANE_NET_START)) {
			/* Not an interesting command */
			ct_sane_info->state = SANE_STATE_NORMAL;
			goto out;
		}

		/* We're interested in the next reply */
		ct_sane_info->state = SANE_STATE_START_REQUESTED;
		goto out;
	}

	/* Is it a reply to an uninteresting command? */
	if (ct_sane_info->state != SANE_STATE_START_REQUESTED)
		goto out;

	/* It's a reply to SANE_NET_START. */
	ct_sane_info->state = SANE_STATE_NORMAL;

	if (datalen < sizeof(struct sane_reply_net_start)) {
		pr_debug("nf_ct_sane: NET_START reply too short\n");
		goto out;
	}

	reply = sb_ptr;
	if (reply->status != htonl(SANE_STATUS_SUCCESS)) {
		/* saned refused the command */
		pr_debug("nf_ct_sane: unsuccessful SANE_STATUS = %u\n",
			 ntohl(reply->status));
		goto out;
	}

	/* Invalid saned reply? Ignore it. */
	if (reply->zero != 0)
		goto out;

	exp = nfexp_new();
	if (exp == NULL)
		return NF_DROP;

	cthelper_get_addr_src(myct->ct, MYCT_DIR_ORIG, &saddr);
	cthelper_get_addr_dst(myct->ct, MYCT_DIR_ORIG, &daddr);

	if (cthelper_expect_init(exp, myct->ct, 0, &saddr, &daddr,
				 IPPROTO_TCP, NULL, &reply->port, 0)) {
		nfexp_destroy(exp);
		return NF_DROP;
	}
	myct->exp = exp;
out:
	return ret;
}

static struct ctd_helper sane_helper = {
	.name		= "sane",
	.l4proto	= IPPROTO_TCP,
	.priv_data_len	= sizeof(struct nf_ct_sane_master),
	.cb		= sane_helper_cb,
	.policy		= {
		[0] = {
			.name			= "sane",
			.expect_max		= 1,
			.expect_timeout		= 5 * 60,
		},
	},
};

static void __attribute__ ((constructor)) sane_init(void)
{
	helper_register(&sane_helper);
}
