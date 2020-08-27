/* Copyright 2007-2010 Jozsef Kadlecsik (kadlec@netfilter.org)
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
static struct ipset_type ipset_bitmap_ip0 = {
	.name = "bitmap:ip",
	.alias = { "ipmap", NULL },
	.revision = 0,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
				IPSET_ARG_NETMASK,
				IPSET_ARG_TIMEOUT,
				/* Backward compatibility */
				IPSET_ARG_FROM_IP,
				IPSET_ARG_TO_IP,
				IPSET_ARG_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "range IP/CIDR|FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP),
			.help = "IP",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.",
	.description = "Initial revision",
};

/* Counters support */
static struct ipset_type ipset_bitmap_ip1 = {
	.name = "bitmap:ip",
	.alias = { "ipmap", NULL },
	.revision = 1,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
				IPSET_ARG_NETMASK,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				/* Backward compatibility */
				IPSET_ARG_FROM_IP,
				IPSET_ARG_TO_IP,
				IPSET_ARG_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "range IP/CIDR|FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP),
			.help = "IP",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.",
	.description = "counters support",
};

/* Comment support */
static struct ipset_type ipset_bitmap_ip2 = {
	.name = "bitmap:ip",
	.alias = { "ipmap", NULL },
	.revision = 2,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
				IPSET_ARG_NETMASK,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				/* Backward compatibility */
				IPSET_ARG_FROM_IP,
				IPSET_ARG_TO_IP,
				IPSET_ARG_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "range IP/CIDR|FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP),
			.help = "IP",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.",
	.description = "comment support",
};

/* skbinfo support */
static struct ipset_type ipset_bitmap_ip3 = {
	.name = "bitmap:ip",
	.alias = { "ipmap", NULL },
	.revision = 3,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
				IPSET_ARG_NETMASK,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_SKBINFO,
				/* Backward compatibility */
				IPSET_ARG_FROM_IP,
				IPSET_ARG_TO_IP,
				IPSET_ARG_NETWORK,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "range IP/CIDR|FROM-TO",
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
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP|IP/CIDR|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP),
			.help = "IP",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_bitmap_ip0);
	ipset_type_add(&ipset_bitmap_ip1);
	ipset_type_add(&ipset_bitmap_ip2);
	ipset_type_add(&ipset_bitmap_ip3);
}
