/*
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * Based on Rusty Russell's IPv4 MASQUERADE target. Development of IPv6 NAT
 * funded by Astaro.
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/nf_nat.h>

enum {
	O_TO_PORTS = 0,
	O_RANDOM,
};

static void MASQUERADE_help(void)
{
	printf(
"MASQUERADE target options:\n"
" --to-ports <port>[-<port>]\n"
"				Port (range) to map to.\n"
" --random\n"
"				Randomize source port.\n");
}

static const struct xt_option_entry MASQUERADE_opts[] = {
	{.name = "to-ports", .id = O_TO_PORTS, .type = XTTYPE_STRING},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* Parses ports */
static void
parse_ports(const char *arg, struct nf_nat_range *r)
{
	char *end;
	unsigned int port, maxport;

	r->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

	if (!xtables_strtoui(arg, &end, &port, 0, UINT16_MAX))
		xtables_param_act(XTF_BAD_VALUE, "MASQUERADE", "--to-ports", arg);

	switch (*end) {
	case '\0':
		r->min_proto.tcp.port
			= r->max_proto.tcp.port
			= htons(port);
		return;
	case '-':
		if (!xtables_strtoui(end + 1, NULL, &maxport, 0, UINT16_MAX))
			break;

		if (maxport < port)
			break;

		r->min_proto.tcp.port = htons(port);
		r->max_proto.tcp.port = htons(maxport);
		return;
	default:
		break;
	}
	xtables_param_act(XTF_BAD_VALUE, "MASQUERADE", "--to-ports", arg);
}

static void MASQUERADE_parse(struct xt_option_call *cb)
{
	const struct ip6t_entry *entry = cb->xt_entry;
	struct nf_nat_range *r = cb->data;
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
	case O_TO_PORTS:
		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");
		parse_ports(cb->arg, r);
		break;
	case O_RANDOM:
		r->flags |=  NF_NAT_RANGE_PROTO_RANDOM;
		break;
	}
}

static void
MASQUERADE_print(const void *ip, const struct xt_entry_target *target,
                 int numeric)
{
	const struct nf_nat_range *r = (const void *)target->data;

	if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(" masq ports: ");
		printf("%hu", ntohs(r->min_proto.tcp.port));
		if (r->max_proto.tcp.port != r->min_proto.tcp.port)
			printf("-%hu", ntohs(r->max_proto.tcp.port));
	}

	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" random");
}

static void
MASQUERADE_save(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_range *r = (const void *)target->data;

	if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(" --to-ports %hu", ntohs(r->min_proto.tcp.port));
		if (r->max_proto.tcp.port != r->min_proto.tcp.port)
			printf("-%hu", ntohs(r->max_proto.tcp.port));
	}

	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" --random");
}

static struct xtables_target masquerade_tg_reg = {
	.name		= "MASQUERADE",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
	.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
	.help		= MASQUERADE_help,
	.x6_parse	= MASQUERADE_parse,
	.print		= MASQUERADE_print,
	.save		= MASQUERADE_save,
	.x6_options	= MASQUERADE_opts,
};

void _init(void)
{
	xtables_register_target(&masquerade_tg_reg);
}
