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

/* Initial revision */
static struct ipset_type ipset_list_set0 = {
	.name = "list:set",
	.alias = { "setlist", NULL },
	.revision = 0,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_setname,
			.print = ipset_print_name,
			.opt = IPSET_OPT_NAME
		},
	},
	.compat_parse_elem = ipset_parse_name_compat,
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_SIZE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
	},
	.usage = "where NAME are existing set names.",
	.description = "Initial revision",
};

/* counters support */
static struct ipset_type ipset_list_set1 = {
	.name = "list:set",
	.alias = { "setlist", NULL },
	.revision = 1,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_setname,
			.print = ipset_print_name,
			.opt = IPSET_OPT_NAME
		},
	},
	.compat_parse_elem = ipset_parse_name_compat,
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_SIZE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
	},
	.usage = "where NAME are existing set names.",
	.description = "counters support",
};

/* comment support */
static struct ipset_type ipset_list_set2 = {
	.name = "list:set",
	.alias = { "setlist", NULL },
	.revision = 2,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_setname,
			.print = ipset_print_name,
			.opt = IPSET_OPT_NAME
		},
	},
	.compat_parse_elem = ipset_parse_name_compat,
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_SIZE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
	},
	.usage = "where NAME are existing set names.",
	.description = "comment support",
};

/* skbinfo support */
static struct ipset_type ipset_list_set3 = {
	.name = "list:set",
	.alias = { "setlist", NULL },
	.revision = 3,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_setname,
			.print = ipset_print_name,
			.opt = IPSET_OPT_NAME
		},
	},
	.compat_parse_elem = ipset_parse_name_compat,
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_SIZE,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_SKBINFO,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_SKBMARK,
				IPSET_ARG_SKBPRIO,
				IPSET_ARG_SKBQUEUE,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_BEFORE,
				IPSET_ARG_AFTER,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_NAME),
			.full = IPSET_FLAG(IPSET_OPT_NAME)
				| IPSET_FLAG(IPSET_OPT_BEFORE),
			.help = "NAME [before|after NAME]",
		},
	},
	.usage = "where NAME are existing set names.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_list_set0);
	ipset_type_add(&ipset_list_set1);
	ipset_type_add(&ipset_list_set2);
	ipset_type_add(&ipset_list_set3);
}
