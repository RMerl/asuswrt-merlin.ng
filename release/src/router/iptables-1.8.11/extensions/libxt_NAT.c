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
#include <iptables.h> /* get_kernel_version */
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <arpa/inet.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/nf_nat.h>

#define TO_IPV4_MRC(ptr) ((const struct nf_nat_ipv4_multi_range_compat *)(ptr))
#define RANGE2_INIT_FROM_IPV4_MRC(ptr) {			\
	.flags		= TO_IPV4_MRC(ptr)->range[0].flags,	\
	.min_addr.ip	= TO_IPV4_MRC(ptr)->range[0].min_ip,	\
	.max_addr.ip	= TO_IPV4_MRC(ptr)->range[0].max_ip,	\
	.min_proto	= TO_IPV4_MRC(ptr)->range[0].min,	\
	.max_proto	= TO_IPV4_MRC(ptr)->range[0].max,	\
};
#define TO_NF_NAT_RANGE(ptr) ((const struct nf_nat_range *)(ptr))
#define RANGE2_INIT_FROM_RANGE(ptr) {				\
	.flags		= TO_NF_NAT_RANGE(ptr)->flags,		\
	.min_addr	= TO_NF_NAT_RANGE(ptr)->min_addr,	\
	.max_addr	= TO_NF_NAT_RANGE(ptr)->max_addr,	\
	.min_proto	= TO_NF_NAT_RANGE(ptr)->min_proto,	\
	.max_proto	= TO_NF_NAT_RANGE(ptr)->max_proto,	\
};

enum {
	O_TO_DEST = 0,
	O_TO_SRC,
	O_TO_PORTS,
	O_RANDOM,
	O_RANDOM_FULLY,
	O_MODE,
	O_PERSISTENT,
	O_PSID,
};

static void SNAT_help(void)
{
	printf(
"SNAT target options:\n"
" --to-source [<ipaddr>[-<ipaddr>]][:port[-port]]\n"
"				Address to map source to.\n"
" --psid <offset,length,psid>\n"
"				Port Set to map source to.\n"
"[--random] [--random-fully] [--persistent]\n");
}

static void MASQUERADE_help(void)
{
	printf(
"MASQUERADE target options:\n"
" --to-ports <port>[-<port>]\n"
"				Port (range) to map to.\n"
" --psid <offset,length,psid>\n"
"				Port Set to map to.\n"
" --random\n"
"				Randomize source port.\n"
" --random-fully\n"
"				Fully randomize source port.\n"
" --mode <fullcone|symmetric>\n"
"				NAT mode.\n");
}

static void DNAT_help(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static void DNAT_help_v2(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port[/port]]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static void REDIRECT_help(void)
{
	printf(
"REDIRECT target options:\n"
" --to-ports <port>[-<port>]\n"
"				Port (range) to map to.\n"
" [--random]\n");
}

static const struct xt_option_entry SNAT_opts[] = {
	{.name = "to-source", .id = O_TO_SRC, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	{.name = "random-fully", .id = O_RANDOM_FULLY, .type = XTTYPE_NONE},
	{.name = "persistent", .id = O_PERSISTENT, .type = XTTYPE_NONE},
	{.name = "psid", .id = O_PSID, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry MASQUERADE_opts[] = {
	{.name = "to-ports", .id = O_TO_PORTS, .type = XTTYPE_STRING},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	{.name = "random-fully", .id = O_RANDOM_FULLY, .type = XTTYPE_NONE},
	{.name = "mode", .id = O_MODE, .type = XTTYPE_STRING},
	{.name = "psid", .id = O_PSID, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry DNAT_opts[] = {
	{.name = "to-destination", .id = O_TO_DEST, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	{.name = "persistent", .id = O_PERSISTENT, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry REDIRECT_opts[] = {
	{.name = "to-ports", .id = O_TO_PORTS, .type = XTTYPE_STRING},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* Parses ports */
static void
parse_ports(const char *arg, bool portok, struct nf_nat_range2 *range)
{
	unsigned int port, maxport, baseport;
	char *end = NULL;

	if (!portok)
		xtables_error(PARAMETER_PROBLEM,
			      "Need TCP, UDP, SCTP or DCCP with port specification");

	range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

	if (!xtables_strtoui(arg, &end, &port, 0, UINT16_MAX)) {
		port = xtables_service_to_port(arg, NULL);
		if (port == (unsigned)-1)
			xtables_error(PARAMETER_PROBLEM,
				      "Port `%s' not valid", arg);
		end = "";
	}

	switch (*end) {
	case '\0':
		range->min_proto.tcp.port
			= range->max_proto.tcp.port
			= htons(port);
		return;
	case '-':
		arg = end + 1;
		break;
	case ':':
		xtables_error(PARAMETER_PROBLEM,
			      "Invalid port:port syntax - use dash");
	default:
		xtables_error(PARAMETER_PROBLEM,
			      "Garbage after port value: `%s'", end);
	}

	/* it is a range, don't allow service names here */
	if (!xtables_strtoui(arg, &end, &maxport, 0, UINT16_MAX))
		xtables_error(PARAMETER_PROBLEM, "Port `%s' not valid", arg);

	if (maxport < port)
		/* People are stupid. */
		xtables_error(PARAMETER_PROBLEM,
			   "Port range `%s' funky", arg);

	range->min_proto.tcp.port = htons(port);
	range->max_proto.tcp.port = htons(maxport);

	switch (*end) {
	case '\0':
		return;
	case '/':
		arg = end + 1;
		break;
	default:
		xtables_error(PARAMETER_PROBLEM,
			      "Garbage after port range: `%s'", end);
	}

	if (!xtables_strtoui(arg, &end, &baseport, 1, UINT16_MAX)) {
		baseport = xtables_service_to_port(arg, NULL);
		if (baseport == (unsigned)-1)
			xtables_error(PARAMETER_PROBLEM,
				      "Port `%s' not valid", arg);
	}

	range->flags |= NF_NAT_RANGE_PROTO_OFFSET;
	range->base_proto.tcp.port = htons(baseport);
}

/* Ranges expected in network order. */
static void
parse_to(const char *orig_arg, bool portok,
	 struct nf_nat_range2 *range, int family)
{
	char *arg, *start, *end, *colon, *dash;

	arg = xtables_strdup(orig_arg);
	start = strchr(arg, '[');
	if (!start) {
		start = arg;
		/* Lets assume one colon is port information.
		 * Otherwise its an IPv6 address */
		colon = strchr(arg, ':');
		if (colon && strchr(colon + 1, ':'))
			colon = NULL;
	} else {
		start++;
		end = strchr(start, ']');
		if (end == NULL || family == AF_INET)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid address format");

		*end = '\0';
		colon = strchr(end + 1, ':');
	}

	if (colon) {
		parse_ports(colon + 1, portok, range);

		/* Starts with colon or [] colon? No IP info...*/
		if (colon == arg || colon == arg + 2) {
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

	if (!inet_pton(family, start, &range->min_addr))
		xtables_error(PARAMETER_PROBLEM,
			      "Bad IP address \"%s\"", start);
	if (dash) {
		if (!inet_pton(family, dash + 1, &range->max_addr))
			xtables_error(PARAMETER_PROBLEM,
				      "Bad IP address \"%s\"", dash + 1);
	} else {
		range->max_addr = range->min_addr;
	}
	free(arg);
	return;
}

/* Parses a MAP-E port-set: --psid offset,length,psid */
static void
parse_psid(const char *arg, bool portok, struct nf_nat_range2 *range)
{
	char *end;
	unsigned int offset, length, psid;

	if (!portok)
		xtables_error(PARAMETER_PROBLEM,
			      "Need TCP, UDP, ICMP, SCTP or DCCP with port specification");

	if (!xtables_strtoui(arg, &end, &offset, 0, 16) || *end++ != ',' ||
	    !xtables_strtoui(end, &end, &length, 0, 16) || *end++ != ',' ||
	    !xtables_strtoui(end, &end, &psid, 0, UINT16_MAX) || *end != '\0')
		xtables_error(PARAMETER_PROBLEM,
			      "Invalid --psid syntax");

	if (offset + length > 16 ||
	    (1 << (16 - length)) - (!!offset << (16 - offset - length)) <= 0)
		xtables_error(PARAMETER_PROBLEM,
			      "Invalid --psid range");

	range->flags |= NF_NAT_RANGE_PROTO_PSID;
	range->min_proto.psid.offset = offset;
	range->min_proto.psid.length = length;
	range->max_proto.psid.id = psid & ((1 << length) - 1);
}

static void __NAT_parse(struct xt_option_call *cb, __u16 proto,
			struct nf_nat_range2 *range, int family)
{
	bool portok = proto == IPPROTO_TCP ||
		      proto == IPPROTO_UDP ||
		      proto == IPPROTO_SCTP ||
		      proto == IPPROTO_DCCP ||
		      proto == IPPROTO_ICMP;

	/* Borrow this field as mode value. The default is symmetric */
	struct nf_nat_ipv4_multi_range_compat *mr = cb->data; mr->range[0].min_ip = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_DEST:
	case O_TO_SRC:
		parse_to(cb->arg, portok, range, family);
		break;
	case O_TO_PORTS:
		parse_ports(cb->arg, portok, range);
		break;
	case O_PERSISTENT:
		range->flags |= NF_NAT_RANGE_PERSISTENT;
		break;
	case O_RANDOM:
		range->flags |= NF_NAT_RANGE_PROTO_RANDOM;
		break;
	case O_RANDOM_FULLY:
		range->flags |= NF_NAT_RANGE_PROTO_RANDOM_FULLY;
		break;
	case O_PSID:
		parse_psid(cb->arg, portok, range);
		break;
	case O_MODE:
		if (strcasecmp(cb->arg, "fullcone") == 0)
			mr->range[0].min_ip = 1;
		else if (strcasecmp(cb->arg, "symmetric") == 0)
			mr->range[0].min_ip = 0;
		else
			xtables_error(PARAMETER_PROBLEM,
				   "Unknown mode %s", cb->arg);
      		break;		
	}
}

static void NAT_parse(struct xt_option_call *cb)
{
	struct nf_nat_ipv4_multi_range_compat *mr = (void *)cb->data;
	const struct ipt_entry *entry = cb->xt_entry;
	struct nf_nat_range2 range = {};

	__NAT_parse(cb, entry->ip.proto, &range, AF_INET);

	switch (cb->entry->id) {
	case O_TO_DEST:
	case O_TO_SRC:
		mr->range->min_ip = range.min_addr.ip;
		mr->range->max_ip = range.max_addr.ip;
		/* fall through */
	case O_PSID:
	case O_TO_PORTS:
		mr->range->min = range.min_proto;
		mr->range->max = range.max_proto;
		/* fall through */
	case O_PERSISTENT:
	case O_RANDOM:
	case O_RANDOM_FULLY:
		mr->range->flags |= range.flags;
		break;
	}
}

static void NAT_parse6(struct xt_option_call *cb)
{
	struct nf_nat_range2 range = RANGE2_INIT_FROM_RANGE(cb->data);
	struct nf_nat_range *range_v1 = (void *)cb->data;
	const struct ip6t_entry *entry = cb->xt_entry;

	__NAT_parse(cb, entry->ipv6.proto, &range, AF_INET6);
	memcpy(range_v1, &range, sizeof(*range_v1));
}

static void DNAT_parse_v2(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;

	__NAT_parse(cb, entry->ip.proto, cb->data, AF_INET);
}

static void DNAT_parse6_v2(struct xt_option_call *cb)
{
	const struct ip6t_entry *entry = cb->xt_entry;

	__NAT_parse(cb, entry->ipv6.proto, cb->data, AF_INET6);
}

static void SNAT_fcheck(struct xt_fcheck_call *cb)
{
	struct nf_nat_ipv4_multi_range_compat *mr = cb->data;

	mr->rangesize = 1;
}

static void DNAT_fcheck(struct xt_fcheck_call *cb)
{
	struct nf_nat_ipv4_multi_range_compat *mr = cb->data;

	mr->rangesize = 1;

	if (mr->range[0].flags & NF_NAT_RANGE_PROTO_OFFSET)
		xtables_error(PARAMETER_PROBLEM,
			      "Shifted portmap ranges not supported with this kernel");
}

static void DNAT_fcheck6(struct xt_fcheck_call *cb)
{
	struct nf_nat_range *range = (void *)cb->data;

	if (range->flags & NF_NAT_RANGE_PROTO_OFFSET)
		xtables_error(PARAMETER_PROBLEM,
			      "Shifted portmap ranges not supported with this kernel");
}

static char *sprint_range(const struct nf_nat_range2 *r, int family)
{
	bool brackets = family == AF_INET6 &&
			r->flags & NF_NAT_RANGE_PROTO_SPECIFIED;
	static char buf[INET6_ADDRSTRLEN * 2 + 3 + 6 * 3];

	buf[0] = '\0';

	if (r->flags & NF_NAT_RANGE_MAP_IPS) {
		if (brackets)
			strcat(buf, "[");
		inet_ntop(family, &r->min_addr,
			  buf + strlen(buf), INET6_ADDRSTRLEN);
		if (memcmp(&r->min_addr, &r->max_addr, sizeof(r->min_addr))) {
			strcat(buf, "-");
			inet_ntop(family, &r->max_addr,
				  buf + strlen(buf), INET6_ADDRSTRLEN);
		}
		if (brackets)
			strcat(buf, "]");
	}
	if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		sprintf(buf + strlen(buf), ":%hu",
			ntohs(r->min_proto.tcp.port));
		if (r->max_proto.tcp.port != r->min_proto.tcp.port)
			sprintf(buf + strlen(buf), "-%hu",
				ntohs(r->max_proto.tcp.port));
		if (r->flags & NF_NAT_RANGE_PROTO_OFFSET)
			sprintf(buf + strlen(buf), "/%hu",
				ntohs(r->base_proto.tcp.port));
	}
	return buf;
}

static void __NAT_print(const struct xt_entry_target *target, const struct nf_nat_range2 *r, int family,
			const char *rangeopt, const char *flag_pfx,
			bool skip_colon)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = (const void *)target->data;
	const struct nf_nat_ipv4_range *r1 = &mr->range[0];

	char *range_str = sprint_range(r, family);

	if (strlen(range_str)) {
		if (range_str[0] == ':' && skip_colon)
			range_str++;
		printf(" %s%s", rangeopt, range_str);
	}
	if (r->flags & NF_NAT_RANGE_PROTO_PSID)
		printf(" %spsid %u,%u,0x%x", flag_pfx,
		       r->min_proto.psid.offset,
		       r->min_proto.psid.length,
		       r->max_proto.psid.id);
	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" %srandom", flag_pfx);
	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY)
		printf(" %srandom-fully", flag_pfx);
	if (r1->min_ip == 1)
		printf("mode: fullcone ");
	if (r->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" %spersistent", flag_pfx);
}

static int
__NAT_xlate(struct xt_xlate *xl, const struct nf_nat_range2 *r,
	     int family, const char *tgt)
{
	char *range_str = sprint_range(r, family);
	const char *sep = " ";

	/* shifted portmap ranges are not supported by nftables */
	if (r->flags & NF_NAT_RANGE_PROTO_OFFSET)
		return 0;

	/* MAP-E PSID port-set matching cannot be expressed in nftables */
	if (r->flags & NF_NAT_RANGE_PROTO_PSID)
		return 0;

	xt_xlate_add(xl, "%s", tgt);
	if (strlen(range_str))
		xt_xlate_add(xl, " to %s", range_str);
	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM) {
		xt_xlate_add(xl, "%srandom", sep);
		sep = ",";
	}
	if (r->flags & NF_NAT_RANGE_PROTO_RANDOM_FULLY) {
		xt_xlate_add(xl, "%sfully-random", sep);
		sep = ",";
	}
	if (r->flags & NF_NAT_RANGE_PERSISTENT) {
		xt_xlate_add(xl, "%spersistent", sep);
		sep = ",";
	}
	return 1;
}

#define PSX_GEN(name, converter, family,                                       \
		print_rangeopt, save_rangeopt, skip_colon, xlate)              \
static void name##_print(const void *ip, const struct xt_entry_target *target, \
			 int numeric)                                          \
{                                                                              \
	struct nf_nat_range2 range = converter(target->data);                  \
	                                                                       \
	__NAT_print(target, &range, family, print_rangeopt, "", skip_colon);           \
}                                                                              \
static void name##_save(const void *ip, const struct xt_entry_target *target)  \
{                                                                              \
	struct nf_nat_range2 range = converter(target->data);                  \
	                                                                       \
	__NAT_print(target, &range, family, save_rangeopt, "--", skip_colon);          \
}                                                                              \
static int name##_xlate(struct xt_xlate *xl,                                   \
			const struct xt_xlate_tg_params *params)               \
{                                                                              \
	struct nf_nat_range2 range = converter(params->target->data);          \
	                                                                       \
	return __NAT_xlate(xl, &range, family, xlate);                         \
}

PSX_GEN(DNAT, RANGE2_INIT_FROM_IPV4_MRC, \
	AF_INET, "to:", "--to-destination ", false, "dnat")

PSX_GEN(DNATv2, *(struct nf_nat_range2 *), \
	AF_INET, "to:", "--to-destination ", false, "dnat")

PSX_GEN(DNAT6, RANGE2_INIT_FROM_RANGE, \
	AF_INET6, "to:", "--to-destination ", false, "dnat")

PSX_GEN(DNAT6v2, *(struct nf_nat_range2 *), \
	AF_INET6, "to:", "--to-destination ", false, "dnat")

PSX_GEN(REDIRECT, RANGE2_INIT_FROM_IPV4_MRC, \
	AF_INET, "redir ports ", "--to-ports ", true, "redirect")

PSX_GEN(REDIRECT6, RANGE2_INIT_FROM_RANGE, \
	AF_INET6, "redir ports ", "--to-ports ", true, "redirect")

PSX_GEN(SNAT, RANGE2_INIT_FROM_IPV4_MRC, \
	AF_INET, "to:", "--to-source ", false, "snat")

PSX_GEN(SNAT6, RANGE2_INIT_FROM_RANGE, \
	AF_INET6, "to:", "--to-source ", false, "snat")

PSX_GEN(MASQUERADE, RANGE2_INIT_FROM_IPV4_MRC, \
	AF_INET, "masq ports: ", "--to-ports ", true, "masquerade")

PSX_GEN(MASQUERADE6, RANGE2_INIT_FROM_RANGE, \
	AF_INET6, "masq ports: ", "--to-ports ", true, "masquerade")

static struct xtables_target nat_tg_reg[] = {
	{
		.name		= "SNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.help		= SNAT_help,
		.x6_parse	= NAT_parse,
		.x6_fcheck	= SNAT_fcheck,
		.print		= SNAT_print,
		.save		= SNAT_save,
		.x6_options	= SNAT_opts,
		.xlate		= SNAT_xlate,
	},
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.revision	= 0,
		.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.help		= DNAT_help,
		.print		= DNAT_print,
		.save		= DNAT_save,
		.x6_parse	= NAT_parse,
		.x6_fcheck	= DNAT_fcheck,
		.x6_options	= DNAT_opts,
		.xlate		= DNAT_xlate,
	},
	{
		.name		= "MASQUERADE",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.help		= MASQUERADE_help,
		.x6_parse	= NAT_parse,
		.x6_fcheck	= SNAT_fcheck,
		.print		= MASQUERADE_print,
		.save		= MASQUERADE_save,
		.x6_options	= MASQUERADE_opts,
		.xlate		= MASQUERADE_xlate,
	},
	{
		.name		= "MASQUERADE",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
		.help		= MASQUERADE_help,
		.x6_parse	= NAT_parse6,
		.print		= MASQUERADE6_print,
		.save		= MASQUERADE6_save,
		.x6_options	= MASQUERADE_opts,
		.xlate		= MASQUERADE6_xlate,
	},
	{
		.name		= "REDIRECT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.revision	= 0,
		.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.help		= REDIRECT_help,
		.print		= REDIRECT_print,
		.save		= REDIRECT_save,
		.x6_parse	= NAT_parse,
		.x6_fcheck	= DNAT_fcheck,
		.x6_options	= REDIRECT_opts,
		.xlate		= REDIRECT_xlate,
	},
	{
		.name		= "SNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.revision	= 1,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
		.help		= SNAT_help,
		.x6_parse	= NAT_parse6,
		.print		= SNAT6_print,
		.save		= SNAT6_save,
		.x6_options	= SNAT_opts,
		.xlate		= SNAT6_xlate,
	},
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.revision	= 1,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
		.help		= DNAT_help,
		.print		= DNAT6_print,
		.save		= DNAT6_save,
		.x6_parse	= NAT_parse6,
		.x6_fcheck	= DNAT_fcheck6,
		.x6_options	= DNAT_opts,
		.xlate		= DNAT6_xlate,
	},
	{
		.name		= "REDIRECT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
		.help		= REDIRECT_help,
		.print		= REDIRECT6_print,
		.save		= REDIRECT6_save,
		.x6_parse	= NAT_parse6,
		.x6_fcheck	= DNAT_fcheck6,
		.x6_options	= REDIRECT_opts,
		.xlate		= REDIRECT6_xlate,
	},
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.revision	= 2,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.help		= DNAT_help_v2,
		.print		= DNATv2_print,
		.save		= DNATv2_save,
		.x6_parse	= DNAT_parse_v2,
		.x6_options	= DNAT_opts,
		.xlate		= DNATv2_xlate,
	},
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.revision	= 2,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.help		= DNAT_help_v2,
		.print		= DNAT6v2_print,
		.save		= DNAT6v2_save,
		.x6_parse	= DNAT_parse6_v2,
		.x6_options	= DNAT_opts,
		.xlate		= DNAT6v2_xlate,
	},
};

void _init(void)
{
	xtables_register_targets(nat_tg_reg, ARRAY_SIZE(nat_tg_reg));
}
