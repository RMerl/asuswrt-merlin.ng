/*
 * Shared library add-on to iptables to add IPv6 flowlabel match support
 *
 * BRCM, Feb, 1. 2019.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <stdbool.h>
#include <xtables.h>
#include <linux/types.h>

#include "linux/netfilter/xt_flowlabel.h"

enum {
	O_FLOWLABEL = 1 << 0,
};


static const struct xt_option_entry flowlabel_mt_opts[] = {
	{.name = "flowlabel", .id = O_FLOWLABEL, .type = XTTYPE_UINT32,
	 .flags = XTOPT_INVERT | XTOPT_MAND, .max = XT_FLOWLABEL_MAX},
	XTOPT_TABLEEND,
};

static void flowlabel_mt_help(void)
{
	printf(
"flowlabel match options:\n"
"[!] --flowlabel value    Match IPv6 flowlabel field value\n"
"                         Max value: 0x%05X\n", XT_FLOWLABEL_MAX);
}

static void flowlabel_mt_parse(struct xt_option_call *cb)
{
	struct xt_flowlabel_info *info = cb->data;

	xtables_option_parse(cb);
	info->flowlabel = cpu_to_be32(cb->val.u32);
	if (cb->invert)
		info->invert = true;
}

static void flowlabel_mt_check(struct xt_fcheck_call *cb)
{
	if (cb->xflags == 0)
		xtables_error(PARAMETER_PROBLEM,
		      "flowlabel match: Parameter --flowlabel is required");
}

static int flowlabel_mt_xlate(struct xt_xlate *xl,
			const struct xt_xlate_mt_params *params)
{
	const struct xt_flowlabel_info *info =
		(const struct xt_flowlabel_info *)params->match->data;

	xt_xlate_add(xl, "flowlabel %s0x%05X", info->invert ? "!= " : "",
		     be32_to_cpu(info->flowlabel));

	return 1;
}

static void flowlabel_mt_print(const void *ip, const struct xt_entry_match *match,
                         int numeric)
{
	const struct xt_flowlabel_info *info = (const void *)match->data;

	printf(" flowlabel match ");
	if (info->invert)
		printf("!");

	printf("0x%05X", be32_to_cpu(info->flowlabel));
}

static void flowlabel_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_flowlabel_info *info = (const void *)match->data;

	if (info->invert)
		printf(" !");
	printf(" --flowlabel 0x%05X", be32_to_cpu(info->flowlabel));
}

static struct xtables_match flowlabel_mt_reg = {
	.family        = NFPROTO_IPV6,
	.name          = "flowlabel",
	.version       = XTABLES_VERSION,
	.size          = XT_ALIGN(sizeof(struct xt_flowlabel_info)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_flowlabel_info)),
	.help          = flowlabel_mt_help,
	.print         = flowlabel_mt_print,
	.save          = flowlabel_mt_save,
	.x6_parse      = flowlabel_mt_parse,
	.x6_fcheck     = flowlabel_mt_check,
	.xlate         = flowlabel_mt_xlate,
	.x6_options    = flowlabel_mt_opts,

};

void _init(void)
{
	xtables_register_match(&flowlabel_mt_reg);
}
