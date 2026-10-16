#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <limits.h> /* INT_MAX in ip6_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <netinet/icmp6.h>

#include "libxt_icmp.h"

enum {
	O_ICMPV6_TYPE = 0,
};

static void icmp6_help(void)
{
	printf(
"icmpv6 match options:\n"
"[!] --icmpv6-type typename	match icmpv6 type\n"
"				(or numeric type or type/code)\n");
	printf("Valid ICMPv6 Types:");
	xt_print_icmp_types(icmpv6_codes, ARRAY_SIZE(icmpv6_codes));
}

static const struct xt_option_entry icmp6_opts[] = {
	{.name = "icmpv6-type", .id = O_ICMPV6_TYPE, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void icmp6_init(struct xt_entry_match *m)
{
	struct ip6t_icmp *icmpv6info = (struct ip6t_icmp *)m->data;

	icmpv6info->code[1] = 0xFF;
}

static void icmp6_parse(struct xt_option_call *cb)
{
	struct ip6t_icmp *icmpv6info = cb->data;

	xtables_option_parse(cb);
	ipt_parse_icmpv6(cb->arg, &icmpv6info->type, icmpv6info->code);
	if (cb->invert)
		icmpv6info->invflags |= IP6T_ICMP_INV;
}

static void print_icmpv6type(uint8_t type,
			   uint8_t code_min, uint8_t code_max,
			   int invert,
			   int numeric)
{
	if (!numeric) {
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(icmpv6_codes); ++i)
			if (icmpv6_codes[i].type == type
			    && icmpv6_codes[i].code_min == code_min
			    && icmpv6_codes[i].code_max == code_max)
				break;

		if (i != ARRAY_SIZE(icmpv6_codes)) {
			printf(" %s%s",
			       invert ? "!" : "",
			       icmpv6_codes[i].name);
			return;
		}
	}

	if (invert)
		printf(" !");

	printf("type %u", type);
	if (code_min == code_max)
		printf(" code %u", code_min);
	else if (code_min != 0 || code_max != 0xFF)
		printf(" codes %u-%u", code_min, code_max);
}

static void icmp6_print(const void *ip, const struct xt_entry_match *match,
                        int numeric)
{
	const struct ip6t_icmp *icmpv6 = (struct ip6t_icmp *)match->data;

	printf(" ipv6-icmp");
	print_icmpv6type(icmpv6->type, icmpv6->code[0], icmpv6->code[1],
		       icmpv6->invflags & IP6T_ICMP_INV,
		       numeric);

	if (icmpv6->invflags & ~IP6T_ICMP_INV)
		printf(" Unknown invflags: 0x%X",
		       icmpv6->invflags & ~IP6T_ICMP_INV);
}

static void icmp6_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ip6t_icmp *icmpv6 = (struct ip6t_icmp *)match->data;

	if (icmpv6->invflags & IP6T_ICMP_INV)
		printf(" !");

	printf(" --icmpv6-type %u", icmpv6->type);
	if (icmpv6->code[0] != 0 || icmpv6->code[1] != 0xFF)
		printf("/%u", icmpv6->code[0]);
}

#define XT_ICMPV6_TYPE(type)	(type - ND_ROUTER_SOLICIT)

static const char *icmp6_type_xlate_array[] = {
	[XT_ICMPV6_TYPE(ND_ROUTER_SOLICIT)]	= "nd-router-solicit",
	[XT_ICMPV6_TYPE(ND_ROUTER_ADVERT)]	= "nd-router-advert",
	[XT_ICMPV6_TYPE(ND_NEIGHBOR_SOLICIT)]	= "nd-neighbor-solicit",
	[XT_ICMPV6_TYPE(ND_NEIGHBOR_ADVERT)]	= "nd-neighbor-advert",
	[XT_ICMPV6_TYPE(ND_REDIRECT)]		= "nd-redirect",
};

static const char *icmp6_type_xlate(unsigned int type)
{
	if (type < ND_ROUTER_SOLICIT || type > ND_REDIRECT)
		return NULL;

	return icmp6_type_xlate_array[XT_ICMPV6_TYPE(type)];
}

static unsigned int type_xlate_print(struct xt_xlate *xl, unsigned int icmptype,
				     unsigned int code_min,
				     unsigned int code_max)
{
	unsigned int i;
	const char *type_name;

	if (code_min == code_max)
		return 0;

	type_name = icmp6_type_xlate(icmptype);

	if (type_name) {
		xt_xlate_add(xl, "%s", type_name);
	} else {
		for (i = 0; i < ARRAY_SIZE(icmpv6_codes); ++i)
			if (icmpv6_codes[i].type == icmptype &&
			    icmpv6_codes[i].code_min == code_min &&
			    icmpv6_codes[i].code_max == code_max)
				break;

		if (i != ARRAY_SIZE(icmpv6_codes))
			xt_xlate_add(xl, "%s", icmpv6_codes[i].name);
		else
			return 0;
	}

	return 1;
}

static int icmp6_xlate(struct xt_xlate *xl,
		       const struct xt_xlate_mt_params *params)
{
	const struct ip6t_icmp *info = (struct ip6t_icmp *)params->match->data;

	xt_xlate_add(xl, "icmpv6 type%s ",
		     (info->invflags & IP6T_ICMP_INV) ? " !=" : "");

	if (!type_xlate_print(xl, info->type, info->code[0], info->code[1]))
		return 0;

	return 1;
}

static struct xtables_match icmp6_mt6_reg = {
	.name 		= "icmp6",
	.version 	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size		= XT_ALIGN(sizeof(struct ip6t_icmp)),
	.userspacesize	= XT_ALIGN(sizeof(struct ip6t_icmp)),
	.help		= icmp6_help,
	.init		= icmp6_init,
	.print		= icmp6_print,
	.save		= icmp6_save,
	.x6_parse	= icmp6_parse,
	.x6_options	= icmp6_opts,
	.xlate		= icmp6_xlate,
};

void _init(void)
{
	xtables_register_match(&icmp6_mt6_reg);
}
