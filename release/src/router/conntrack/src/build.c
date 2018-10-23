/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include "network.h"
#include "conntrackd.h"

static inline void *
put_header(struct nethdr *n, int attr, size_t len)
{
	struct netattr *nta = NETHDR_TAIL(n);
	int total_size = NTA_ALIGN(NTA_LENGTH(len));
	int attr_size = NTA_LENGTH(len);
	n->len += total_size;
	nta->nta_attr = htons(attr);
	nta->nta_len = htons(attr_size);
	memset((unsigned char *)nta + attr_size, 0, total_size - attr_size);
	return NTA_DATA(nta);
}

static inline void
addattr(struct nethdr *n, int attr, const void *data, size_t len)
{
	void *ptr = put_header(n, attr, len);
	memcpy(ptr, data, len);
}

static inline void
ct_build_u8(const struct nf_conntrack *ct, int a, struct nethdr *n, int b)
{
	void *ptr = put_header(n, b, sizeof(uint8_t));
	memcpy(ptr, nfct_get_attr(ct, a), sizeof(uint8_t));
}

static inline void 
ct_build_u16(const struct nf_conntrack *ct, int a, struct nethdr *n, int b)
{
	uint16_t data = nfct_get_attr_u16(ct, a);
	data = htons(data);
	addattr(n, b, &data, sizeof(uint16_t));
}

static inline void 
ct_build_u32(const struct nf_conntrack *ct, int a, struct nethdr *n, int b)
{
	uint32_t data = nfct_get_attr_u32(ct, a);
	data = htonl(data);
	addattr(n, b, &data, sizeof(uint32_t));
}

static inline void
ct_build_u128(const struct nf_conntrack *ct, int a, struct nethdr *n, int b)
{
	const char *data = nfct_get_attr(ct, a);
	addattr(n, b, data, sizeof(uint32_t) * 4);
}

static inline void
ct_build_str(const struct nf_conntrack *ct, int a, struct nethdr *n, int b)
{
	const void *data = nfct_get_attr(ct, a);
	addattr(n, b, data, strlen(data)+1);
}

static inline void 
ct_build_group(const struct nf_conntrack *ct, int a, struct nethdr *n, 
	      int b, int size)
{
	void *ptr = put_header(n, b, size);
	nfct_get_attr_grp(ct, a, ptr);
}

static inline void 
ct_build_natseqadj(const struct nf_conntrack *ct, struct nethdr *n)
{
	struct nta_attr_natseqadj data = {
		.orig_seq_correction_pos =
		htonl(nfct_get_attr_u32(ct, ATTR_ORIG_NAT_SEQ_CORRECTION_POS)),
		.orig_seq_offset_before = 
		htonl(nfct_get_attr_u32(ct, ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE)),
		.orig_seq_offset_after =
		htonl(nfct_get_attr_u32(ct, ATTR_ORIG_NAT_SEQ_OFFSET_AFTER)),
		.repl_seq_correction_pos = 
		htonl(nfct_get_attr_u32(ct, ATTR_REPL_NAT_SEQ_CORRECTION_POS)),
		.repl_seq_offset_before =
		htonl(nfct_get_attr_u32(ct, ATTR_REPL_NAT_SEQ_OFFSET_BEFORE)),
		.repl_seq_offset_after = 
		htonl(nfct_get_attr_u32(ct, ATTR_REPL_NAT_SEQ_OFFSET_AFTER))
	};
	addattr(n, NTA_NAT_SEQ_ADJ, &data, sizeof(struct nta_attr_natseqadj));
}

static inline void
ct_build_synproxy(const struct nf_conntrack *ct, struct nethdr *n)
{
	struct nta_attr_synproxy data = {
		.isn	= htonl(nfct_get_attr_u32(ct, ATTR_SYNPROXY_ISN)),
		.its	= htonl(nfct_get_attr_u32(ct, ATTR_SYNPROXY_ITS)),
		.tsoff	= htonl(nfct_get_attr_u32(ct, ATTR_SYNPROXY_TSOFF)),
	};
	addattr(n, NTA_SYNPROXY, &data, sizeof(struct nta_attr_synproxy));
}

static enum nf_conntrack_attr nat_type[] =
	{ ATTR_ORIG_NAT_SEQ_CORRECTION_POS, ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE,
	  ATTR_ORIG_NAT_SEQ_OFFSET_AFTER, ATTR_REPL_NAT_SEQ_CORRECTION_POS,
	  ATTR_REPL_NAT_SEQ_OFFSET_BEFORE, ATTR_REPL_NAT_SEQ_OFFSET_AFTER };

/* ICMP, UDP and TCP are always loaded with nf_conntrack_ipv4 */
static void build_l4proto_tcp(const struct nf_conntrack *ct, struct nethdr *n)
{
	if (!nfct_attr_is_set(ct, ATTR_TCP_STATE))
		return;

	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, NTA_PORT,
		      sizeof(struct nfct_attr_grp_port));
	ct_build_u8(ct, ATTR_TCP_STATE, n, NTA_TCP_STATE);
	if (CONFIG(sync).tcp_window_tracking) {
		ct_build_u8(ct, ATTR_TCP_WSCALE_ORIG, n, NTA_TCP_WSCALE_ORIG);
		ct_build_u8(ct, ATTR_TCP_WSCALE_REPL, n, NTA_TCP_WSCALE_REPL);
	}
}

static void build_l4proto_sctp(const struct nf_conntrack *ct, struct nethdr *n)
{
	/* SCTP is optional, make sure nf_conntrack_sctp is loaded */
	if (!nfct_attr_is_set(ct, ATTR_SCTP_STATE))
		return;

	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, NTA_PORT,
		      sizeof(struct nfct_attr_grp_port));
	ct_build_u8(ct, ATTR_SCTP_STATE, n, NTA_SCTP_STATE);
	ct_build_u32(ct, ATTR_SCTP_VTAG_ORIG, n, NTA_SCTP_VTAG_ORIG);
	ct_build_u32(ct, ATTR_SCTP_VTAG_REPL, n, NTA_SCTP_VTAG_REPL);
}

static void build_l4proto_dccp(const struct nf_conntrack *ct, struct nethdr *n)
{
	/* DCCP is optional, make sure nf_conntrack_dccp is loaded */
	if (!nfct_attr_is_set(ct, ATTR_DCCP_STATE))
		return;

	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, NTA_PORT,
		      sizeof(struct nfct_attr_grp_port));
	ct_build_u8(ct, ATTR_DCCP_STATE, n, NTA_DCCP_STATE);
	ct_build_u8(ct, ATTR_DCCP_ROLE, n, NTA_DCCP_ROLE);
}

static void build_l4proto_icmp(const struct nf_conntrack *ct, struct nethdr *n)
{
	/* This is also used by ICMPv6 and nf_conntrack_ipv6 is optional */
	if (!nfct_attr_is_set(ct, ATTR_ICMP_TYPE))
		return;

	ct_build_u8(ct, ATTR_ICMP_TYPE, n, NTA_ICMP_TYPE);
	ct_build_u8(ct, ATTR_ICMP_CODE, n, NTA_ICMP_CODE);
	ct_build_u16(ct, ATTR_ICMP_ID, n, NTA_ICMP_ID);
}

static void build_l4proto_udp(const struct nf_conntrack *ct, struct nethdr *n)
{
	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, NTA_PORT,
		      sizeof(struct nfct_attr_grp_port));
}

static void ct_build_clabel(const struct nf_conntrack *ct, struct nethdr *n)
{
	const struct nfct_bitmask *b;
	uint32_t *words;
	unsigned int wordcount, i, maxbit;

	if (!nfct_attr_is_set(ct, ATTR_CONNLABELS))
		return;

	b = nfct_get_attr(ct, ATTR_CONNLABELS);

	maxbit = nfct_bitmask_maxbit(b);
	for (i=0; i <= maxbit; i++) {
		if (nfct_bitmask_test_bit(b, i))
			break;
	}

	if (i > maxbit)
		return;

	wordcount = (nfct_bitmask_maxbit(b) / 32) + 1;
	words = put_header(n, NTA_LABELS, wordcount * sizeof(*words));

	for (i=0; i < wordcount; i++) {
		int bit = 31;
		uint32_t tmp = 0;

		do {
			if (nfct_bitmask_test_bit(b, (32 * i) + bit))
				tmp |= (1 << bit);
		} while (--bit >= 0);

		words[i] = htonl(tmp);
	}
}

#ifndef IPPROTO_DCCP
#define IPPROTO_DCCP 33
#endif

static struct build_l4proto {
	void (*build)(const struct nf_conntrack *, struct nethdr *n);
} l4proto_fcn[IPPROTO_MAX] = {
	[IPPROTO_TCP]		= { .build = build_l4proto_tcp },
	[IPPROTO_SCTP]		= { .build = build_l4proto_sctp },
	[IPPROTO_DCCP]		= { .build = build_l4proto_dccp },
	[IPPROTO_ICMP]		= { .build = build_l4proto_icmp },
	[IPPROTO_ICMPV6]	= { .build = build_l4proto_icmp },
	[IPPROTO_UDP]		= { .build = build_l4proto_udp },
};

void ct2msg(const struct nf_conntrack *ct, struct nethdr *n)
{
	uint8_t l4proto = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV4)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV4, n, NTA_IPV4, 
			      sizeof(struct nfct_attr_grp_ipv4));
	} else if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV6)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV6, n, NTA_IPV6, 
			      sizeof(struct nfct_attr_grp_ipv6));
	}

	ct_build_u32(ct, ATTR_STATUS, n, NTA_STATUS); 
	ct_build_u8(ct, ATTR_L4PROTO, n, NTA_L4PROTO);

	if (l4proto_fcn[l4proto].build)
		l4proto_fcn[l4proto].build(ct, n);

	if (!CONFIG(commit_timeout) && nfct_attr_is_set(ct, ATTR_TIMEOUT))
		ct_build_u32(ct, ATTR_TIMEOUT, n, NTA_TIMEOUT);
	if (nfct_attr_is_set(ct, ATTR_MARK))
		ct_build_u32(ct, ATTR_MARK, n, NTA_MARK);

	/* setup the master conntrack */
	if (nfct_attr_grp_is_set(ct, ATTR_GRP_MASTER_IPV4)) {
		ct_build_group(ct, ATTR_GRP_MASTER_IPV4, n, NTA_MASTER_IPV4,
			      sizeof(struct nfct_attr_grp_ipv4));
		ct_build_u8(ct, ATTR_MASTER_L4PROTO, n, NTA_MASTER_L4PROTO);
		if (nfct_attr_grp_is_set(ct, ATTR_GRP_MASTER_PORT)) {
			ct_build_group(ct, ATTR_GRP_MASTER_PORT,
				      n, NTA_MASTER_PORT, 
				      sizeof(struct nfct_attr_grp_port));
		}
	} else if (nfct_attr_grp_is_set(ct, ATTR_GRP_MASTER_IPV6)) {
		ct_build_group(ct, ATTR_GRP_MASTER_IPV6, n, NTA_MASTER_IPV6,
			      sizeof(struct nfct_attr_grp_ipv6));
		ct_build_u8(ct, ATTR_MASTER_L4PROTO, n, NTA_MASTER_L4PROTO);
		if (nfct_attr_grp_is_set(ct, ATTR_GRP_MASTER_PORT)) {
			ct_build_group(ct, ATTR_GRP_MASTER_PORT,
				      n, NTA_MASTER_PORT,
				      sizeof(struct nfct_attr_grp_port));
		}
	}

	/*  NAT */
	switch (nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO)) {
	case AF_INET:
		if (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT))
			ct_build_u32(ct, ATTR_REPL_IPV4_DST, n, NTA_SNAT_IPV4);
		if (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT))
			ct_build_u32(ct, ATTR_REPL_IPV4_SRC, n, NTA_DNAT_IPV4);
		break;
	case AF_INET6:
		if (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT)) {
			ct_build_u128(ct, ATTR_REPL_IPV6_DST, n,
				      NTA_SNAT_IPV6);
		}
		if (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT)) {
			ct_build_u128(ct, ATTR_REPL_IPV6_SRC, n,
				      NTA_DNAT_IPV6);
		}
		break;
	default:
		break;
	}
	if (nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT))
		ct_build_u16(ct, ATTR_REPL_PORT_DST, n, NTA_SPAT_PORT);
	if (nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT))
		ct_build_u16(ct, ATTR_REPL_PORT_SRC, n, NTA_DPAT_PORT);

	/* NAT sequence adjustment */
	if (nfct_attr_is_set_array(ct, nat_type, 6))
		ct_build_natseqadj(ct, n);

	if (nfct_attr_is_set(ct, ATTR_HELPER_NAME))
		ct_build_str(ct, ATTR_HELPER_NAME, n, NTA_HELPER_NAME);

	if (nfct_attr_is_set(ct, ATTR_CONNLABELS))
		ct_build_clabel(ct, n);

	if (nfct_attr_is_set(ct, ATTR_SYNPROXY_ISN) &&
	    nfct_attr_is_set(ct, ATTR_SYNPROXY_ITS) &&
	    nfct_attr_is_set(ct, ATTR_SYNPROXY_TSOFF))
		ct_build_synproxy(ct, n);
}

static void
exp_build_l4proto_tcp(const struct nf_conntrack *ct, struct nethdr *n, int a)
{
	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, a,
			sizeof(struct nfct_attr_grp_port));
}

static void
exp_build_l4proto_sctp(const struct nf_conntrack *ct, struct nethdr *n, int a)
{
	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, a,
			sizeof(struct nfct_attr_grp_port));
}

static void
exp_build_l4proto_dccp(const struct nf_conntrack *ct, struct nethdr *n, int a)
{
	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, a,
		      sizeof(struct nfct_attr_grp_port));
}

static void
exp_build_l4proto_udp(const struct nf_conntrack *ct, struct nethdr *n, int a)
{
	ct_build_group(ct, ATTR_GRP_ORIG_PORT, n, a,
		      sizeof(struct nfct_attr_grp_port));
}

static struct exp_build_l4proto {
	void (*build)(const struct nf_conntrack *, struct nethdr *n, int a);
} exp_l4proto_fcn[IPPROTO_MAX] = {
	[IPPROTO_TCP]		= { .build = exp_build_l4proto_tcp },
	[IPPROTO_SCTP]		= { .build = exp_build_l4proto_sctp },
	[IPPROTO_DCCP]		= { .build = exp_build_l4proto_dccp },
	[IPPROTO_UDP]		= { .build = exp_build_l4proto_udp },
};

static inline void
exp_build_u32(const struct nf_expect *exp, int a, struct nethdr *n, int b)
{
	uint32_t data = nfexp_get_attr_u32(exp, a);
	data = htonl(data);
	addattr(n, b, &data, sizeof(uint32_t));
}

static inline void
exp_build_str(const struct nf_expect *exp, int a, struct nethdr *n, int b)
{
	const char *data = nfexp_get_attr(exp, a);
	addattr(n, b, data, strlen(data)+1);
}

void exp2msg(const struct nf_expect *exp, struct nethdr *n)
{
	const struct nf_conntrack *ct = nfexp_get_attr(exp, ATTR_EXP_MASTER);
	uint8_t l4proto = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	/* master conntrack for this expectation. */
	if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV4)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV4, n, NTA_EXP_MASTER_IPV4,
			      sizeof(struct nfct_attr_grp_ipv4));
	} else if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV6)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV6, n, NTA_EXP_MASTER_IPV6,
			      sizeof(struct nfct_attr_grp_ipv6));
	}
	ct_build_u8(ct, ATTR_L4PROTO, n, NTA_EXP_MASTER_L4PROTO);

	if (exp_l4proto_fcn[l4proto].build)
		exp_l4proto_fcn[l4proto].build(ct, n, NTA_EXP_MASTER_PORT);

	/* the expectation itself. */
	ct = nfexp_get_attr(exp, ATTR_EXP_EXPECTED);

	if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV4)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV4, n, NTA_EXP_EXPECT_IPV4,
			      sizeof(struct nfct_attr_grp_ipv4));
	} else if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV6)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV6, n, NTA_EXP_EXPECT_IPV6,
			      sizeof(struct nfct_attr_grp_ipv6));
	}
	ct_build_u8(ct, ATTR_L4PROTO, n, NTA_EXP_EXPECT_L4PROTO);

	if (exp_l4proto_fcn[l4proto].build)
		exp_l4proto_fcn[l4proto].build(ct, n, NTA_EXP_EXPECT_PORT);

	/* mask for the expectation. */
	ct = nfexp_get_attr(exp, ATTR_EXP_MASK);

	if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV4)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV4, n, NTA_EXP_MASK_IPV4,
			      sizeof(struct nfct_attr_grp_ipv4));
	} else if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV6)) {
		ct_build_group(ct, ATTR_GRP_ORIG_IPV6, n, NTA_EXP_MASK_IPV6,
			      sizeof(struct nfct_attr_grp_ipv6));
	}
	ct_build_u8(ct, ATTR_L4PROTO, n, NTA_EXP_MASK_L4PROTO);

	if (exp_l4proto_fcn[l4proto].build)
		exp_l4proto_fcn[l4proto].build(ct, n, NTA_EXP_MASK_PORT);

	if (!CONFIG(commit_timeout) && nfexp_attr_is_set(exp, ATTR_EXP_TIMEOUT))
		exp_build_u32(exp, ATTR_EXP_TIMEOUT, n, NTA_EXP_TIMEOUT);

	exp_build_u32(exp, ATTR_EXP_FLAGS, n, NTA_EXP_FLAGS);
	if (nfexp_attr_is_set(exp, ATTR_EXP_CLASS))
		exp_build_u32(exp, ATTR_EXP_CLASS, n, NTA_EXP_CLASS);

	/* include NAT information, if any. */
	ct = nfexp_get_attr(exp, ATTR_EXP_NAT_TUPLE);
	if (ct != NULL) {
		if (nfct_attr_grp_is_set(ct, ATTR_GRP_ORIG_IPV4)) {
			ct_build_group(ct, ATTR_GRP_ORIG_IPV4, n,
					NTA_EXP_NAT_IPV4,
					sizeof(struct nfct_attr_grp_ipv4));
		}
		ct_build_u8(ct, ATTR_L4PROTO, n, NTA_EXP_NAT_L4PROTO);
		if (exp_l4proto_fcn[l4proto].build)
			exp_l4proto_fcn[l4proto].build(ct, n, NTA_EXP_NAT_PORT);

		exp_build_u32(exp, ATTR_EXP_NAT_DIR, n, NTA_EXP_NAT_DIR);
	}
	if (nfexp_attr_is_set(exp, ATTR_EXP_HELPER_NAME))
		exp_build_str(exp, ATTR_EXP_HELPER_NAME, n, NTA_EXP_HELPER_NAME);
	if (nfexp_attr_is_set(exp, ATTR_EXP_FN))
		exp_build_str(exp, ATTR_EXP_FN, n, NTA_EXP_FN);
}
