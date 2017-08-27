/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void __autocomplete(struct nf_conntrack *ct, int dir)
{
	struct __nfct_tuple *this = NULL, *other = NULL;

	switch(dir) {
	case __DIR_ORIG:
		this = &ct->head.orig;
		other = &ct->repl;
		break;
	case __DIR_REPL:
		this = &ct->repl;
		other = &ct->head.orig;
		break;
	}

	this->l3protonum = other->l3protonum;
	this->protonum = other->protonum;

	memcpy(&this->src.v6, &other->dst.v6, sizeof(union __nfct_address));
	memcpy(&this->dst.v6, &other->src.v6, sizeof(union __nfct_address));

	switch(this->protonum) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_SCTP:
	case IPPROTO_DCCP:
	case IPPROTO_GRE:
	case IPPROTO_UDPLITE:
		this->l4src.all = other->l4dst.all;
		this->l4dst.all = other->l4src.all;
		break;
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		/* the setter already autocompletes the reply tuple. */
		break;
	}

	/* XXX: this is safe but better convert bitset to uint64_t */
        ct->head.set[0] |= TS_ORIG | TS_REPL;
}

static void setobjopt_undo_snat(struct nf_conntrack *ct)
{
	ct->snat.min_ip = ct->repl.dst.v4;
	ct->snat.max_ip = ct->snat.min_ip;
	ct->repl.dst.v4 = ct->head.orig.src.v4;
	set_bit(ATTR_SNAT_IPV4, ct->head.set);
}

static void setobjopt_undo_dnat(struct nf_conntrack *ct)
{
	ct->dnat.min_ip = ct->repl.src.v4;
	ct->dnat.max_ip = ct->dnat.min_ip;
	ct->repl.src.v4 = ct->head.orig.dst.v4;
	set_bit(ATTR_DNAT_IPV4, ct->head.set);
}

static void setobjopt_undo_spat(struct nf_conntrack *ct)
{
	ct->snat.l4min.all = ct->repl.l4dst.tcp.port;
	ct->snat.l4max.all = ct->snat.l4min.all;
	ct->repl.l4dst.tcp.port =
			ct->head.orig.l4src.tcp.port;
	set_bit(ATTR_SNAT_PORT, ct->head.set);
}

static void setobjopt_undo_dpat(struct nf_conntrack *ct)
{
	ct->dnat.l4min.all = ct->repl.l4src.tcp.port;
	ct->dnat.l4max.all = ct->dnat.l4min.all;
	ct->repl.l4src.tcp.port =
			ct->head.orig.l4dst.tcp.port;
	set_bit(ATTR_DNAT_PORT, ct->head.set);
}

static void setobjopt_setup_orig(struct nf_conntrack *ct)
{
	__autocomplete(ct, __DIR_ORIG);
}

static void setobjopt_setup_repl(struct nf_conntrack *ct)
{
	__autocomplete(ct, __DIR_REPL);
}

static const setobjopt setobjopt_array[__NFCT_SOPT_MAX] = {
	[NFCT_SOPT_UNDO_SNAT] 		= setobjopt_undo_snat,
	[NFCT_SOPT_UNDO_DNAT] 		= setobjopt_undo_dnat,
	[NFCT_SOPT_UNDO_SPAT] 		= setobjopt_undo_spat,
	[NFCT_SOPT_UNDO_DPAT] 		= setobjopt_undo_dpat,
	[NFCT_SOPT_SETUP_ORIGINAL] 	= setobjopt_setup_orig,
	[NFCT_SOPT_SETUP_REPLY]		= setobjopt_setup_repl,
};

int __setobjopt(struct nf_conntrack *ct, unsigned int option)
{
	if (unlikely(option > NFCT_SOPT_MAX))
		return -1;

	setobjopt_array[option](ct);
	return 0;
}

static int getobjopt_is_snat(const struct nf_conntrack *ct)
{
	return ((test_bit(ATTR_STATUS, ct->head.set) ?
		ct->status & IPS_SRC_NAT_DONE : 1) &&
		ct->repl.dst.v4 != 
		ct->head.orig.src.v4);
}

static int getobjopt_is_dnat(const struct nf_conntrack *ct)
{
	return ((test_bit(ATTR_STATUS, ct->head.set) ?
		ct->status & IPS_DST_NAT_DONE : 1) &&
		ct->repl.src.v4 !=
		ct->head.orig.dst.v4);
}

static int getobjopt_is_spat(const struct nf_conntrack *ct)
{
	return ((test_bit(ATTR_STATUS, ct->head.set) ?
		ct->status & IPS_SRC_NAT_DONE : 1) &&
		ct->repl.l4dst.tcp.port !=
		ct->head.orig.l4src.tcp.port);
}

static int getobjopt_is_dpat(const struct nf_conntrack *ct)
{
	return ((test_bit(ATTR_STATUS, ct->head.set) ?
		ct->status & IPS_DST_NAT_DONE : 1) &&
		ct->repl.l4src.tcp.port !=
		ct->head.orig.l4dst.tcp.port);
}

static const getobjopt getobjopt_array[__NFCT_GOPT_MAX] = {
	[NFCT_GOPT_IS_SNAT] = getobjopt_is_snat,
	[NFCT_GOPT_IS_DNAT] = getobjopt_is_dnat,
	[NFCT_GOPT_IS_SPAT] = getobjopt_is_spat,
	[NFCT_GOPT_IS_DPAT] = getobjopt_is_dpat,
};

int __getobjopt(const struct nf_conntrack *ct, unsigned int option)
{
	if (unlikely(option > NFCT_GOPT_MAX))
		return -1;

	return getobjopt_array[option](ct);
}
