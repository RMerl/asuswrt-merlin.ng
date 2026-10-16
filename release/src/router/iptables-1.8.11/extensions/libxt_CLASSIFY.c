/*
 * Copyright (c) 2003-2013 Patrick McHardy <kaber@trash.net>
 */

#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_CLASSIFY.h>
#include <linux/pkt_sched.h>

enum {
	O_SET_CLASS = 0,
};

static void
CLASSIFY_help(void)
{
	printf(
"CLASSIFY target options:\n"
"--set-class MAJOR:MINOR    Set skb->priority value (always hexadecimal!)\n");
}

static const struct xt_option_entry CLASSIFY_opts[] = {
	{.name = "set-class", .id = O_SET_CLASS, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND},
	XTOPT_TABLEEND,
};

static int CLASSIFY_string_to_priority(const char *s, unsigned int *p)
{
	unsigned int i, j;

	if (sscanf(s, "%x:%x", &i, &j) != 2)
		return 1;
	
	*p = TC_H_MAKE(i<<16, j);
	return 0;
}

static void CLASSIFY_parse(struct xt_option_call *cb)
{
	struct xt_classify_target_info *clinfo = cb->data;

	xtables_option_parse(cb);
	if (CLASSIFY_string_to_priority(cb->arg, &clinfo->priority))
		xtables_error(PARAMETER_PROBLEM,
			   "Bad class value \"%s\"", cb->arg);
}

static void
CLASSIFY_print_class(unsigned int priority, int numeric)
{
	printf(" %x:%x", TC_H_MAJ(priority)>>16, TC_H_MIN(priority));
}

static void
CLASSIFY_print(const void *ip,
      const struct xt_entry_target *target,
      int numeric)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)target->data;
	printf(" CLASSIFY set");
	CLASSIFY_print_class(clinfo->priority, numeric);
}

static void
CLASSIFY_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)target->data;

	printf(" --set-class %.4x:%.4x",
	       TC_H_MAJ(clinfo->priority)>>16, TC_H_MIN(clinfo->priority));
}

static void
CLASSIFY_arp_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)target->data;

	printf(" --set-class %x:%x",
	       TC_H_MAJ(clinfo->priority)>>16, TC_H_MIN(clinfo->priority));
}

static void
CLASSIFY_arp_print(const void *ip,
      const struct xt_entry_target *target,
      int numeric)
{
	CLASSIFY_arp_save(ip, target);
}

static int CLASSIFY_xlate(struct xt_xlate *xl,
			  const struct xt_xlate_tg_params *params)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)params->target->data;
	__u32 handle = clinfo->priority;

	xt_xlate_add(xl, "meta priority set ");

	switch (handle) {
	case TC_H_ROOT:
		xt_xlate_add(xl, "root");
		break;
	case TC_H_UNSPEC:
		xt_xlate_add(xl, "none");
		break;
	default:
		xt_xlate_add(xl, "%0x:%0x", TC_H_MAJ(handle) >> 16,
			     TC_H_MIN(handle));
		break;
	}

	return 1;
}

static struct xtables_target classify_tg_reg[] = {
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "CLASSIFY",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_classify_target_info)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_classify_target_info)),
		.help		= CLASSIFY_help,
		.print		= CLASSIFY_print,
		.save		= CLASSIFY_save,
		.x6_parse	= CLASSIFY_parse,
		.x6_options	= CLASSIFY_opts,
		.xlate          = CLASSIFY_xlate,
	},
	{
		.family		= NFPROTO_ARP,
		.name		= "CLASSIFY",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_classify_target_info)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_classify_target_info)),
		.help		= CLASSIFY_help,
		.print		= CLASSIFY_arp_print,
		.save		= CLASSIFY_arp_save,
		.x6_parse	= CLASSIFY_parse,
		.x6_options	= CLASSIFY_opts,
		.xlate          = CLASSIFY_xlate,
	}
};

void _init(void)
{
	xtables_register_targets(classify_tg_reg, ARRAY_SIZE(classify_tg_reg));
}
