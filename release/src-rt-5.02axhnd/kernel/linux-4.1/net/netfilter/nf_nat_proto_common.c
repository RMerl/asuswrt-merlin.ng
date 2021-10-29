/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/random.h>
#include <linux/netfilter.h>
#include <linux/export.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_l3proto.h>
#include <net/netfilter/nf_nat_l4proto.h>

bool nf_nat_l4proto_in_range(const struct nf_conntrack_tuple *tuple,
			     enum nf_nat_manip_type maniptype,
			     const struct nf_nat_range *range)
{
	const union nf_conntrack_man_proto *min = &range->min_proto;
	const union nf_conntrack_man_proto *max = &range->max_proto;
	__be16 port;

	if (maniptype == NF_NAT_MANIP_SRC)
		port = tuple->src.u.all;
	else
		port = tuple->dst.u.all;

	if (range->flags & NF_NAT_RANGE_PROTO_PSID) {
		unsigned int a = range->min_proto.psid.offset;
		unsigned int k = range->min_proto.psid.length;
		unsigned int m = 16 - a - k;
		u_int16_t psid = range->max_proto.psid.id;
		u_int16_t id = ntohs(port);

		return (a == 0 || (id >> (16 - a))) &&
		       !(((id >> m) ^ psid) & ~(~0U << k));
	}

	return ntohs(port) >= ntohs(min->all) &&
	       ntohs(port) <= ntohs(max->all);
}
EXPORT_SYMBOL_GPL(nf_nat_l4proto_in_range);

void nf_nat_l4proto_unique_tuple(const struct nf_nat_l3proto *l3proto,
				 struct nf_conntrack_tuple *tuple,
				 const struct nf_nat_range *range,
				 enum nf_nat_manip_type maniptype,
				 const struct nf_conn *ct,
				 u16 *rover)
{
	unsigned int range_size, min, max, i;
	__be16 *portptr;
	u_int16_t off;

	if (maniptype == NF_NAT_MANIP_SRC)
		portptr = &tuple->src.u.all;
	else
		portptr = &tuple->dst.u.all;

	if (range->flags & NF_NAT_RANGE_PROTO_PSID) {
		unsigned int a = range->min_proto.psid.offset;
		unsigned int k = range->min_proto.psid.length;
		unsigned int m = 16 - a - k;
		u_int16_t psid = range->max_proto.psid.id << m;

		range_size = (1 << (16 - k)) - (!!a << m);
		if (range_size == 0)
			return;

		if (range->flags & NF_NAT_RANGE_PROTO_RANDOM) {
			off = l3proto->secure_port(tuple, maniptype == NF_NAT_MANIP_SRC
							  ? tuple->dst.u.all
							  : tuple->src.u.all);
		} else if (range->flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY) {
			off = prandom_u32();
		} else {
			off = *rover;
		}

		for (i = 0; ; ++off) {
			unsigned int n = off % range_size;
			*portptr = htons((((n >> m) + !!a) << (16 - a)) |
					 psid | (n & ~(~0U << m)));
			if (++i != range_size && nf_nat_used_tuple(tuple, ct))
				continue;
			if (!(range->flags & NF_NAT_RANGE_PROTO_RANDOM_ALL))
				*rover = off;
			return;
		}
		return;
	}

	/* If no range specified... */
	if (!(range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)) {
		/* If it's dst rewrite, can't change port */
		if (maniptype == NF_NAT_MANIP_DST)
			return;

		if (ntohs(*portptr) < 1024) {
			/* Loose convention: >> 512 is credential passing */
			if (ntohs(*portptr) < 512) {
				min = 1;
				range_size = 511 - min + 1;
			} else {
				min = 600;
				range_size = 1023 - min + 1;
			}
		} else {
			min = 1024;
			range_size = 65535 - 1024 + 1;
		}
	} else {
		min = ntohs(range->min_proto.all);
		max = ntohs(range->max_proto.all);
		if (unlikely(max < min))
			swap(max, min);
		range_size = max - min + 1;
	}

	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM) {
		off = l3proto->secure_port(tuple, maniptype == NF_NAT_MANIP_SRC
						  ? tuple->dst.u.all
						  : tuple->src.u.all);
	} else if (range->flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY) {
		off = prandom_u32();
	} else {
		off = *rover;
	}

	for (i = 0; ; ++off) {
		*portptr = htons(min + off % range_size);
		if (++i != range_size && nf_nat_used_tuple(tuple, ct))
			continue;
		if (!(range->flags & NF_NAT_RANGE_PROTO_RANDOM_ALL))
			*rover = off;
		return;
	}
}
EXPORT_SYMBOL_GPL(nf_nat_l4proto_unique_tuple);

#if IS_ENABLED(CONFIG_NF_CT_NETLINK)
int nf_nat_l4proto_nlattr_to_range(struct nlattr *tb[],
				   struct nf_nat_range *range)
{
	if (tb[CTA_PROTONAT_PORT_MIN]) {
		range->min_proto.all = nla_get_be16(tb[CTA_PROTONAT_PORT_MIN]);
		range->max_proto.all = range->min_proto.all;
		range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;
	}
	if (tb[CTA_PROTONAT_PORT_MAX]) {
		range->max_proto.all = nla_get_be16(tb[CTA_PROTONAT_PORT_MAX]);
		range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(nf_nat_l4proto_nlattr_to_range);
#endif
