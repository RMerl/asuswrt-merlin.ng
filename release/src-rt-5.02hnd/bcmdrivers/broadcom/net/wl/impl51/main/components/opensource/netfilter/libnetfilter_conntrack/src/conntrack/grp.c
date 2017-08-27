/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

const struct attr_grp_bitmask attr_grp_bitmask[ATTR_GRP_MAX]= {
	[ATTR_GRP_ORIG_IPV4] = {
		.bitmask[0] = (1 << ATTR_ORIG_IPV4_SRC) |
			      (1 << ATTR_ORIG_IPV4_DST) |
			      (1 << ATTR_ORIG_L3PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_REPL_IPV4] = {
		.bitmask[0] = (1 << ATTR_REPL_IPV4_SRC) |
			      (1 << ATTR_REPL_IPV4_DST) |
			      (1 << ATTR_REPL_L3PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_ORIG_IPV6] = {
		.bitmask[0] = (1 << ATTR_ORIG_IPV6_SRC) |
			      (1 << ATTR_ORIG_IPV6_DST) |
			      (1 << ATTR_ORIG_L3PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_REPL_IPV6] = {
		.bitmask[0] = (1 << ATTR_REPL_IPV6_SRC) |
			      (1 << ATTR_REPL_IPV6_DST) |
			      (1 << ATTR_REPL_L3PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_ORIG_PORT] = {
		.bitmask[0] = (1 << ATTR_ORIG_PORT_SRC) |
			      (1 << ATTR_ORIG_PORT_DST) |
			      (1 << ATTR_ORIG_L4PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_REPL_PORT] = {
		.bitmask[0] = (1 << ATTR_REPL_PORT_SRC) |
			      (1 << ATTR_REPL_PORT_DST) |
			      (1 << ATTR_REPL_L4PROTO),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_ICMP] = {
		.bitmask[0] = (1 << ATTR_ICMP_CODE) |
			      (1 << ATTR_ICMP_TYPE) |
			      (1 << ATTR_ICMP_ID),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_MASTER_IPV4] = {
		.bitmask[1] = (1 << (ATTR_MASTER_IPV4_SRC - 32)) |
			      (1 << (ATTR_MASTER_IPV4_DST - 32)) |
			      (1 << (ATTR_MASTER_L3PROTO - 32)),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_MASTER_IPV6] = {
		.bitmask[1] = (1 << (ATTR_MASTER_IPV6_SRC - 32)) |
			      (1 << (ATTR_MASTER_IPV6_DST - 32)) |
			      (1 << (ATTR_MASTER_L3PROTO - 32)),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_MASTER_PORT] = {
		.bitmask[1] = (1 << (ATTR_MASTER_PORT_SRC - 32)) |
			      (1 << (ATTR_MASTER_PORT_DST - 32)) |
			      (1 << (ATTR_MASTER_L4PROTO - 32)),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_ORIG_COUNTERS] = {
		.bitmask[0] = (1 << (ATTR_ORIG_COUNTER_PACKETS)) |
			      (1 << (ATTR_ORIG_COUNTER_BYTES)),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_REPL_COUNTERS] = {
		.bitmask[0] = (1 << (ATTR_REPL_COUNTER_PACKETS)) |
			      (1 << (ATTR_REPL_COUNTER_BYTES)),
		.type = NFCT_BITMASK_AND,
	},
	[ATTR_GRP_ORIG_ADDR_SRC] = {
		.bitmask[0] = (1 << ATTR_ORIG_IPV4_SRC) |
			      (1 << ATTR_ORIG_IPV6_SRC),
		.type = NFCT_BITMASK_OR,
	},
	[ATTR_GRP_ORIG_ADDR_DST] = {
		.bitmask[0] = (1 << ATTR_ORIG_IPV4_DST) |
			      (1 << ATTR_ORIG_IPV6_DST),
		.type = NFCT_BITMASK_OR,

	},
	[ATTR_GRP_REPL_ADDR_SRC] = {
		.bitmask[0] = (1 << ATTR_REPL_IPV4_SRC) |
			      (1 << ATTR_REPL_IPV6_SRC),
		.type = NFCT_BITMASK_OR,
	},
	[ATTR_GRP_REPL_ADDR_DST] = {
		.bitmask[0] = (1 << ATTR_REPL_IPV4_DST) |
			      (1 << ATTR_REPL_IPV6_DST),
		.type = NFCT_BITMASK_OR,
	},
};
