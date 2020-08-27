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
static struct ipset_type ipset_bitmap_port0 = {
	.name = "bitmap:port",
	.alias = { "portmap", NULL },
	.revision = 0,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_tcp_udp_port,
			.print = ipset_print_port,
			.opt = IPSET_OPT_PORT
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_PORTRANGE,
				IPSET_ARG_TIMEOUT,
				/* Backward compatibility */
				IPSET_ARG_FROM_PORT,
				IPSET_ARG_TO_PORT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "range [PROTO:]FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT),
			.help = "[PROTO:]PORT",
		},
	},
	.usage = "where PORT, FROM and TO are port numbers or port names from /etc/services.\n"
		 "      PROTO is only needed if a service name is used and it does not exist\n"
		 "      as a TCP service; just the resolved service numer is stored in the set.",
	.description = "Initial revision",
};

/* Counters support */
static struct ipset_type ipset_bitmap_port1 = {
	.name = "bitmap:port",
	.alias = { "portmap", NULL },
	.revision = 1,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_tcp_udp_port,
			.print = ipset_print_port,
			.opt = IPSET_OPT_PORT
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_PORTRANGE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				/* Backward compatibility */
				IPSET_ARG_FROM_PORT,
				IPSET_ARG_TO_PORT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "range [PROTO:]FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT),
			.help = "[PROTO:]PORT",
		},
	},
	.usage = "where PORT, FROM and TO are port numbers or port names from /etc/services.\n"
		 "      PROTO is only needed if a service name is used and it does not exist\n"
		 "      as a TCP service; just the resolved service numer is stored in the set.",
	.description = "counters support",
};

/* Comment support */
static struct ipset_type ipset_bitmap_port2 = {
	.name = "bitmap:port",
	.alias = { "portmap", NULL },
	.revision = 2,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_tcp_udp_port,
			.print = ipset_print_port,
			.opt = IPSET_OPT_PORT
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_PORTRANGE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				/* Backward compatibility */
				IPSET_ARG_FROM_PORT,
				IPSET_ARG_TO_PORT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "range [PROTO:]FROM-TO",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT),
			.help = "[PROTO:]PORT",
		},
	},
	.usage = "where PORT, FROM and TO are port numbers or port names from /etc/services.\n"
		 "      PROTO is only needed if a service name is used and it does not exist\n"
		 "      as a TCP service; just the resolved service numer is stored in the set.",
	.description = "comment support",
};

/* skbinfo support */
static struct ipset_type ipset_bitmap_port3 = {
	.name = "bitmap:port",
	.alias = { "portmap", NULL },
	.revision = 3,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_tcp_udp_port,
			.print = ipset_print_port,
			.opt = IPSET_OPT_PORT
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_PORTRANGE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_SKBINFO,
				/* Backward compatibility */
				IPSET_ARG_FROM_PORT,
				IPSET_ARG_TO_PORT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "range [PROTO:]FROM-TO",
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
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT)
				| IPSET_FLAG(IPSET_OPT_PORT_TO),
			.help = "[PROTO:]PORT|FROM-TO",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_PORT),
			.full = IPSET_FLAG(IPSET_OPT_PORT),
			.help = "[PROTO:]PORT",
		},
	},
	.usage = "where PORT, FROM and TO are port numbers or port names from /etc/services.\n"
		 "      PROTO is only needed if a service name is used and it does not exist\n"
		 "      as a TCP service; just the resolved service numer is stored in the set.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_bitmap_port0);
	ipset_type_add(&ipset_bitmap_port1);
	ipset_type_add(&ipset_bitmap_port2);
	ipset_type_add(&ipset_bitmap_port3);
}
