/* Copyright 2007-2013 Jozsef Kadlecsik (kadlec@netfilter.org)
 * Copyright 2013 Smoothwall Ltd. (vytas.dauksa@smoothwall.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <libipset/data.h>			/* IPSET_OPT_* */
#include <libipset/parse.h>			/* parser functions */
#include <libipset/print.h>			/* printing functions */
#include <libipset/types.h>			/* prototypes */

/* Initial release */
static struct ipset_type ipset_hash_ipmark0 = {
	.name = "hash:ip,mark",
	.alias = { "ipmarkhash", NULL },
	.revision = 0,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_single6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_mark,
			.print = ipset_print_mark,
			.opt = IPSET_OPT_MARK
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_MARKMASK,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_IGNORED_FROM,
				IPSET_ARG_IGNORED_TO,
				IPSET_ARG_IGNORED_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname).\n"
		 "      Adding/deleting multiple elements in IP/CIDR or FROM-TO form\n"
		 "      is supported for IPv4.\n"
		 "      Adding/deleting single mark element\n"
		 "      is supported both for IPv4 and IPv6.",
	.description = "initial revision",
};

/* Forceadd support */
static struct ipset_type ipset_hash_ipmark1 = {
	.name = "hash:ip,mark",
	.alias = { "ipmarkhash", NULL },
	.revision = 1,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_single6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_mark,
			.print = ipset_print_mark,
			.opt = IPSET_OPT_MARK
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_MARKMASK,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_FORCEADD,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_IGNORED_FROM,
				IPSET_ARG_IGNORED_TO,
				IPSET_ARG_IGNORED_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname).\n"
		 "      Adding/deleting multiple elements in IP/CIDR or FROM-TO form\n"
		 "      is supported for IPv4.\n"
		 "      Adding/deleting single mark element\n"
		 "      is supported both for IPv4 and IPv6.",
	.description = "forceadd support",
};

/* skbinfo support */
static struct ipset_type ipset_hash_ipmark2 = {
	.name = "hash:ip,mark",
	.alias = { "ipmarkhash", NULL },
	.revision = 2,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_single6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_mark,
			.print = ipset_print_mark,
			.opt = IPSET_OPT_MARK
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_MARKMASK,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_FORCEADD,
				IPSET_ARG_SKBINFO,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_IGNORED_FROM,
				IPSET_ARG_IGNORED_TO,
				IPSET_ARG_IGNORED_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_SKBMARK,
				IPSET_ARG_SKBPRIO,
				IPSET_ARG_SKBQUEUE,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_MARK),
			.help = "IP,MARK",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname).\n"
		 "      Adding/deleting multiple elements in IP/CIDR or FROM-TO form\n"
		 "      is supported for IPv4.\n"
		 "      Adding/deleting single mark element\n"
		 "      is supported both for IPv4 and IPv6.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_hash_ipmark0);
	ipset_type_add(&ipset_hash_ipmark1);
	ipset_type_add(&ipset_hash_ipmark2);
}
