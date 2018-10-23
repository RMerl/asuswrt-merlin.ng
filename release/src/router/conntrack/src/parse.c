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

#include "network.h"

#include <stdlib.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#ifndef ssizeof
#define ssizeof(x) (int)sizeof(x)
#endif

static void ct_parse_u8(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_u16(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_u32(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_u128(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_str(struct nf_conntrack *ct,
			 const struct netattr *, void *data);
static void ct_parse_group(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_nat_seq_adj(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_synproxy(struct nf_conntrack *ct, int attr, void *data);
static void ct_parse_clabel(struct nf_conntrack *ct,
			    const struct netattr *, void *data);

struct ct_parser {
	void 	(*parse)(struct nf_conntrack *ct, int attr, void *data);
	void	(*parse2)(struct nf_conntrack *ct, const struct netattr *, void *);
	uint16_t attr;
	uint16_t size;
	uint16_t max_size;
};

static struct ct_parser h[NTA_MAX] = {
	[NTA_IPV4] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_ORIG_IPV4,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_IPV6] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_ORIG_IPV6,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv6)),
	},
	[NTA_PORT] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_ORIG_PORT,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_L4PROTO] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_L4PROTO,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_TCP_STATE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_TCP_STATE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_STATUS] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_STATUS,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_MARK] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_MARK,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_TIMEOUT] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_TIMEOUT,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_MASTER_IPV4] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_MASTER_IPV4,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_MASTER_IPV6] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_MASTER_IPV6,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv6)),
	},
	[NTA_MASTER_L4PROTO] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_MASTER_L4PROTO,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_MASTER_PORT] = {
		.parse	= ct_parse_group,
		.attr	= ATTR_GRP_MASTER_PORT,
		.size	= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_SNAT_IPV4]	= {
		.parse	= ct_parse_u32,
		.attr	= ATTR_SNAT_IPV4,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_DNAT_IPV4] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_DNAT_IPV4,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_SPAT_PORT]	= {
		.parse	= ct_parse_u16,
		.attr	= ATTR_SNAT_PORT,
		.size	= NTA_SIZE(sizeof(uint16_t)),
	},
	[NTA_DPAT_PORT]	= {
		.parse	= ct_parse_u16,
		.attr	= ATTR_DNAT_PORT,
		.size	= NTA_SIZE(sizeof(uint16_t)),
	},
	[NTA_NAT_SEQ_ADJ] = {
		.parse	= ct_parse_nat_seq_adj,
		.size	= NTA_SIZE(sizeof(struct nta_attr_natseqadj)),
	},
	[NTA_SCTP_STATE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_SCTP_STATE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_SCTP_VTAG_ORIG] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_SCTP_VTAG_ORIG,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_SCTP_VTAG_REPL] = {
		.parse	= ct_parse_u32,
		.attr	= ATTR_SCTP_VTAG_REPL,
		.size	= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_DCCP_STATE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_DCCP_STATE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_DCCP_ROLE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_DCCP_ROLE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_ICMP_TYPE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_ICMP_TYPE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_ICMP_CODE] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_ICMP_CODE,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_ICMP_ID] = {
		.parse	= ct_parse_u16,
		.attr	= ATTR_ICMP_ID,
		.size	= NTA_SIZE(sizeof(uint16_t)),
	},
	[NTA_TCP_WSCALE_ORIG] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_TCP_WSCALE_ORIG,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_TCP_WSCALE_REPL] = {
		.parse	= ct_parse_u8,
		.attr	= ATTR_TCP_WSCALE_REPL,
		.size	= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_HELPER_NAME] = {
		.parse2	= ct_parse_str,
		.attr	= ATTR_HELPER_NAME,
		.max_size = NFCT_HELPER_NAME_MAX,
	},
	[NTA_LABELS] = {
		.parse2 = ct_parse_clabel,
		.attr	= ATTR_CONNLABELS,
		.max_size = NTA_SIZE(NTA_LABELS_MAX_SIZE),
	},
	[NTA_SNAT_IPV6]	= {
		.parse	= ct_parse_u128,
		.attr	= ATTR_SNAT_IPV6,
		.size	= NTA_SIZE(sizeof(uint32_t) * 4),
	},
	[NTA_DNAT_IPV6] = {
		.parse	= ct_parse_u128,
		.attr	= ATTR_DNAT_IPV6,
		.size	= NTA_SIZE(sizeof(uint32_t) * 4),
	},
	[NTA_SYNPROXY] = {
		.parse	= ct_parse_synproxy,
		.size	= NTA_SIZE(sizeof(struct nta_attr_synproxy)),
	},
};

static void
ct_parse_u8(struct nf_conntrack *ct, int attr, void *data)
{
	uint8_t *value = (uint8_t *) data;
	nfct_set_attr_u8(ct, h[attr].attr, *value);
}

static void
ct_parse_u16(struct nf_conntrack *ct, int attr, void *data)
{
	uint16_t *value = (uint16_t *) data;
	nfct_set_attr_u16(ct, h[attr].attr, ntohs(*value));
}

static void
ct_parse_u32(struct nf_conntrack *ct, int attr, void *data)
{
	uint32_t *value = (uint32_t *) data;
	nfct_set_attr_u32(ct, h[attr].attr, ntohl(*value));
}

static void
ct_parse_u128(struct nf_conntrack *ct, int attr, void *data)
{
	nfct_set_attr(ct, h[attr].attr, data);
}

static void
ct_parse_str(struct nf_conntrack *ct, const struct netattr *attr, void *data)
{
	nfct_set_attr(ct, h[attr->nta_attr].attr, data);
}

static void
ct_parse_group(struct nf_conntrack *ct, int attr, void *data)
{
	nfct_set_attr_grp(ct, h[attr].attr, data);
}

static void
ct_parse_clabel(struct nf_conntrack *ct, const struct netattr *attr, void *data)
{
	struct nfct_bitmask *bitm;
	unsigned int i, wordcount;
	const uint32_t *words;
	unsigned int len;

	len = attr->nta_len - NTA_LENGTH(0);
	wordcount =  len / sizeof(*words);
	if (!wordcount)
		return;

	if (len & (sizeof(*words) - 1))
		return;

	bitm = nfct_bitmask_new((len * 8) - 1);
	if (!bitm)
		return;

	words = data;
	for (i=0; i < wordcount; i++) {
		uint32_t word;
		int bit;

		if (words[i] == 0)
			continue;

		word = htonl(words[i]);
		bit = 31;
		do {
			if (word & (1 << bit))
				nfct_bitmask_set_bit(bitm, (32 * i) + bit);
		} while (--bit >= 0);
	}
	nfct_set_attr(ct, ATTR_CONNLABELS, bitm);
}

static void
ct_parse_nat_seq_adj(struct nf_conntrack *ct, int attr, void *data)
{
	struct nta_attr_natseqadj *this = data;
	nfct_set_attr_u32(ct, ATTR_ORIG_NAT_SEQ_CORRECTION_POS, 
			  ntohl(this->orig_seq_correction_pos));
	nfct_set_attr_u32(ct, ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE, 
			  ntohl(this->orig_seq_offset_before));
	nfct_set_attr_u32(ct, ATTR_ORIG_NAT_SEQ_OFFSET_AFTER, 
			  ntohl(this->orig_seq_offset_after));
	nfct_set_attr_u32(ct, ATTR_REPL_NAT_SEQ_CORRECTION_POS, 
			  ntohl(this->repl_seq_correction_pos));
	nfct_set_attr_u32(ct, ATTR_REPL_NAT_SEQ_OFFSET_BEFORE, 
			  ntohl(this->repl_seq_offset_before));
	nfct_set_attr_u32(ct, ATTR_REPL_NAT_SEQ_OFFSET_AFTER, 
			  ntohl(this->repl_seq_offset_after));
}

static void ct_parse_synproxy(struct nf_conntrack *ct, int attr, void *data)
{
	struct nta_attr_synproxy *this = data;

	nfct_set_attr_u32(ct, ATTR_SYNPROXY_ISN, ntohl(this->isn));
	nfct_set_attr_u32(ct, ATTR_SYNPROXY_ITS, ntohl(this->its));
	nfct_set_attr_u32(ct, ATTR_SYNPROXY_TSOFF, ntohl(this->tsoff));
}

int msg2ct(struct nf_conntrack *ct, struct nethdr *net, size_t remain)
{
	int len;
	struct netattr *attr;

	if (remain < net->len)
		return -1;

	len = net->len - NETHDR_SIZ;
	attr = NETHDR_DATA(net);

	while (len > ssizeof(struct netattr)) {
		ATTR_NETWORK2HOST(attr);
		if (attr->nta_len > len)
			return -1;
		if (attr->nta_len < NTA_LENGTH(0))
			return -1;
		if (attr->nta_attr >= NTA_MAX)
			return -1;
		if (h[attr->nta_attr].size &&
		    attr->nta_len != h[attr->nta_attr].size)
			return -1;

		if (h[attr->nta_attr].max_size) {
			if (attr->nta_len > h[attr->nta_attr].max_size)
				return -1;
			h[attr->nta_attr].parse2(ct, attr, NTA_DATA(attr));
			attr = NTA_NEXT(attr, len);
			continue;
		}

		if (h[attr->nta_attr].parse == NULL) {
			attr = NTA_NEXT(attr, len);
			continue;
		}
		h[attr->nta_attr].parse(ct, attr->nta_attr, NTA_DATA(attr));
		attr = NTA_NEXT(attr, len);
	}

	return 0;
}

static void exp_parse_ct_group(void *ct, int attr, void *data);
static void exp_parse_ct_u8(void *ct, int attr, void *data);
static void exp_parse_u32(void *exp, int attr, void *data);
static void exp_parse_str(void *exp, int attr, void *data);

static struct exp_parser {
	void 	(*parse)(void *obj, int attr, void *data);
	int 	exp_attr;
	int 	ct_attr;
	int	size;
	int	max_size;
} exp_h[NTA_EXP_MAX] = {
	[NTA_EXP_MASTER_IPV4] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASTER,
		.ct_attr	= ATTR_GRP_ORIG_IPV4,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_EXP_MASTER_IPV6] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASTER,
		.ct_attr	= ATTR_GRP_ORIG_IPV6,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv6)),
	},
	[NTA_EXP_MASTER_L4PROTO] = {
		.parse		= exp_parse_ct_u8,
		.exp_attr	= ATTR_EXP_MASTER,
		.ct_attr	= ATTR_L4PROTO,
		.size		= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_EXP_MASTER_PORT] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASTER,
		.ct_attr	= ATTR_GRP_ORIG_PORT,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_EXP_EXPECT_IPV4] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_EXPECTED,
		.ct_attr	= ATTR_GRP_ORIG_IPV4,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_EXP_EXPECT_IPV6] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_EXPECTED,
		.ct_attr	= ATTR_GRP_ORIG_IPV6,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv6)),
	},
	[NTA_EXP_EXPECT_L4PROTO] = {
		.parse		= exp_parse_ct_u8,
		.exp_attr	= ATTR_EXP_EXPECTED,
		.ct_attr	= ATTR_L4PROTO,
		.size		= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_EXP_EXPECT_PORT] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_EXPECTED,
		.ct_attr	= ATTR_GRP_ORIG_PORT,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_EXP_MASK_IPV4] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASK,
		.ct_attr	= ATTR_GRP_ORIG_IPV4,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_EXP_MASK_IPV6] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASK,
		.ct_attr	= ATTR_GRP_ORIG_IPV6,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv6)),
	},
	[NTA_EXP_MASK_L4PROTO] = {
		.parse		= exp_parse_ct_u8,
		.exp_attr	= ATTR_EXP_MASK,
		.ct_attr	= ATTR_L4PROTO,
		.size		= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_EXP_MASK_PORT] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_MASK,
		.ct_attr	= ATTR_GRP_ORIG_PORT,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_EXP_TIMEOUT] = {
		.parse		= exp_parse_u32,
		.exp_attr	= ATTR_EXP_TIMEOUT,
		.size		= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_EXP_FLAGS] = {
		.parse		= exp_parse_u32,
		.exp_attr	= ATTR_EXP_FLAGS,
		.size		= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_EXP_CLASS] = {
		.parse		= exp_parse_u32,
		.exp_attr	= ATTR_EXP_CLASS,
		.size		= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_EXP_NAT_IPV4] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_NAT_TUPLE,
		.ct_attr	= ATTR_GRP_ORIG_IPV4,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_ipv4)),
	},
	[NTA_EXP_NAT_L4PROTO] = {
		.parse		= exp_parse_ct_u8,
		.exp_attr	= ATTR_EXP_NAT_TUPLE,
		.ct_attr	= ATTR_L4PROTO,
		.size		= NTA_SIZE(sizeof(uint8_t)),
	},
	[NTA_EXP_NAT_PORT] = {
		.parse		= exp_parse_ct_group,
		.exp_attr	= ATTR_EXP_NAT_TUPLE,
		.ct_attr	= ATTR_GRP_ORIG_PORT,
		.size		= NTA_SIZE(sizeof(struct nfct_attr_grp_port)),
	},
	[NTA_EXP_NAT_DIR] = {
		.parse		= exp_parse_u32,
		.exp_attr	= ATTR_EXP_NAT_DIR,
		.size		= NTA_SIZE(sizeof(uint32_t)),
	},
	[NTA_EXP_HELPER_NAME] = {
		.parse		= exp_parse_str,
		.exp_attr	= ATTR_EXP_HELPER_NAME,
		.max_size	= NFCT_HELPER_NAME_MAX,
	},
	[NTA_EXP_FN] = {
		.parse		= exp_parse_str,
		.exp_attr	= ATTR_EXP_FN,
		.max_size	= 32,	/* XXX: artificial limit */
	},
};

static void exp_parse_ct_group(void *ct, int attr, void *data)
{
	nfct_set_attr_grp(ct, exp_h[attr].ct_attr, data);
}

static void exp_parse_ct_u8(void *ct, int attr, void *data)
{
	uint8_t *value = (uint8_t *) data;
	nfct_set_attr_u8(ct, exp_h[attr].ct_attr, *value);
}

static void exp_parse_u32(void *exp, int attr, void *data)
{
	uint32_t *value = (uint32_t *) data;
	nfexp_set_attr_u32(exp, exp_h[attr].exp_attr, ntohl(*value));
}

static void exp_parse_str(void *exp, int attr, void *data)
{
	nfexp_set_attr(exp, exp_h[attr].exp_attr, data);
}

int msg2exp(struct nf_expect *exp, struct nethdr *net, size_t remain)
{
	int len;
	struct netattr *attr;
	struct nf_conntrack *master, *expected, *mask, *nat;

	if (remain < net->len)
		return -1;

	len = net->len - NETHDR_SIZ;
	attr = NETHDR_DATA(net);

	master = nfct_new();
	if (master == NULL)
		goto err_master;

	expected = nfct_new();
	if (expected == NULL)
		goto err_expected;

	mask = nfct_new();
	if (mask == NULL)
		goto err_mask;

	nat = nfct_new();
	if (nat == NULL)
		goto err_nat;

	while (len > ssizeof(struct netattr)) {
		ATTR_NETWORK2HOST(attr);
		if (attr->nta_len > len)
			goto err;
		if (attr->nta_attr >= NTA_EXP_MAX)
			goto err;
		if (attr->nta_len < NTA_LENGTH(0))
			goto err;
		if (exp_h[attr->nta_attr].size &&
		    attr->nta_len != exp_h[attr->nta_attr].size)
			goto err;
		if (exp_h[attr->nta_attr].max_size &&
		    attr->nta_len > exp_h[attr->nta_attr].max_size)
			goto err;
		if (exp_h[attr->nta_attr].parse == NULL) {
			attr = NTA_NEXT(attr, len);
			continue;
		}
		switch (exp_h[attr->nta_attr].exp_attr) {
		case ATTR_EXP_MASTER:
			exp_h[attr->nta_attr].parse(master, attr->nta_attr,
						    NTA_DATA(attr));
			break;
		case ATTR_EXP_EXPECTED:
			exp_h[attr->nta_attr].parse(expected, attr->nta_attr,
						    NTA_DATA(attr));
			break;
		case ATTR_EXP_MASK:
			exp_h[attr->nta_attr].parse(mask, attr->nta_attr,
						    NTA_DATA(attr));
			break;
		case ATTR_EXP_NAT_TUPLE:
			exp_h[attr->nta_attr].parse(nat, attr->nta_attr,
						    NTA_DATA(attr));
			nfexp_set_attr(exp, ATTR_EXP_NAT_TUPLE, nat);
			break;
		case ATTR_EXP_TIMEOUT:
		case ATTR_EXP_FLAGS:
		case ATTR_EXP_CLASS:
		case ATTR_EXP_HELPER_NAME:
		case ATTR_EXP_NAT_DIR:
		case ATTR_EXP_FN:
			exp_h[attr->nta_attr].parse(exp, attr->nta_attr,
						    NTA_DATA(attr));
			break;
		}
		attr = NTA_NEXT(attr, len);
	}

	nfexp_set_attr(exp, ATTR_EXP_MASTER, master);
	nfexp_set_attr(exp, ATTR_EXP_EXPECTED, expected);
	nfexp_set_attr(exp, ATTR_EXP_MASK, mask);

	/* We can release the conntrack objects at this point because the
	 * setter makes a copy of them. This is not efficient, it would be
	 * better to save that extra copy but this is how the library works.
	 * I'm sorry, I cannot change it without breaking backward
	 * compatibility. Probably it is a good idea to think of adding new
	 * interfaces in the near future to get it better. */
	nfct_destroy(mask);
	nfct_destroy(expected);
	nfct_destroy(master);
	nfct_destroy(nat);

	return 0;
err:
	nfct_destroy(nat);
err_nat:
	nfct_destroy(mask);
err_mask:
	nfct_destroy(expected);
err_expected:
	nfct_destroy(master);
err_master:
	return -1;
}
