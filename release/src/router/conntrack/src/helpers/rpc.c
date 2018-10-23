/*
 * (C) 2012 by Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Based on: RPC extension for conntrack.
 *
 * This port has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 *
 * Original copyright notice:
 *
 * (C) 2000 by Marcelo Barbosa Lima <marcelo.lima@dcc.unicamp.br>
 * (C) 2001 by Rusty Russell <rusty@rustcorp.com.au>
 * (C) 2002,2003 by Ian (Larry) Latter <Ian.Latter@mq.edu.au>
 * (C) 2004,2005 by David Stes <stes@pandora.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "conntrackd.h"
#include "network.h"	/* for before and after */
#include "helper.h"
#include "myct.h"
#include "log.h"

#include <errno.h>

#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>
#define _GNU_SOURCE
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_tcp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <linux/netfilter.h>

/* RFC 1050: RPC: Remote Procedure Call Protocol Specification Version 2 */
/* RFC 1014: XDR: External Data Representation Standard */
#define SUPPORTED_RPC_VERSION	2

struct rpc_info {
	/* XID */
	uint32_t xid;
	/* program */
	uint32_t pm_prog;
	/* program version */
	uint32_t pm_vers;
	/* transport protocol: TCP|UDP */
	uint32_t pm_prot;
};

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int
nf_nat_rpc(struct pkt_buff *pkt, int dir, struct nf_expect *exp,
	   uint8_t proto, uint32_t *port_ptr)
{
	const struct nf_conntrack *expected;
	struct nf_conntrack *nat_tuple;
	uint16_t initial_port, port;

	expected = nfexp_get_attr(exp, ATTR_EXP_EXPECTED);

	nat_tuple = nfct_new();
	if (nat_tuple == NULL)
		return NF_ACCEPT;

	initial_port = nfct_get_attr_u16(expected, ATTR_PORT_DST);

	nfexp_set_attr_u32(exp, ATTR_EXP_NAT_DIR, !dir);

	/* libnetfilter_conntrack needs this */
	nfct_set_attr_u8(nat_tuple, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_SRC, 0);
	nfct_set_attr_u32(nat_tuple, ATTR_IPV4_DST, 0);
	nfct_set_attr_u8(nat_tuple, ATTR_L4PROTO, proto);
	nfct_set_attr_u16(nat_tuple, ATTR_PORT_DST, 0);

	/* When you see the packet, we need to NAT it the same as the
	 * this one. */
	nfexp_set_attr(exp, ATTR_EXP_FN, "nat-follow-master");

	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(initial_port); port != 0; port++) {
		int ret;

		nfct_set_attr_u16(nat_tuple, ATTR_PORT_SRC, htons(port));
		nfexp_set_attr(exp, ATTR_EXP_NAT_TUPLE, nat_tuple);

		ret = cthelper_add_expect(exp);
		if (ret == 0)
			break;
		else if (ret != -EBUSY) {
			port = 0;
			break;
		}
	}
	nfct_destroy(nat_tuple);

	if (port == 0)
		return NF_DROP;

	*port_ptr = htonl(port);

	return NF_ACCEPT;
}

#define OFFSET(o, n)	((o) += n)
#define ROUNDUP(n)	((((n) + 3)/4)*4)

static int
rpc_call(const uint32_t *data, uint32_t offset, uint32_t datalen,
	 struct rpc_info *rpc_info)
{
	uint32_t p, r;

	/* RPC CALL message body */

	/* call_body {
	 *	rpcvers
	 *	prog
	 *	vers
	 *	proc
	 *	opaque_auth cred
	 *	opaque_auth verf
	 *	pmap
	 * }
	 *
	 * opaque_auth {
	 *	flavour
	 *	opaque[len] <= MAX_AUTH_BYTES
	 * }
	 */
	if (datalen < OFFSET(offset, 4*4 + 2*2*4)) {
		pr_debug("RPC CALL: too short packet: %u < %u\n",
			 datalen, offset);
		return -1;
	}
	/* Check rpcversion */
	p = IXDR_GET_INT32(data);
	if (p != SUPPORTED_RPC_VERSION) {
		pr_debug("RPC CALL: wrong rpcvers %u != %u\n",
			 p, SUPPORTED_RPC_VERSION);
		return -1;
	}
	/* Skip non-portmap requests */
	p = IXDR_GET_INT32(data);
	if (p != PMAPPROG) {
		pr_debug("RPC CALL: not portmap %u != %lu\n",
			 p, PMAPPROG);
		return -1;
	}
	/* Check portmap version */
	p = IXDR_GET_INT32(data);
	if (p != PMAPVERS) {
		pr_debug("RPC CALL: wrong portmap version %u != %lu\n",
			 p, PMAPVERS);
		return -1;
	}
	/* Skip non PMAPPROC_GETPORT requests */
	p = IXDR_GET_INT32(data);
	if (p != PMAPPROC_GETPORT) {
		pr_debug("RPC CALL: not PMAPPROC_GETPORT %u != %lu\n",
			 p, PMAPPROC_GETPORT);
		return -1;
	}
	/* Check and skip credentials */
	r = IXDR_GET_INT32(data);
	p = IXDR_GET_INT32(data);
	pr_debug("RPC CALL: cred: %u %u (%u, %u)\n",
		 r, p, datalen, offset);
	if (p > MAX_AUTH_BYTES) {
		pr_debug("RPC CALL: invalid sized cred %u > %u\n",
			 p, MAX_AUTH_BYTES);
		return -1;
	}
	r = ROUNDUP(p);
	if (datalen < OFFSET(offset, r)) {
		pr_debug("RPC CALL: too short to carry cred: %u < %u, %u\n",
			 datalen, offset, r);
		return -1;
	}
	data += r/4;
	/* Check and skip verifier */
	r = IXDR_GET_INT32(data);
	p = IXDR_GET_INT32(data);
	pr_debug("RPC CALL: verf: %u %u (%u, %u)\n",
		 r, p, datalen, offset);
	if (p > MAX_AUTH_BYTES) {
		pr_debug("RPC CALL: invalid sized verf %u > %u\n",
			 p, MAX_AUTH_BYTES);
		return -1;
	}
	r = ROUNDUP(p);
	if (datalen < OFFSET(offset, r)) {
		pr_debug("RPC CALL: too short to carry verf: %u < %u, %u\n",
			 datalen, offset, r);
		return -1;
	}
	data += r/4;
	/* pmap {
	 *	prog
	 *	vers
	 *	prot
	 *	port
	 * }
	 */
	/* Check CALL size */
	if (datalen != offset + 4*4) {
		pr_debug("RPC CALL: invalid size to carry pmap: %u != %u\n",
			 datalen, offset + 4*4);
		return -1;
	}
	rpc_info->pm_prog = IXDR_GET_INT32(data);
	rpc_info->pm_vers = IXDR_GET_INT32(data);
	rpc_info->pm_prot = IXDR_GET_INT32(data);
	/* Check supported protocols */
	if (!(rpc_info->pm_prot == IPPROTO_TCP
	      || rpc_info->pm_prot == IPPROTO_UDP)) {
		pr_debug("RPC CALL: unsupported protocol %u",
			 rpc_info->pm_prot);
		return -1;
	}
	p = IXDR_GET_INT32(data);
	/* Check port: must be zero */
	if (p != 0) {
		pr_debug("RPC CALL: port is nonzero %u\n",
			 ntohl(p));
		return -1;
	}
	pr_debug("RPC CALL: processed: xid %u, prog %u, vers %u, prot %u\n",
		 rpc_info->xid, rpc_info->pm_prog,
		 rpc_info->pm_vers, rpc_info->pm_prot);

	return 0;
}

static int
rpc_reply(uint32_t *data, uint32_t offset, uint32_t datalen,
	  struct rpc_info *rpc_info, uint32_t **port_ptr)
{
	uint16_t port;
	uint32_t p, r;

	/* RPC REPLY message body */

	/* reply_body {
	 *	reply_stat
	 *	xdr_union {
	 *		accepted_reply
	 *		rejected_reply
	 *	}
	 * }
	 * accepted_reply {
	 *	opaque_auth verf
	 *	accept_stat
	 *	xdr_union {
	 *		port
	 *		struct mismatch_info
	 *	}
	 * }
	 */

	/* Check size: reply status */
	if (datalen < OFFSET(offset, 4)) {
		pr_debug("RPC REPL: too short, missing rp_stat: %u < %u\n",
			 datalen, offset);
		return -1;
	}
	p = IXDR_GET_INT32(data);
	/* Check accepted request */
	if (p != MSG_ACCEPTED) {
		pr_debug("RPC REPL: not accepted %u != %u\n",
			 p, MSG_ACCEPTED);
		return -1;
	}
	/* Check and skip verifier */
	if (datalen < OFFSET(offset, 2*4)) {
		pr_debug("RPC REPL: too short, missing verf: %u < %u\n",
			 datalen, offset);
		return -1;
	}
	r = IXDR_GET_INT32(data);
	p = IXDR_GET_INT32(data);
	pr_debug("RPC REPL: verf: %u %u (%u, %u)\n",
		 r, p, datalen, offset);
	if (p > MAX_AUTH_BYTES) {
		pr_debug("RPC REPL: invalid sized verf %u > %u\n",
			 p, MAX_AUTH_BYTES);
		return -1;
	}
	r = ROUNDUP(p);
	/* verifier + ac_stat + port */
	if (datalen != OFFSET(offset, r) + 2*4) {
		pr_debug("RPC REPL: invalid size to carry verf and "
			 "success: %u != %u\n",
			 datalen, offset + 2*4);
		return -1;
	}
	data += r/4;
	/* Check success */
	p = IXDR_GET_INT32(data);
	if (p != SUCCESS) {
		pr_debug("RPC REPL: not success %u != %u\n",
			 p, SUCCESS);
		return -1;
	}
	/* Get port */
	*port_ptr = data;
	port = IXDR_GET_INT32(data); /* -Wunused-but-set-parameter */
	if (port == 0) {
		pr_debug("RPC REPL: port is zero\n");
		return -1;
	}
	pr_debug("RPC REPL: processed: xid %u, prog %u, vers %u, "
		 "prot %u, port %u\n",
		 rpc_info->xid, rpc_info->pm_prog, rpc_info->pm_vers,
		 rpc_info->pm_prot, port);
	return 0;
}

static int
rpc_helper_cb(struct pkt_buff *pkt, uint32_t protoff,
	      struct myct *myct, uint32_t ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	unsigned int offset = protoff, datalen;
	uint32_t *data, *port_ptr = NULL, xid;
	uint16_t port;
	uint8_t proto = nfct_get_attr_u8(myct->ct, ATTR_L4PROTO);
	enum msg_type rm_dir;
	struct rpc_info *rpc_info = myct->priv_data;
	union nfct_attr_grp_addr addr, daddr;
	struct nf_expect *exp = NULL;
	int ret = NF_ACCEPT;

	/* Until there's been traffic both ways, don't look into TCP packets. */
	if (proto == IPPROTO_TCP
	    && ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED_REPLY) {
		pr_debug("TCP RPC: Conntrackinfo = %u\n", ctinfo);
		return ret;
	}
	if (proto == IPPROTO_TCP) {
		struct tcphdr *th =
			(struct tcphdr *) (pktb_network_header(pkt) + protoff);
		offset += th->doff * 4;
	} else {
		offset += sizeof(struct udphdr);
	}
	/* Skip broken headers */
	if (offset % 4) {
		pr_debug("RPC: broken header: offset %u%%4 != 0\n", offset);
		return ret;
	}

	/* Take into Record Fragment header */
	if (proto == IPPROTO_TCP)
		offset += 4;

	datalen = pktb_len(pkt);
	data = (uint32_t *)(pktb_network_header(pkt) + offset);

	/* rpc_msg {
	 *	xid
	 *	direction
	 *	xdr_union {
	 *		call_body
	 *		reply_body
	 *	}
	 * }
	 */

	 /* Check minimal msg size: xid + direction */
	if (datalen < OFFSET(offset, 2*4)) {
		pr_debug("RPC: too short packet: %u < %u\n",
			 datalen, offset);
		return ret;
	}
	xid = IXDR_GET_INT32(data);
	rm_dir = IXDR_GET_INT32(data);

	/* Check direction */
	if (!((rm_dir == CALL && dir == MYCT_DIR_ORIG)
	      || (rm_dir == REPLY && dir == MYCT_DIR_REPL))) {
		pr_debug("RPC: rm_dir != dir %u != %u\n", rm_dir, dir);
		goto out;
	}

	if (rm_dir == CALL) {
		if (rpc_call(data, offset, datalen, rpc_info) < 0)
			goto out;

		rpc_info->xid = xid;

		return ret;
	} else {
		/* Check XID */
		if (xid != rpc_info->xid) {
			pr_debug("RPC REPL: XID does not match: %u != %u\n",
				 xid, rpc_info->xid);
			goto out;
		}
		if (rpc_reply(data, offset, datalen, rpc_info, &port_ptr) < 0)
			goto out;

		port = IXDR_GET_INT32(port_ptr);
		port = htons(port);

		/* We refer to the reverse direction ("!dir") tuples here,
		 * because we're expecting something in the other direction.
		 * Doesn't matter unless NAT is happening.  */
		cthelper_get_addr_dst(myct->ct, !dir, &daddr);
		cthelper_get_addr_src(myct->ct, !dir, &addr);

		exp = nfexp_new();
		if (exp == NULL)
			goto out;

		if (cthelper_expect_init(exp, myct->ct, 0, &addr, &daddr,
					 rpc_info->pm_prot,
					 NULL, &port, NF_CT_EXPECT_PERMANENT)) {
			pr_debug("RPC: failed to init expectation\n");
			goto out_exp;
		}

		/* Now, NAT might want to mangle the packet, and register the
		 * (possibly changed) expectation itself. */
		if (nfct_get_attr_u32(myct->ct, ATTR_STATUS) & IPS_NAT_MASK) {
			ret = nf_nat_rpc(pkt, dir, exp, rpc_info->pm_prot,
					 port_ptr);
			goto out_exp;
		}

		/* Can't expect this?  Best to drop packet now. */
		if (cthelper_add_expect(exp) < 0) {
			pr_debug("RPC: cannot add expectation: %s\n",
				 strerror(errno));
			ret = NF_DROP;
		}
	}

out_exp:
	nfexp_destroy(exp);
out:
	rpc_info->xid = 0;
	return ret;
}

static struct ctd_helper rpc_helper_tcp = {
	.name		= "rpc",
	.l4proto	= IPPROTO_TCP,
	.cb		= rpc_helper_cb,
	.priv_data_len	= sizeof(struct rpc_info),
	.policy		= {
		{
			.name			= "rpc",
			.expect_max		= 1,
			.expect_timeout		= 300,
		},
	},
};

static struct ctd_helper rpc_helper_udp = {
	.name		= "rpc",
	.l4proto	= IPPROTO_UDP,
	.cb		= rpc_helper_cb,
	.priv_data_len	= sizeof(struct rpc_info),
	.policy		= {
		{
			.name			= "rpc",
			.expect_max		= 1,
			.expect_timeout		= 300,
		},
	},
};

void __attribute__ ((constructor)) rpc_init(void);

void rpc_init(void)
{
	helper_register(&rpc_helper_tcp);
	helper_register(&rpc_helper_udp);
}
