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
static struct ipset_type ipset_bitmap_ipmac0 = {
	.name = "bitmap:ip,mac",
	.alias = { "macipmap", NULL },
	.revision = 0,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_TWO,
	.last_elem_optional = true,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_single_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_ether,
			.print = ipset_print_ether,
			.opt = IPSET_OPT_ETHER
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
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
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.\n"
		 "      MAC is a valid MAC address.",
	.description = "Initial revision",
};

/* Counters support */
static struct ipset_type ipset_bitmap_ipmac1 = {
	.name = "bitmap:ip,mac",
	.alias = { "macipmap", NULL },
	.revision = 1,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_TWO,
	.last_elem_optional = true,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_single_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_ether,
			.print = ipset_print_ether,
			.opt = IPSET_OPT_ETHER
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
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
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.\n"
		 "      MAC is a valid MAC address.",
	.description = "counters support",
};

/* Comment support */
static struct ipset_type ipset_bitmap_ipmac2 = {
	.name = "bitmap:ip,mac",
	.alias = { "macipmap", NULL },
	.revision = 2,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_TWO,
	.last_elem_optional = true,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_single_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_ether,
			.print = ipset_print_ether,
			.opt = IPSET_OPT_ETHER
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
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
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.\n"
		 "      MAC is a valid MAC address.",
	.description = "comment support",
};

/* skbinfo support */
static struct ipset_type ipset_bitmap_ipmac3 = {
	.name = "bitmap:ip,mac",
	.alias = { "macipmap", NULL },
	.revision = 3,
	.family = NFPROTO_IPV4,
	.dimension = IPSET_DIM_TWO,
	.last_elem_optional = true,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_single_ip,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_ether,
			.print = ipset_print_ether,
			.opt = IPSET_OPT_ETHER
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_IPRANGE,
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
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_ETHER),
			.help = "IP[,MAC]",
		},
	},
	.usage = "where IP, FROM and TO are IPv4 addresses (or hostnames),\n"
		 "      CIDR is a valid IPv4 CIDR prefix.\n"
		 "      MAC is a valid MAC address.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_bitmap_ipmac0);
	ipset_type_add(&ipset_bitmap_ipmac1);
	ipset_type_add(&ipset_bitmap_ipmac2);
	ipset_type_add(&ipset_bitmap_ipmac3);
}
