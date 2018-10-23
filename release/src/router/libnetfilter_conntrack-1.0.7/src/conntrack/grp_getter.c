/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void get_attr_grp_orig_ipv4(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv4 *this = data;
	this->src = ct->head.orig.src.v4;
	this->dst = ct->head.orig.dst.v4;
}

static void get_attr_grp_repl_ipv4(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv4 *this = data;
	this->src = ct->repl.src.v4;
	this->dst = ct->repl.dst.v4;
}

static void get_attr_grp_orig_ipv6(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv6 *this = data;
	memcpy(this->src, &ct->head.orig.src.v6, sizeof(uint32_t)*4);
	memcpy(this->dst, &ct->head.orig.dst.v6, sizeof(uint32_t)*4);
}

static void get_attr_grp_repl_ipv6(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv6 *this = data;
	memcpy(this->src, &ct->repl.src.v6, sizeof(uint32_t)*4);
	memcpy(this->dst, &ct->repl.dst.v6, sizeof(uint32_t)*4);
}

static void get_attr_grp_orig_port(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_port *this = data;
	this->sport = ct->head.orig.l4src.all;
	this->dport = ct->head.orig.l4dst.all;
}

static void get_attr_grp_repl_port(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_port *this = data;
	this->sport = ct->repl.l4src.all;
	this->dport = ct->repl.l4dst.all;
}

static void get_attr_grp_icmp(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_icmp *this = data;
	this->type = ct->head.orig.l4dst.icmp.type;
	this->code = ct->head.orig.l4dst.icmp.code;
	this->id = ct->head.orig.l4src.icmp.id;
}

static void get_attr_grp_master_ipv4(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv4 *this = data;
	this->src = ct->master.src.v4;
	this->dst = ct->master.dst.v4;
}

static void get_attr_grp_master_ipv6(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ipv6 *this = data;
	memcpy(this->src, &ct->master.src.v6, sizeof(uint32_t)*4);
	memcpy(this->dst, &ct->master.dst.v6, sizeof(uint32_t)*4);
}

static void get_attr_grp_master_port(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_port *this = data;
	this->sport = ct->master.l4src.all;
	this->dport = ct->master.l4dst.all;
}

static void get_attr_grp_orig_ctrs(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ctrs *this = data;
	this->packets = ct->counters[__DIR_ORIG].packets;
	this->bytes = ct->counters[__DIR_ORIG].bytes;
}

static void get_attr_grp_repl_ctrs(const struct nf_conntrack *ct, void *data)
{
	struct nfct_attr_grp_ctrs *this = data;
	this->packets = ct->counters[__DIR_REPL].packets;
	this->bytes = ct->counters[__DIR_REPL].bytes;
}

static void
get_attr_grp_orig_addr_src(const struct nf_conntrack *ct, void *data)
{
	union nfct_attr_grp_addr *this = data;
	memcpy(&this->addr, &ct->head.orig.src, sizeof(ct->head.orig.src));
}

static void
get_attr_grp_orig_addr_dst(const struct nf_conntrack *ct, void *data)
{
	union nfct_attr_grp_addr *this = data;
	memcpy(&this->addr, &ct->head.orig.dst, sizeof(ct->head.orig.dst));
}

static void
get_attr_grp_repl_addr_src(const struct nf_conntrack *ct, void *data)
{
	union nfct_attr_grp_addr *this = data;
	memcpy(&this->addr, &ct->repl.src, sizeof(ct->repl.src));
}

static void
get_attr_grp_repl_addr_dst(const struct nf_conntrack *ct, void *data)
{
	union nfct_attr_grp_addr *this = data;
	memcpy(&this->addr, &ct->repl.dst, sizeof(ct->repl.dst));
}

const get_attr_grp get_attr_grp_array[ATTR_GRP_MAX] = {
	[ATTR_GRP_ORIG_IPV4]		= get_attr_grp_orig_ipv4,
	[ATTR_GRP_REPL_IPV4]		= get_attr_grp_repl_ipv4,
	[ATTR_GRP_ORIG_IPV6]		= get_attr_grp_orig_ipv6,
	[ATTR_GRP_REPL_IPV6]		= get_attr_grp_repl_ipv6,
	[ATTR_GRP_ORIG_PORT]		= get_attr_grp_orig_port,
	[ATTR_GRP_REPL_PORT]		= get_attr_grp_repl_port,
	[ATTR_GRP_ICMP]			= get_attr_grp_icmp,
	[ATTR_GRP_MASTER_IPV4]		= get_attr_grp_master_ipv4,
	[ATTR_GRP_MASTER_IPV6]		= get_attr_grp_master_ipv6,
	[ATTR_GRP_MASTER_PORT]		= get_attr_grp_master_port,
	[ATTR_GRP_ORIG_COUNTERS]	= get_attr_grp_orig_ctrs,
	[ATTR_GRP_REPL_COUNTERS]	= get_attr_grp_repl_ctrs,
	[ATTR_GRP_ORIG_ADDR_SRC]	= get_attr_grp_orig_addr_src,
	[ATTR_GRP_ORIG_ADDR_DST]	= get_attr_grp_orig_addr_dst,
	[ATTR_GRP_REPL_ADDR_SRC]	= get_attr_grp_repl_addr_src,
	[ATTR_GRP_REPL_ADDR_DST]	= get_attr_grp_repl_addr_dst,
};
