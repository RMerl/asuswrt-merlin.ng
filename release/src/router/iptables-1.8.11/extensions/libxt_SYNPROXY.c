
/*
 * Copyright (c) 2013 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_SYNPROXY.h>

enum {
	O_SACK_PERM = 0,
	O_TIMESTAMP,
	O_WSCALE,
	O_MSS,
	O_ECN,
};

static void SYNPROXY_help(void)
{
	printf(
"SYNPROXY target options:\n"
"  --sack-perm                        Set SACK_PERM\n"
"  --timestamp                        Set TIMESTAMP\n"
"  --wscale value                     Set window scaling factor\n"
"  --mss value                        Set MSS value\n"
"  --ecn                              Set ECN\n");
}

static const struct xt_option_entry SYNPROXY_opts[] = {
	{.name = "sack-perm", .id = O_SACK_PERM, .type = XTTYPE_NONE, },
	{.name = "timestamp", .id = O_TIMESTAMP, .type = XTTYPE_NONE, },
	{.name = "wscale",    .id = O_WSCALE,    .type = XTTYPE_UINT32, },
	{.name = "mss",       .id = O_MSS,       .type = XTTYPE_UINT32, },
	{.name = "ecn",       .id = O_ECN,	 .type = XTTYPE_NONE, },
	XTOPT_TABLEEND,
};

static void SYNPROXY_parse(struct xt_option_call *cb)
{
	struct xt_synproxy_info *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SACK_PERM:
		info->options |= XT_SYNPROXY_OPT_SACK_PERM;
		break;
	case O_TIMESTAMP:
		info->options |= XT_SYNPROXY_OPT_TIMESTAMP;
		break;
	case O_WSCALE:
		info->options |= XT_SYNPROXY_OPT_WSCALE;
		info->wscale = cb->val.u32;
		break;
	case O_MSS:
		info->options |= XT_SYNPROXY_OPT_MSS;
		info->mss = cb->val.u32;
		break;
	case O_ECN:
		info->options |= XT_SYNPROXY_OPT_ECN;
		break;
	}
}

static void SYNPROXY_check(struct xt_fcheck_call *cb)
{
}

static void SYNPROXY_print(const void *ip, const struct xt_entry_target *target,
                           int numeric)
{
	const struct xt_synproxy_info *info =
		(const struct xt_synproxy_info *)target->data;

	printf(" SYNPROXY ");
	if (info->options & XT_SYNPROXY_OPT_SACK_PERM)
		printf("sack-perm ");
	if (info->options & XT_SYNPROXY_OPT_TIMESTAMP)
		printf("timestamp ");
	if (info->options & XT_SYNPROXY_OPT_WSCALE)
		printf("wscale %u ", info->wscale);
	if (info->options & XT_SYNPROXY_OPT_MSS)
		printf("mss %u ", info->mss);
	if (info->options & XT_SYNPROXY_OPT_ECN)
		printf("ecn ");
}

static void SYNPROXY_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_synproxy_info *info =
		(const struct xt_synproxy_info *)target->data;

	if (info->options & XT_SYNPROXY_OPT_SACK_PERM)
		printf(" --sack-perm");
	if (info->options & XT_SYNPROXY_OPT_TIMESTAMP)
		printf(" --timestamp");
	if (info->options & XT_SYNPROXY_OPT_WSCALE)
		printf(" --wscale %u", info->wscale);
	if (info->options & XT_SYNPROXY_OPT_MSS)
		printf(" --mss %u", info->mss);
	if (info->options & XT_SYNPROXY_OPT_ECN)
		printf(" --ecn");
}

static int SYNPROXY_xlate(struct xt_xlate *xl,
		          const struct xt_xlate_tg_params *params)
{
	const struct xt_synproxy_info *info =
		(const struct xt_synproxy_info *)params->target->data;

	xt_xlate_add(xl, "synproxy ");

	if (info->options & XT_SYNPROXY_OPT_SACK_PERM)
		xt_xlate_add(xl, "sack-perm ");
	if (info->options & XT_SYNPROXY_OPT_TIMESTAMP)
		xt_xlate_add(xl, "timestamp ");
	if (info->options & XT_SYNPROXY_OPT_WSCALE)
		xt_xlate_add(xl, "wscale %u ", info->wscale);
	if (info->options & XT_SYNPROXY_OPT_MSS)
		xt_xlate_add(xl, "mss %u ", info->mss);
	if (info->options & XT_SYNPROXY_OPT_ECN)
		xt_xlate_add(xl, "ecn ");

	return 1;
}

static struct xtables_target synproxy_tg_reg = {
	.family        = NFPROTO_UNSPEC,
	.name          = "SYNPROXY",
	.version       = XTABLES_VERSION,
	.revision      = 0,
	.size          = XT_ALIGN(sizeof(struct xt_synproxy_info)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_synproxy_info)),
	.help          = SYNPROXY_help,
	.print         = SYNPROXY_print,
	.save          = SYNPROXY_save,
	.x6_parse      = SYNPROXY_parse,
	.x6_fcheck     = SYNPROXY_check,
	.x6_options    = SYNPROXY_opts,
	.xlate         = SYNPROXY_xlate,
};

void _init(void)
{
	xtables_register_target(&synproxy_tg_reg);
}
