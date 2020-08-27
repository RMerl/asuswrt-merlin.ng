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
static struct ipset_type ipset_hash_net0 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 0,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ipnet,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "Initial revision",
};

/* Add/del range support */
static struct ipset_type ipset_hash_net1 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 1,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "Add/del range support",
};

/* nomatch flag support */
static struct ipset_type ipset_hash_net2 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 2,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "nomatch flag support",
};

/* counters support */
static struct ipset_type ipset_hash_net3 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 3,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NOMATCH,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "counters support",
};

/* comment support */
static struct ipset_type ipset_hash_net4 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 4,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NOMATCH,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "comment support",
};

/* forceadd support */
static struct ipset_type ipset_hash_net5 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 5,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
				IPSET_ARG_HASHSIZE,
				IPSET_ARG_MAXELEM,
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_COUNTERS,
				IPSET_ARG_COMMENT,
				IPSET_ARG_FORCEADD,
				/* Ignored options: backward compatibilty */
				IPSET_ARG_PROBES,
				IPSET_ARG_RESIZE,
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NOMATCH,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "forceadd support",
};

/* skbinfo support */
static struct ipset_type ipset_hash_net6 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 6,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.cmd = {
		[IPSET_CREATE] = {
			.args = {
				IPSET_ARG_FAMILY,
				/* Aliases */
				IPSET_ARG_INET,
				IPSET_ARG_INET6,
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
				IPSET_ARG_NONE,
			},
			.need = 0,
			.full = 0,
			.help = "",
		},
		[IPSET_ADD] = {
			.args = {
				IPSET_ARG_TIMEOUT,
				IPSET_ARG_NOMATCH,
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
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO),
			.help = "IP[/CIDR]",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR),
			.help = "IP[/CIDR]",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is an IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.",
	.description = "skbinfo support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_hash_net0);
	ipset_type_add(&ipset_hash_net1);
	ipset_type_add(&ipset_hash_net2);
	ipset_type_add(&ipset_hash_net3);
	ipset_type_add(&ipset_hash_net4);
	ipset_type_add(&ipset_hash_net5);
	ipset_type_add(&ipset_hash_net6);
}
