/* Copyright 2011 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <libipset/data.h>			/* IPSET_OPT_* */
#include <libipset/parse.h>			/* parser functions */
#include <libipset/print.h>			/* printing functions */
#include <libipset/ipset.h>			/* ipset_port_usage */
#include <libipset/types.h>			/* prototypes */

/* Initial revision */
static struct ipset_type ipset_hash_netiface0 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 0,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "Initial revision",
};

/* nomatch flag support */
static struct ipset_type ipset_hash_netiface1 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 1,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "nomatch flag support",
};

/* /0 network support */
static struct ipset_type ipset_hash_netiface2 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 2,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "/0 network support",
};

/* counters support */
static struct ipset_type ipset_hash_netiface3 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 3,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "counters support",
};

/* comment support */
static struct ipset_type ipset_hash_netiface4 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 4,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "comment support",
};

/* forceadd support */
static struct ipset_type ipset_hash_netiface5 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 5,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "forceadd support",
};

/* skbinfo support */
static struct ipset_type ipset_hash_netiface6 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 6,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "skbinfo support",
};
/* interface wildcard support */
static struct ipset_type ipset_hash_netiface7 = {
	.name = "hash:net,iface",
	.alias = { "netifacehash", NULL },
	.revision = 7,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_TWO,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
		[IPSET_DIM_TWO - 1] = {
			.parse = ipset_parse_iface,
			.print = ipset_print_iface,
			.opt = IPSET_OPT_IFACE
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
				IPSET_ARG_IFACE_WILDCARD,
				IPSET_ARG_PACKETS,
				IPSET_ARG_BYTES,
				IPSET_ARG_ADT_COMMENT,
				IPSET_ARG_SKBMARK,
				IPSET_ARG_SKBPRIO,
				IPSET_ARG_SKBQUEUE,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_DEL] = {
			.args = {
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IP_TO)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR]|FROM-TO,[physdev:]IFACE",
		},
		[IPSET_TEST] = {
			.args = {
				IPSET_ARG_NOMATCH,
				IPSET_ARG_NONE,
			},
			.need = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_IFACE),
			.full = IPSET_FLAG(IPSET_OPT_IP)
				| IPSET_FLAG(IPSET_OPT_CIDR)
				| IPSET_FLAG(IPSET_OPT_IFACE)
				| IPSET_FLAG(IPSET_OPT_PHYSDEV),
			.help = "IP[/CIDR],[physdev:]IFACE",
		},
	},
	.usage = "where depending on the INET family\n"
		 "      IP is a valid IPv4 or IPv6 address (or hostname),\n"
		 "      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
		 "      Adding/deleting multiple elements with IPv4 is supported.",
	.description = "skbinfo and wildcard support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_hash_netiface0);
	ipset_type_add(&ipset_hash_netiface1);
	ipset_type_add(&ipset_hash_netiface2);
	ipset_type_add(&ipset_hash_netiface3);
	ipset_type_add(&ipset_hash_netiface4);
	ipset_type_add(&ipset_hash_netiface5);
	ipset_type_add(&ipset_hash_netiface6);
	ipset_type_add(&ipset_hash_netiface7);
}
