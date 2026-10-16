/* Shared library add-on to ip6tables to add customized REJECT support.
 *
 * (C) 2000 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 * 
 * ported to IPv6 by Harald Welte <laforge@gnumonks.org>
 *
 */
#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter_ipv6/ip6t_REJECT.h>

struct reject_names {
	const char *name;
	const char *alias;
	const char *desc;
	const char *xlate;
};

enum {
	O_REJECT_WITH = 0,
};

static const struct reject_names reject_table[] = {
	[IP6T_ICMP6_NO_ROUTE] = {
		"icmp6-no-route", "no-route",
		"ICMPv6 no route",
		"no-route",
	},
	[IP6T_ICMP6_ADM_PROHIBITED] = {
		"icmp6-adm-prohibited", "adm-prohibited",
		"ICMPv6 administratively prohibited",
		"admin-prohibited",
	},
#if 0
	[IP6T_ICMP6_NOT_NEIGHBOR] = {
		"icmp6-not-neighbor", "not-neighbor",
		"ICMPv6 not a neighbor",
	},
#endif
	[IP6T_ICMP6_ADDR_UNREACH] = {
		"icmp6-addr-unreachable", "addr-unreach",
		"ICMPv6 address unreachable",
		"addr-unreachable",
	},
	[IP6T_ICMP6_PORT_UNREACH] = {
		"icmp6-port-unreachable", "port-unreach",
		"ICMPv6 port unreachable",
		"port-unreachable",
	},
#if 0
	[IP6T_ICMP6_ECHOREPLY] = {},
#endif
	[IP6T_TCP_RESET] = {
		"tcp-reset", "tcp-reset",
		"TCP RST packet",
		"tcp reset",
	},
	[IP6T_ICMP6_POLICY_FAIL] = {
		"icmp6-policy-fail", "policy-fail",
		"ICMPv6 policy fail",
		"policy-fail",
	},
	[IP6T_ICMP6_REJECT_ROUTE] = {
		"icmp6-reject-route", "reject-route",
		"ICMPv6 reject route",
		"reject-route",
	},
};

static void
print_reject_types(void)
{
	unsigned int i;

	printf("Valid reject types:\n");

	for (i = 0; i < ARRAY_SIZE(reject_table); ++i) {
		if (!reject_table[i].name)
			continue;
		printf("    %-25s\t%s\n", reject_table[i].name, reject_table[i].desc);
		printf("    %-25s\talias\n", reject_table[i].alias);
	}
	printf("\n");
}

static void REJECT_help(void)
{
	printf(
"REJECT target options:\n"
"--reject-with type              drop input packet and send back\n"
"                                a reply packet according to type:\n");

	print_reject_types();
}

static const struct xt_option_entry REJECT_opts[] = {
	{.name = "reject-with", .id = O_REJECT_WITH, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};

static void REJECT_init(struct xt_entry_target *t)
{
	struct ip6t_reject_info *reject = (struct ip6t_reject_info *)t->data;

	/* default */
	reject->with = IP6T_ICMP6_PORT_UNREACH;

}

static void REJECT_parse(struct xt_option_call *cb)
{
	struct ip6t_reject_info *reject = cb->data;
	unsigned int i;

	xtables_option_parse(cb);
	for (i = 0; i < ARRAY_SIZE(reject_table); ++i) {
		if (!reject_table[i].name)
			continue;
		if (strncasecmp(reject_table[i].name,
		      cb->arg, strlen(cb->arg)) == 0 ||
		    strncasecmp(reject_table[i].alias,
		      cb->arg, strlen(cb->arg)) == 0) {
			reject->with = i;
			return;
		}
	}
	xtables_error(PARAMETER_PROBLEM,
		"unknown reject type \"%s\"", cb->arg);
}

static void REJECT_print(const void *ip, const struct xt_entry_target *target,
                         int numeric)
{
	const struct ip6t_reject_info *reject
		= (const struct ip6t_reject_info *)target->data;

	printf(" reject-with %s", reject_table[reject->with].name);
}

static void REJECT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct ip6t_reject_info *reject
		= (const struct ip6t_reject_info *)target->data;

	printf(" --reject-with %s", reject_table[reject->with].name);
}

static int REJECT_xlate(struct xt_xlate *xl,
			const struct xt_xlate_tg_params *params)
{
	const struct ip6t_reject_info *reject =
		(const struct ip6t_reject_info *)params->target->data;

	if (reject->with == IP6T_ICMP6_PORT_UNREACH)
		xt_xlate_add(xl, "reject");
	else if (reject->with == IP6T_TCP_RESET)
		xt_xlate_add(xl, "reject with %s",
			     reject_table[reject->with].xlate);
	else
		xt_xlate_add(xl, "reject with icmpv6 type %s",
			     reject_table[reject->with].xlate);

	return 1;
}

static struct xtables_target reject_tg6_reg = {
	.name = "REJECT",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size 		= XT_ALIGN(sizeof(struct ip6t_reject_info)),
	.userspacesize 	= XT_ALIGN(sizeof(struct ip6t_reject_info)),
	.help		= REJECT_help,
	.init		= REJECT_init,
	.print		= REJECT_print,
	.save		= REJECT_save,
	.x6_parse	= REJECT_parse,
	.x6_options	= REJECT_opts,
	.xlate		= REJECT_xlate,
};

void _init(void)
{
	xtables_register_target(&reject_tg6_reg);
}
