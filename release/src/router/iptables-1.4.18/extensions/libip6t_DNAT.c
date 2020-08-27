/*
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * Based on Rusty Russell's IPv4 DNAT target. Development of IPv6 NAT
 * funded by Astaro.
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <iptables.h>
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/nf_nat.h>

enum {
	O_TO_DEST = 0,
	O_RANDOM,
	O_PERSISTENT,
	O_X_TO_DEST,
	F_TO_DEST   = 1 << O_TO_DEST,
	F_RANDOM   = 1 << O_RANDOM,
	F_X_TO_DEST = 1 << O_X_TO_DEST,
};

static void DNAT_help(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static const struct xt_option_entry DNAT_opts[] = {
	{.name = "to-destination", .id = O_TO_DEST, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_MULTI},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	{.name = "persistent", .id = O_PERSISTENT, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* Ranges expected in network order. */
static void
parse_to(const char *orig_arg, int portok, struct nf_nat_range *range)
{
	char *arg, *start, *end = NULL, *colon = NULL, *dash, *error;
	const struct in6_addr *ip;

	arg = strdup(orig_arg);
	if (arg == NULL)
		xtables_error(RESOURCE_PROBLEM, "strdup");

	start = strchr(arg, '[');
	if (start == NULL) {
		start = arg;
		/* Lets assume one colon is port information. Otherwise its an IPv6 address */
		colon = strchr(arg, ':');
		if (colon && strchr(colon+1, ':'))
			colon = NULL;
	}
	else {
		start++;
		end = strchr(start, ']');
		if (end == NULL)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid address format");

		*end = '\0';
		colon = strchr(end + 1, ':');
	}

	if (colon) {
		int port;

		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");

		range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

		port = atoi(colon+1);
		if (port <= 0 || port > 65535)
			xtables_error(PARAMETER_PROBLEM,
				   "Port `%s' not valid\n", colon+1);

		error = strchr(colon+1, ':');
		if (error)
			xtables_error(PARAMETER_PROBLEM,
				   "Invalid port:port syntax - use dash\n");

		dash = strchr(colon, '-');
		if (!dash) {
			range->min_proto.tcp.port
				= range->max_proto.tcp.port
				= htons(port);
		} else {
			int maxport;

			maxport = atoi(dash + 1);
			if (maxport <= 0 || maxport > 65535)
				xtables_error(PARAMETER_PROBLEM,
					   "Port `%s' not valid\n", dash+1);
			if (maxport < port)
				/* People are stupid. */
				xtables_error(PARAMETER_PROBLEM,
					   "Port range `%s' funky\n", colon+1);
			range->min_proto.tcp.port = htons(port);
			range->max_proto.tcp.port = htons(maxport);
		}
		/* Starts with colon or [] colon? No IP info...*/
		if (colon == arg || colon == arg+2) {
			free(arg);
			return;
		}
		*colon = '\0';
	}

	range->flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(start, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ip6addr(start);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			      start);
	range->min_addr.in6 = *ip;
	if (dash) {
		ip = xtables_numeric_to_ip6addr(dash + 1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				      dash+1);
		range->max_addr.in6 = *ip;
	} else
		range->max_addr = range->min_addr;

	free(arg);
	return;
}

static void DNAT_parse(struct xt_option_call *cb)
{
	const struct ip6t_entry *entry = cb->xt_entry;
	struct nf_nat_range *range = cb->data;
	int portok;

	if (entry->ipv6.proto == IPPROTO_TCP ||
	    entry->ipv6.proto == IPPROTO_UDP ||
	    entry->ipv6.proto == IPPROTO_SCTP ||
	    entry->ipv6.proto == IPPROTO_DCCP ||
	    entry->ipv6.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_DEST:
		if (cb->xflags & F_X_TO_DEST) {
			if (!kernel_version)
				get_kernel_version();
			if (kernel_version > LINUX_VERSION(2, 6, 10))
				xtables_error(PARAMETER_PROBLEM,
					   "DNAT: Multiple --to-destination not supported");
		}
		parse_to(cb->arg, portok, range);
		break;
	case O_PERSISTENT:
		range->flags |= NF_NAT_RANGE_PERSISTENT;
		break;
	}
}

static void DNAT_fcheck(struct xt_fcheck_call *cb)
{
	static const unsigned int f = F_TO_DEST | F_RANDOM;
	struct nf_nat_range *mr = cb->data;

	if ((cb->xflags & f) == f)
		mr->flags |= NF_NAT_RANGE_PROTO_RANDOM;
}

static void print_range(const struct nf_nat_range *range)
{
	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("[");
		printf("%s", xtables_ip6addr_to_numeric(&range->min_addr.in6));
		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr)))
			printf("-%s", xtables_ip6addr_to_numeric(&range->max_addr.in6));
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("]");
	}
	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(":");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
	}
}

static void DNAT_print(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	const struct nf_nat_range *range = (const void *)target->data;

	printf(" to:");
	print_range(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" persistent");
}

static void DNAT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_range *range = (const void *)target->data;

	printf(" --to-destination ");
	print_range(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" --random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" --persistent");
}

static struct xtables_target snat_tg_reg = {
	.name		= "DNAT",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.revision	= 1,
	.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
	.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
	.help		= DNAT_help,
	.x6_parse	= DNAT_parse,
	.x6_fcheck	= DNAT_fcheck,
	.print		= DNAT_print,
	.save		= DNAT_save,
	.x6_options	= DNAT_opts,
};

void _init(void)
{
	xtables_register_target(&snat_tg_reg);
}
