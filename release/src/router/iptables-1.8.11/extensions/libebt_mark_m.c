/* ebt_mark_m
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002
 *
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat in 2015.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_mark_m.h>

enum {
	O_MARK = 0,
};

static const struct xt_option_entry brmark_m_opts[] = {
	{ .name = "mark", .id = O_MARK, .type = XTTYPE_STRING,
	  .flags = XTOPT_INVERT | XTOPT_MAND },
	XTOPT_TABLEEND,
};

static void brmark_m_print_help(void)
{
	printf(
"mark option:\n"
"[!] --mark    [value][/mask]: Match nfmask value (see man page)\n");
}

static void brmark_m_parse(struct xt_option_call *cb)
{
	struct ebt_mark_m_info *info = cb->data;
	char *end;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_MARK:
		info->invert = cb->invert;
		info->mark = strtoul(cb->arg, &end, 0);
		info->bitmask = EBT_MARK_AND;
		if (*end == '/') {
			if (end == cb->arg)
				info->bitmask = EBT_MARK_OR;
			info->mask = strtoul(end+1, &end, 0);
		} else {
			info->mask = UINT32_MAX;
		}
		if (*end != '\0' || end == cb->arg)
			xtables_error(PARAMETER_PROBLEM, "Bad mark value '%s'",
				      cb->arg);
		break;
	}
}

static void brmark_m_print(const void *ip, const struct xt_entry_match *match,
			   int numeric)
{
	struct ebt_mark_m_info *info = (struct ebt_mark_m_info *)match->data;

	if (info->invert)
		printf("! ");
	printf("--mark ");
	if (info->bitmask == EBT_MARK_OR)
		printf("/0x%lx ", info->mask);
	else if (info->mask != 0xffffffff)
		printf("0x%lx/0x%lx ", info->mark, info->mask);
	else
		printf("0x%lx ", info->mark);
}

static int brmark_m_xlate(struct xt_xlate *xl,
			  const struct xt_xlate_mt_params *params)
{
	const struct ebt_mark_m_info *info = (const void*)params->match->data;
	enum xt_op op = XT_OP_EQ;

	if (info->invert)
		op = XT_OP_NEQ;

	xt_xlate_add(xl, "meta mark ");

	if (info->bitmask == EBT_MARK_OR) {
		xt_xlate_add(xl, "and 0x%x %s0 ", (uint32_t)info->mask,
			     info->invert ? "" : "!= ");
	} else if (info->mask != UINT32_MAX) {
		xt_xlate_add(xl, "and 0x%x %s0x%x ", (uint32_t)info->mask,
			   op == XT_OP_EQ ? "" : "!= ", (uint32_t)info->mark);
	} else {
		xt_xlate_add(xl, "%s0x%x ",
			   op == XT_OP_EQ ? "" : "!= ", (uint32_t)info->mark);
	}

	return 1;
}
static struct xtables_match brmark_m_match = {
	.name		= "mark_m",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_mark_m_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_mark_m_info)),
	.help		= brmark_m_print_help,
	.x6_parse	= brmark_m_parse,
	.print		= brmark_m_print,
	.xlate		= brmark_m_xlate,
	.x6_options	= brmark_m_opts,
};

void _init(void)
{
	xtables_register_match(&brmark_m_match);
}
