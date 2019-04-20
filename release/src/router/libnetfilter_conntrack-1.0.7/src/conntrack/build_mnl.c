/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "internal/internal.h"
#include <limits.h>
#include <libmnl/libmnl.h>

static int
nfct_build_tuple_ip(struct nlmsghdr *nlh, const struct __nfct_tuple *t)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_TUPLE_IP);
	if (nest == NULL)
		return -1;

	switch(t->l3protonum) {
	case AF_INET:
		mnl_attr_put_u32(nlh, CTA_IP_V4_SRC, t->src.v4);
		mnl_attr_put_u32(nlh, CTA_IP_V4_DST, t->dst.v4);
		break;
	case AF_INET6:
		mnl_attr_put(nlh, CTA_IP_V6_SRC, sizeof(struct in6_addr),
				&t->src.v6);
		mnl_attr_put(nlh, CTA_IP_V6_DST, sizeof(struct in6_addr),
				&t->dst.v6);
		break;
	default:
		mnl_attr_nest_cancel(nlh, nest);
		return -1;
	}
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_tuple_proto(struct nlmsghdr *nlh, const struct __nfct_tuple *t)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_TUPLE_PROTO);
	if (nest == NULL)
		return -1;

	mnl_attr_put_u8(nlh, CTA_PROTO_NUM, t->protonum);

	switch(t->protonum) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_SCTP:
	case IPPROTO_DCCP:
	case IPPROTO_GRE:
	case IPPROTO_UDPLITE:
		mnl_attr_put_u16(nlh, CTA_PROTO_SRC_PORT, t->l4src.tcp.port);
		mnl_attr_put_u16(nlh, CTA_PROTO_DST_PORT, t->l4dst.tcp.port);
		break;
	case IPPROTO_ICMP:
		mnl_attr_put_u8(nlh, CTA_PROTO_ICMP_CODE, t->l4dst.icmp.code);
		mnl_attr_put_u8(nlh, CTA_PROTO_ICMP_TYPE, t->l4dst.icmp.type);
		mnl_attr_put_u16(nlh, CTA_PROTO_ICMP_ID, t->l4src.icmp.id);
		break;
	case IPPROTO_ICMPV6:
		mnl_attr_put_u8(nlh, CTA_PROTO_ICMPV6_CODE, t->l4dst.icmp.code);
		mnl_attr_put_u8(nlh, CTA_PROTO_ICMPV6_TYPE, t->l4dst.icmp.type);
		mnl_attr_put_u16(nlh, CTA_PROTO_ICMPV6_ID, t->l4src.icmp.id);
		break;
	default:
		mnl_attr_nest_cancel(nlh, nest);
		return -1;
	}
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

int
nfct_build_tuple_raw(struct nlmsghdr *nlh, const struct __nfct_tuple *t)
{
	if (nfct_build_tuple_ip(nlh, t) < 0)
		return -1;
	if (nfct_build_tuple_proto(nlh, t) < 0)
		return -1;

	return 0;
}

int
nfct_build_tuple(struct nlmsghdr *nlh, const struct __nfct_tuple *t, int type)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, type);
	if (nest == NULL)
		return -1;

	if (nfct_build_tuple_raw(nlh, t) < 0)
		goto err;

	mnl_attr_nest_end(nlh, nest);
	return 0;
err:
	mnl_attr_nest_cancel(nlh, nest);
	return -1;
}

static int
nfct_build_protoinfo(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest, *nest_proto;

	switch(ct->head.orig.protonum) {
	case IPPROTO_TCP:
		/* Preliminary attribute check to avoid sending an empty
		 * CTA_PROTOINFO_TCP nest, which results in EINVAL in
		 * Linux kernel <= 2.6.25. */
		if (!(test_bit(ATTR_TCP_STATE, ct->head.set) ||
		      test_bit(ATTR_TCP_FLAGS_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_FLAGS_REPL, ct->head.set) ||
		      test_bit(ATTR_TCP_MASK_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_MASK_REPL, ct->head.set) ||
		      test_bit(ATTR_TCP_WSCALE_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_WSCALE_REPL, ct->head.set))) {
			break;
		}
		nest = mnl_attr_nest_start(nlh, CTA_PROTOINFO);
		nest_proto = mnl_attr_nest_start(nlh, CTA_PROTOINFO_TCP);
		if (test_bit(ATTR_TCP_STATE, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_TCP_STATE,
					ct->protoinfo.tcp.state);
		}
		if (test_bit(ATTR_TCP_FLAGS_ORIG, ct->head.set) &&
		    test_bit(ATTR_TCP_MASK_ORIG, ct->head.set)) {
			mnl_attr_put(nlh, CTA_PROTOINFO_TCP_FLAGS_ORIGINAL,
				     sizeof(struct nf_ct_tcp_flags),
				     &ct->protoinfo.tcp.flags[0]);
		}
		if (test_bit(ATTR_TCP_FLAGS_REPL, ct->head.set) &&
		    test_bit(ATTR_TCP_MASK_REPL, ct->head.set)) {
			mnl_attr_put(nlh, CTA_PROTOINFO_TCP_FLAGS_REPLY,
				     sizeof(struct nf_ct_tcp_flags),
				     &ct->protoinfo.tcp.flags[1]);
		}
		if (test_bit(ATTR_TCP_WSCALE_ORIG, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_TCP_WSCALE_ORIGINAL,
					ct->protoinfo.tcp.wscale[__DIR_ORIG]);
		}
		if (test_bit(ATTR_TCP_WSCALE_REPL, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_TCP_WSCALE_REPLY,
					ct->protoinfo.tcp.wscale[__DIR_REPL]);
		}
		mnl_attr_nest_end(nlh, nest_proto);
		mnl_attr_nest_end(nlh, nest);
		break;
	case IPPROTO_SCTP:
		/* See comment above on TCP. */
		if (!(test_bit(ATTR_SCTP_STATE, ct->head.set) ||
		      test_bit(ATTR_SCTP_VTAG_ORIG, ct->head.set) ||
		      test_bit(ATTR_SCTP_VTAG_REPL, ct->head.set))) {
			break;
		}
		nest = mnl_attr_nest_start(nlh, CTA_PROTOINFO);
		nest_proto = mnl_attr_nest_start(nlh, CTA_PROTOINFO_SCTP);

		if (test_bit(ATTR_SCTP_STATE, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_SCTP_STATE,
					ct->protoinfo.sctp.state);
		}
		if (test_bit(ATTR_SCTP_VTAG_ORIG, ct->head.set)) {
			mnl_attr_put_u32(nlh, CTA_PROTOINFO_SCTP_VTAG_ORIGINAL,
				htonl(ct->protoinfo.sctp.vtag[__DIR_ORIG]));
		}
		if (test_bit(ATTR_SCTP_VTAG_REPL, ct->head.set)) {
			mnl_attr_put_u32(nlh, CTA_PROTOINFO_SCTP_VTAG_REPLY,
				htonl(ct->protoinfo.sctp.vtag[__DIR_REPL]));
		}
		mnl_attr_nest_end(nlh, nest_proto);
		mnl_attr_nest_end(nlh, nest);
		break;
	case IPPROTO_DCCP:
		/* See comment above on TCP. */
		if (!(test_bit(ATTR_DCCP_STATE, ct->head.set) ||
		      test_bit(ATTR_DCCP_ROLE, ct->head.set) ||
		      test_bit(ATTR_DCCP_HANDSHAKE_SEQ, ct->head.set))) {
			break;
		}
		nest = mnl_attr_nest_start(nlh, CTA_PROTOINFO);
		nest_proto = mnl_attr_nest_start(nlh, CTA_PROTOINFO_DCCP);
		if (test_bit(ATTR_DCCP_STATE, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_DCCP_STATE,
					ct->protoinfo.dccp.state);
		}
		if (test_bit(ATTR_DCCP_ROLE, ct->head.set)) {
			mnl_attr_put_u8(nlh, CTA_PROTOINFO_DCCP_ROLE,
					ct->protoinfo.dccp.role);
		}
		if (test_bit(ATTR_DCCP_HANDSHAKE_SEQ, ct->head.set)) {
			uint64_t handshake_seq =
				be64toh(ct->protoinfo.dccp.handshake_seq);

			mnl_attr_put_u64(nlh, CTA_PROTOINFO_DCCP_HANDSHAKE_SEQ,
					 handshake_seq);
		}
		mnl_attr_nest_end(nlh, nest_proto);
		mnl_attr_nest_end(nlh, nest);
	default:
		break;
	}
	return 0;
}

static int
nfct_nat_seq_adj(struct nlmsghdr *nlh, const struct nf_conntrack *ct, int dir)
{
	mnl_attr_put_u32(nlh, CTA_NAT_SEQ_CORRECTION_POS,
			 htonl(ct->natseq[dir].correction_pos));
	mnl_attr_put_u32(nlh, CTA_NAT_SEQ_OFFSET_BEFORE,
			 htonl(ct->natseq[dir].offset_before));
	mnl_attr_put_u32(nlh, CTA_NAT_SEQ_OFFSET_AFTER,
			 htonl(ct->natseq[dir].offset_after));
	return 0;
}

static int
nfct_build_nat_seq_adj(struct nlmsghdr *nlh, const struct nf_conntrack *ct,
		      int dir)
{
	int type = (dir == __DIR_ORIG) ? CTA_NAT_SEQ_ADJ_ORIG :
					 CTA_NAT_SEQ_ADJ_REPLY;
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, type);
	nfct_nat_seq_adj(nlh, ct, dir);
	mnl_attr_nest_end(nlh, nest);

	return 0;
}

static int
nfct_build_protonat(struct nlmsghdr *nlh, const struct nf_conntrack *ct,
		   const struct __nfct_nat *nat)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_PROTO);

	switch (ct->head.orig.protonum) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		mnl_attr_put_u16(nlh, CTA_PROTONAT_PORT_MIN,
				 nat->l4min.tcp.port);
		mnl_attr_put_u16(nlh, CTA_PROTONAT_PORT_MAX,
				 nat->l4max.tcp.port);
	break;
	}
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_nat(struct nlmsghdr *nlh, const struct __nfct_nat *nat,
	       uint8_t l3protonum)
{
	switch (l3protonum) {
	case AF_INET:
		mnl_attr_put_u32(nlh, CTA_NAT_MINIP, nat->min_ip.v4);
		break;
	case AF_INET6:
		mnl_attr_put(nlh, CTA_NAT_V6_MINIP, sizeof(struct in6_addr),
			     &nat->min_ip.v6);
		break;
	default:
		break;
	}
	return 0;
}

static int
nfct_build_snat(struct nlmsghdr *nlh, const struct nf_conntrack *ct,
		uint8_t l3protonum)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_SRC);
	nfct_build_nat(nlh, &ct->snat, l3protonum);
	nfct_build_protonat(nlh, ct, &ct->snat);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_snat_ipv4(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_SRC);
	nfct_build_nat(nlh, &ct->snat, AF_INET);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_snat_ipv6(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_SRC);
	nfct_build_nat(nlh, &ct->snat, AF_INET6);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_snat_port(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_SRC);
	nfct_build_protonat(nlh, ct, &ct->snat);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_dnat(struct nlmsghdr *nlh, const struct nf_conntrack *ct,
		uint8_t l3protonum)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_DST);
	nfct_build_nat(nlh, &ct->dnat, l3protonum);
	nfct_build_protonat(nlh, ct, &ct->dnat);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_dnat_ipv4(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_DST);
	nfct_build_nat(nlh, &ct->dnat, AF_INET);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_dnat_ipv6(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_DST);
	nfct_build_nat(nlh, &ct->dnat, AF_INET6);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_dnat_port(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_NAT_DST);
	nfct_build_protonat(nlh, ct, &ct->dnat);
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_status(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	mnl_attr_put_u32(nlh, CTA_STATUS, htonl(ct->status | IPS_CONFIRMED));
	return 0;
}

static int
nfct_build_timeout(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	mnl_attr_put_u32(nlh, CTA_TIMEOUT, htonl(ct->timeout));
	return 0;
}

static int
nfct_build_mark(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	mnl_attr_put_u32(nlh, CTA_MARK, htonl(ct->mark));
	return 0;
}

static int
nfct_build_secmark(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	mnl_attr_put_u32(nlh, CTA_SECMARK, htonl(ct->secmark));
	return 0;
}

static int
nfct_build_helper_name(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_HELP);
	mnl_attr_put_strz(nlh, CTA_HELP_NAME, ct->helper_name);

	if (ct->helper_info != NULL) {
		mnl_attr_put(nlh, CTA_HELP_INFO, ct->helper_info_len,
				ct->helper_info);
	}
	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int
nfct_build_zone(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	mnl_attr_put_u16(nlh, CTA_ZONE, htons(ct->zone));
	return 0;
}

static void
nfct_build_labels(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	struct nfct_bitmask *b = ct->connlabels;
	unsigned int size = b->words * sizeof(b->bits[0]);
	mnl_attr_put(nlh, CTA_LABELS, size, b->bits);

	if (test_bit(ATTR_CONNLABELS_MASK, ct->head.set)) {
		b = ct->connlabels_mask;
		if (size == (b->words * sizeof(b->bits[0])))
			mnl_attr_put(nlh, CTA_LABELS_MASK, size, b->bits);
	}
}

static void nfct_build_synproxy(struct nlmsghdr *nlh,
				const struct nf_conntrack *ct)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, CTA_SYNPROXY);
	mnl_attr_put_u32(nlh, CTA_SYNPROXY_ISN, htonl(ct->synproxy.isn));
	mnl_attr_put_u32(nlh, CTA_SYNPROXY_ITS, htonl(ct->synproxy.its));
	mnl_attr_put_u32(nlh, CTA_SYNPROXY_TSOFF, htonl(ct->synproxy.tsoff));
	mnl_attr_nest_end(nlh, nest);
}

int
nfct_nlmsg_build(struct nlmsghdr *nlh, const struct nf_conntrack *ct)
{
	if (!test_bit(ATTR_ORIG_L3PROTO, ct->head.set)) {
		errno = EINVAL;
		return -1;
	}

	if (test_bit(ATTR_ORIG_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_L3PROTO, ct->head.set) ||
	    test_bit(ATTR_ORIG_L4PROTO, ct->head.set) ||
	    test_bit(ATTR_ORIG_ZONE, ct->head.set) ||
	    test_bit(ATTR_ICMP_TYPE, ct->head.set) ||
	    test_bit(ATTR_ICMP_CODE, ct->head.set) ||
	    test_bit(ATTR_ICMP_ID, ct->head.set)) {
		const struct __nfct_tuple *t = &ct->head.orig;
		struct nlattr *nest;

		nest = mnl_attr_nest_start(nlh, CTA_TUPLE_ORIG);
		if (nest == NULL)
			return -1;

		if (nfct_build_tuple_raw(nlh, t) < 0) {
			mnl_attr_nest_cancel(nlh, nest);
			return -1;
		}

		if (test_bit(ATTR_ORIG_ZONE, ct->head.set))
			mnl_attr_put_u16(nlh, CTA_TUPLE_ZONE, htons(t->zone));

		mnl_attr_nest_end(nlh, nest);
	}

	if (test_bit(ATTR_REPL_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_L3PROTO, ct->head.set) ||
	    test_bit(ATTR_REPL_L4PROTO, ct->head.set) ||
	    test_bit(ATTR_REPL_ZONE, ct->head.set) ||
	    test_bit(ATTR_ICMP_TYPE, ct->head.set) ||
	    test_bit(ATTR_ICMP_CODE, ct->head.set) ||
	    test_bit(ATTR_ICMP_ID, ct->head.set)) {
		const struct __nfct_tuple *t = &ct->repl;
		struct nlattr *nest;

		nest = mnl_attr_nest_start(nlh, CTA_TUPLE_REPLY);
		if (nest == NULL)
			return -1;

		if (nfct_build_tuple_raw(nlh, t) < 0) {
			mnl_attr_nest_cancel(nlh, nest);
			return -1;
		}

		if (test_bit(ATTR_REPL_ZONE, ct->head.set))
			mnl_attr_put_u16(nlh, CTA_TUPLE_ZONE, htons(t->zone));

		mnl_attr_nest_end(nlh, nest);
	}

	if (test_bit(ATTR_MASTER_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_L3PROTO, ct->head.set) ||
	    test_bit(ATTR_MASTER_L4PROTO, ct->head.set)) {
		nfct_build_tuple(nlh, &ct->master, CTA_TUPLE_MASTER);
	}

	if (test_bit(ATTR_STATUS, ct->head.set))
		nfct_build_status(nlh, ct);

	if (test_bit(ATTR_TIMEOUT, ct->head.set))
		nfct_build_timeout(nlh, ct);

	if (test_bit(ATTR_MARK, ct->head.set))
		nfct_build_mark(nlh, ct);

	if (test_bit(ATTR_SECMARK, ct->head.set))
		nfct_build_secmark(nlh, ct);

	nfct_build_protoinfo(nlh, ct);

	if (test_bit(ATTR_SNAT_IPV4, ct->head.set) &&
	    test_bit(ATTR_SNAT_PORT, ct->head.set)) {
		nfct_build_snat(nlh, ct, AF_INET);
	} else if (test_bit(ATTR_SNAT_IPV6, ct->head.set) &&
		   test_bit(ATTR_SNAT_PORT, ct->head.set)) {
		nfct_build_snat(nlh, ct, AF_INET6);
	} else if (test_bit(ATTR_SNAT_IPV4, ct->head.set)) {
		nfct_build_snat_ipv4(nlh, ct);
	} else if (test_bit(ATTR_SNAT_IPV6, ct->head.set)) {
		nfct_build_snat_ipv6(nlh, ct);
	} else if (test_bit(ATTR_SNAT_PORT, ct->head.set)) {
		nfct_build_snat_port(nlh, ct);
	}

	if (test_bit(ATTR_DNAT_IPV4, ct->head.set) &&
	    test_bit(ATTR_DNAT_PORT, ct->head.set)) {
		nfct_build_dnat(nlh, ct, AF_INET);
	} else if (test_bit(ATTR_DNAT_IPV6, ct->head.set) &&
		   test_bit(ATTR_DNAT_PORT, ct->head.set)) {
		nfct_build_dnat(nlh, ct, AF_INET6);
	} else if (test_bit(ATTR_DNAT_IPV4, ct->head.set)) {
		nfct_build_dnat_ipv4(nlh, ct);
	} else if (test_bit(ATTR_DNAT_IPV6, ct->head.set)) {
		nfct_build_dnat_ipv6(nlh, ct);
	} else if (test_bit(ATTR_DNAT_PORT, ct->head.set)) {
		nfct_build_dnat_port(nlh, ct);
	}

	if (test_bit(ATTR_ORIG_NAT_SEQ_CORRECTION_POS, ct->head.set) &&
	    test_bit(ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE, ct->head.set) &&
	    test_bit(ATTR_ORIG_NAT_SEQ_OFFSET_AFTER, ct->head.set)) {
		nfct_build_nat_seq_adj(nlh, ct, __DIR_ORIG);
	}
	if (test_bit(ATTR_REPL_NAT_SEQ_CORRECTION_POS, ct->head.set) &&
	    test_bit(ATTR_REPL_NAT_SEQ_OFFSET_BEFORE, ct->head.set) &&
	    test_bit(ATTR_REPL_NAT_SEQ_OFFSET_AFTER, ct->head.set)) {
		nfct_build_nat_seq_adj(nlh, ct, __DIR_REPL);
	}

	if (test_bit(ATTR_HELPER_NAME, ct->head.set))
		nfct_build_helper_name(nlh, ct);

	if (test_bit(ATTR_ZONE, ct->head.set))
		nfct_build_zone(nlh, ct);

	if (test_bit(ATTR_CONNLABELS, ct->head.set))
		nfct_build_labels(nlh, ct);

	if (test_bit(ATTR_SYNPROXY_ISN, ct->head.set) &&
	    test_bit(ATTR_SYNPROXY_ITS, ct->head.set) &&
	    test_bit(ATTR_SYNPROXY_TSOFF, ct->head.set))
		nfct_build_synproxy(nlh, ct);

	return 0;
}
