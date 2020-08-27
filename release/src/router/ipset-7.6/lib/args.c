/* Copyright 2017 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <libipset/types.h>			/* ipset_args[] */

static const struct ipset_arg ipset_args[] = {
	[IPSET_ARG_FAMILY] = {
		.name = { "family", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_FAMILY,
		.parse = ipset_parse_family,
		.print = ipset_print_family,
		.help = "[family inet|inet6]|[-4|-6]",
	},
	/* Alias: family inet */
	[IPSET_ARG_INET] = {
		.name = { "-4", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_FAMILY,
		.parse = ipset_parse_family,
		.help = "",
	},
	/* Alias: family inet6 */
	[IPSET_ARG_INET6] = {
		.name = { "-6", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_FAMILY,
		.parse = ipset_parse_family,
		.help = "",
	},
	/* Hash types */
	[IPSET_ARG_HASHSIZE] = {
		.name = { "hashsize", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_HASHSIZE,
		.parse = ipset_parse_uint32,
		.print = ipset_print_number,
		.help = "[hashsize VALUE]",
	},
	[IPSET_ARG_MAXELEM] = {
		.name = { "maxelem", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_MAXELEM,
		.parse = ipset_parse_uint32,
		.print = ipset_print_number,
		.help = "[maxelem VALUE]",
	},
	/* Ignored options: backward compatibilty */
	[IPSET_ARG_PROBES] = {
		.name = { "probes", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_PROBES,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	[IPSET_ARG_RESIZE] = {
		.name = { "resize", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_RESIZE,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	[IPSET_ARG_GC] = {
		.name = { "gc", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_GC,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	[IPSET_ARG_IGNORED_FROM] = {
		.name = { "from", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	[IPSET_ARG_IGNORED_TO] = {
		.name = { "to", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP_TO,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	[IPSET_ARG_IGNORED_NETWORK] = {
		.name = { "network", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP,
		.parse = ipset_parse_ignored,
		.print = ipset_print_number,
	},
	/* List type */
	[IPSET_ARG_SIZE] = {
		.name = { "size", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_SIZE,
		.parse = ipset_parse_uint32,
		.print = ipset_print_number,
		.help = "[size VALUE]",
	},
	/* IP-type elements */
	[IPSET_ARG_IPRANGE] = {
		.name = { "range", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP,
		.parse = ipset_parse_netrange,
		.print = ipset_print_ip,
	},
	[IPSET_ARG_NETMASK] = {
		.name = { "netmask", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_NETMASK,
		.parse = ipset_parse_netmask,
		.print = ipset_print_number,
		.help = "[netmask CIDR]",
	},
	/* Port-type elements */
	[IPSET_ARG_PORTRANGE] = {
		.name = { "range", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_PORT,
		.parse = ipset_parse_tcp_udp_port,
		.print = ipset_print_port,
	},
	/* Setname type elements */
	[IPSET_ARG_BEFORE] = {
		.name = { "before", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_NAMEREF,
		.parse = ipset_parse_before,
		.help = "[before|after NAME]",
	},
	[IPSET_ARG_AFTER] = {
		.name = { "after", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_NAMEREF,
		.parse = ipset_parse_after,
	},
	/* Backward compatibility */
	[IPSET_ARG_FROM_IP] = {
		.name = { "from", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP,
		.parse = ipset_parse_single_ip,
	},
	[IPSET_ARG_TO_IP] = {
		.name = { "to", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP_TO,
		.parse = ipset_parse_single_ip,
	},
	[IPSET_ARG_NETWORK] = {
		.name = { "network", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_IP,
		.parse = ipset_parse_net,
	},
	[IPSET_ARG_FROM_PORT] = {
		.name = { "from", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_PORT,
		.parse = ipset_parse_single_tcp_port,
	},
	[IPSET_ARG_TO_PORT] = {
		.name = { "to", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_PORT_TO,
		.parse = ipset_parse_single_tcp_port,
	},
	/* Extra flags, options */
	[IPSET_ARG_FORCEADD] = {
		.name = { "forceadd", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_FORCEADD,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[forceadd]",
	},
	[IPSET_ARG_MARKMASK] = {
		.name = { "markmask", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_MARKMASK,
		.parse = ipset_parse_uint32,
		.print = ipset_print_mark,
		.help = "markmask VALUE",
	},
	[IPSET_ARG_NOMATCH] = {
		.name = { "nomatch", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_NOMATCH,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[nomatch]",
	},
	[IPSET_ARG_IFACE_WILDCARD] = {
		.name = { "wildcard", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_IFACE_WILDCARD,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[wildcard]",
	},
	/* Extensions */
	[IPSET_ARG_TIMEOUT] = {
		.name = { "timeout", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_TIMEOUT,
		.parse = ipset_parse_timeout,
		.print = ipset_print_number,
		.help = "[timeout VALUE]",
	},
	[IPSET_ARG_COUNTERS] = {
		.name = { "counters", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_COUNTERS,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[counters]",
	},
	[IPSET_ARG_PACKETS] = {
		.name = { "packets", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_PACKETS,
		.parse = ipset_parse_uint64,
		.print = ipset_print_number,
		.help = "[packets VALUE]",
	},
	[IPSET_ARG_BYTES] = {
		.name = { "bytes", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_BYTES,
		.parse = ipset_parse_uint64,
		.print = ipset_print_number,
		.help = "[bytes VALUE]",
	},
	[IPSET_ARG_COMMENT] = {
		.name = { "comment", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_CREATE_COMMENT,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[comment]",
	},
	[IPSET_ARG_ADT_COMMENT] = {
		.name = { "comment", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_ADT_COMMENT,
		.parse = ipset_parse_comment,
		.print = ipset_print_comment,
		.help = "[comment \"string\"]",
	},
	[IPSET_ARG_SKBINFO] = {
		.name = { "skbinfo", NULL },
		.has_arg = IPSET_NO_ARG,
		.opt = IPSET_OPT_SKBINFO,
		.parse = ipset_parse_flag,
		.print = ipset_print_flag,
		.help = "[skbinfo]",
	},
	[IPSET_ARG_SKBMARK] = {
		.name = { "skbmark", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_SKBMARK,
		.parse = ipset_parse_skbmark,
		.print = ipset_print_skbmark,
		.help = "[skbmark VALUE]",
	},
	[IPSET_ARG_SKBPRIO] = {
		.name = { "skbprio", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_SKBPRIO,
		.parse = ipset_parse_skbprio,
		.print = ipset_print_skbprio,
		.help = "[skbprio VALUE]",
	},
	[IPSET_ARG_SKBQUEUE] = {
		.name = { "skbqueue", NULL },
		.has_arg = IPSET_MANDATORY_ARG,
		.opt = IPSET_OPT_SKBQUEUE,
		.parse = ipset_parse_uint16,
		.print = ipset_print_number,
		.help = "[skbqueue VALUE]",
	},
};

const struct ipset_arg *
ipset_keyword(enum ipset_keywords i)
{
	return (i > IPSET_ARG_NONE && i < IPSET_ARG_MAX)
			? &ipset_args[i] : NULL;
}

const char *
ipset_ignored_optname(unsigned int opt)
{
	enum ipset_keywords i;

	for (i = IPSET_ARG_NONE + 1 ; i < IPSET_ARG_MAX; i++)
		if (ipset_args[i].opt == opt)
			return ipset_args[i].name[0];
	return "";
}
