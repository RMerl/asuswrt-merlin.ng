/*
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * Based on Rusty Russell's IPv4 REDIRECT target. Development of IPv6 NAT
 * funded by Astaro.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/nf_nat.h>

enum {
	O_TO_PORTS = 0,
	O_RANDOM,
	F_TO_PORTS = 1 << O_TO_PORTS,
	F_RANDOM   = 1 << O_RANDOM,
};

static void REDIRECT_help(void)
{
	printf(
"REDIRECT target options:\n"
" --to-ports <port>[-<port>]\n"
"				Port (range) to map to.\n"
" [--random]\n");
}

static const struct xt_option_entry REDIRECT_opts[] = {
	{.name = "to-ports", .id = O_TO_PORTS, .type = XTTYPE_STRING},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* Parses ports */
static void
parse_ports(const char *arg, struct nf_nat_range *range)
{
	char *end = "";
	unsigned int port, maxport;

	range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

	if (!xtables_strtoui(arg, &end, &port, 0, UINT16_MAX) &&
	    (port = xtables_service_to_port(arg, NULL)) == (unsigned)-1)
		xtables_param_act(XTF_BAD_VALUE, "REDIRECT", "--to-ports", arg);

	switch (*end) {
	case '\0':
		range->min_proto.tcp.port
			= range->max_proto.tcp.port
			= htons(port);
		return;
	case '-':
		if (!xtables_strtoui(end + 1, NULL, &maxport, 0, UINT16_MAX) &&
		    (maxport = xtables_service_to_port(end + 1, NULL)) == (unsigned)-1)
			break;

		if (maxport < port)
			break;

		range->min_proto.tcp.port = htons(port);
		range->max_proto.tcp.port = htons(maxport);
		return;
	default:
		break;
	}
	xtables_param_act(XTF_BAD_VALUE, "REDIRECT", "--to-ports", arg);
}

static void REDIRECT_parse(struct xt_option_call *cb)
{
	const struct ip6t_entry *entry = cb->xt_entry;
	struct nf_nat_range *range = (void *)(*cb->target)->data;
	int portok;

	if (entry->ipv6.proto == IPPROTO_TCP
	    || entry->ipv6.proto == IPPROTO_UDP
	    || entry->ipv6.proto == IPPROTO_SCTP
	    || entry->ipv6.proto == IPPROTO_DCCP
	    || entry->ipv6.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_PORTS:
		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");
		parse_ports(cb->arg, range);
		if (cb->xflags & F_RANDOM)
			range->flags |= NF_NAT_RANGE_PROTO_RANDOM;
		break;
	case O_RANDOM:
		if (cb->xflags & F_TO_PORTS)
			range->flags |= NF_NAT_RANGE_PROTO_RANDOM;
		break;
	}
}

static void REDIRECT_print(const void *ip, const struct xt_entry_target *target,
                           int numeric)
{
	const struct nf_nat_range *range = (const void *)target->data;

	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(" redir ports ");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
		if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
			printf(" random");
	}
}

static void REDIRECT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_range *range = (const void *)target->data;

	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(" --to-ports ");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
		if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
			printf(" --random");
	}
}

static struct xtables_target redirect_tg_reg = {
	.name		= "REDIRECT",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
	.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
	.help		= REDIRECT_help,
	.x6_parse	= REDIRECT_parse,
	.print		= REDIRECT_print,
	.save		= REDIRECT_save,
	.x6_options	= REDIRECT_opts,
};

void _init(void)
{
	xtables_register_target(&redirect_tg_reg);
}
