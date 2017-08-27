/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void filter_attr_l4proto(struct nfct_filter *filter, const void *value)
{
	if (filter->l4proto_len >= __FILTER_L4PROTO_MAX)
		return;

	set_bit(*((int *) value), filter->l4proto_map);
	filter->l4proto_len++;
}

static void 
filter_attr_l4proto_state(struct nfct_filter *filter, const void *value)
{
	const struct nfct_filter_proto *this = value;

	set_bit_u16(this->state, &filter->l4proto_state[this->proto].map);
	filter->l4proto_state[this->proto].len++;
}

static void filter_attr_src_ipv4(struct nfct_filter *filter, const void *value)
{
	const struct nfct_filter_ipv4 *this = value;

	if (filter->l3proto_elems[0] >= __FILTER_ADDR_MAX)
		return;

	filter->l3proto[0][filter->l3proto_elems[0]].addr = this->addr;
	filter->l3proto[0][filter->l3proto_elems[0]].mask = this->mask;
	filter->l3proto_elems[0]++;
}

static void filter_attr_dst_ipv4(struct nfct_filter *filter, const void *value)
{
	const struct nfct_filter_ipv4 *this = value;

	if (filter->l3proto_elems[1] >= __FILTER_ADDR_MAX)
		return;

	filter->l3proto[1][filter->l3proto_elems[1]].addr = this->addr;
	filter->l3proto[1][filter->l3proto_elems[1]].mask = this->mask;
	filter->l3proto_elems[1]++;
}

static void filter_attr_src_ipv6(struct nfct_filter *filter, const void *value)
{
	const struct nfct_filter_ipv6 *this = value;

	if (filter->l3proto_elems_ipv6[0] >= __FILTER_IPV6_MAX)
		return;

	memcpy(filter->l3proto_ipv6[0][filter->l3proto_elems_ipv6[0]].addr,
	       this->addr, sizeof(u_int32_t)*4);
	memcpy(filter->l3proto_ipv6[0][filter->l3proto_elems_ipv6[0]].mask,
	       this->mask, sizeof(u_int32_t)*4);
	filter->l3proto_elems_ipv6[0]++;
}

static void filter_attr_dst_ipv6(struct nfct_filter *filter, const void *value)
{
	const struct nfct_filter_ipv6 *this = value;

	if (filter->l3proto_elems_ipv6[1] >= __FILTER_IPV6_MAX)
		return;

	memcpy(filter->l3proto_ipv6[1][filter->l3proto_elems_ipv6[1]].addr,
	       this->addr, sizeof(u_int32_t)*4);
	memcpy(filter->l3proto_ipv6[1][filter->l3proto_elems_ipv6[1]].mask,
	       this->mask, sizeof(u_int32_t)*4);
	filter->l3proto_elems_ipv6[1]++;
}

const filter_attr filter_attr_array[NFCT_FILTER_MAX] = {
	[NFCT_FILTER_L4PROTO]		= filter_attr_l4proto,
	[NFCT_FILTER_L4PROTO_STATE]	= filter_attr_l4proto_state,
	[NFCT_FILTER_SRC_IPV4]		= filter_attr_src_ipv4,
	[NFCT_FILTER_DST_IPV4]		= filter_attr_dst_ipv4,
	[NFCT_FILTER_SRC_IPV6]		= filter_attr_src_ipv6,
	[NFCT_FILTER_DST_IPV6]		= filter_attr_dst_ipv6,
};
