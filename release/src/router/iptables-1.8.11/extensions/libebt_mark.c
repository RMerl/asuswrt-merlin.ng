/* ebt_mark
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002, September 2006
 *
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat in 2015.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

enum {
	O_SET_MARK = 0,
	O_AND_MARK,
	O_OR_MARK,
	O_XOR_MARK,
	O_MARK_TARGET,
	F_SET_MARK  = 1 << O_SET_MARK,
	F_AND_MARK  = 1 << O_AND_MARK,
	F_OR_MARK   = 1 << O_OR_MARK,
	F_XOR_MARK  = 1 << O_XOR_MARK,
	F_ANY       = F_SET_MARK | F_AND_MARK | F_OR_MARK | F_XOR_MARK,
};

static const struct xt_option_entry brmark_opts[] = {
	{ .name = "mark-target",.id = O_MARK_TARGET, .type = XTTYPE_STRING },
	/* an oldtime messup, we should have always used the scheme
	 * <extension-name>-<option> */
	{ .name = "set-mark",	.id = O_SET_MARK, .type = XTTYPE_UINT32,
	  .excl = F_ANY },
	{ .name = "mark-set",	.id = O_SET_MARK, .type = XTTYPE_UINT32,
	  .excl = F_ANY },
	{ .name = "mark-or",	.id = O_OR_MARK, .type = XTTYPE_UINT32,
	  .excl = F_ANY },
	{ .name = "mark-and",	.id = O_AND_MARK, .type = XTTYPE_UINT32,
	  .excl = F_ANY },
	{ .name = "mark-xor",	.id = O_XOR_MARK, .type = XTTYPE_UINT32,
	  .excl = F_ANY },
	XTOPT_TABLEEND,
};

static void brmark_print_help(void)
{
	printf(
	"mark target options:\n"
	" --mark-set value     : Set nfmark value\n"
	" --mark-or  value     : Or nfmark with value (nfmark |= value)\n"
	" --mark-and value     : And nfmark with value (nfmark &= value)\n"
	" --mark-xor value     : Xor nfmark with value (nfmark ^= value)\n"
	" --mark-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void brmark_init(struct xt_entry_target *target)
{
	struct ebt_mark_t_info *info = (struct ebt_mark_t_info *)target->data;

	info->target = EBT_ACCEPT;
	info->mark = 0;
}

static void brmark_parse(struct xt_option_call *cb)
{
	static const unsigned long target_orval[] = {
		[O_SET_MARK]	= MARK_SET_VALUE,
		[O_AND_MARK]	= MARK_AND_VALUE,
		[O_OR_MARK]	= MARK_OR_VALUE,
		[O_XOR_MARK]	= MARK_XOR_VALUE,
	};
	struct ebt_mark_t_info *info = cb->data;
	unsigned int tmp;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_MARK_TARGET:
		if (ebt_fill_target(cb->arg, &tmp))
			xtables_error(PARAMETER_PROBLEM,
				      "Illegal --mark-target target");
		/* the 4 lsb are left to designate the target */
		info->target = (info->target & ~EBT_VERDICT_BITS) |
			       (tmp & EBT_VERDICT_BITS);
		return;
	case O_SET_MARK:
	case O_OR_MARK:
	case O_AND_MARK:
	case O_XOR_MARK:
		break;
	default:
		return;
	}
	/* mutual code */
	info->mark = cb->val.u32;
	info->target = (info->target & EBT_VERDICT_BITS) |
		       target_orval[cb->entry->id];
}

static void brmark_print(const void *ip, const struct xt_entry_target *target,
			 int numeric)
{
	struct ebt_mark_t_info *info = (struct ebt_mark_t_info *)target->data;
	int tmp;

	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp == MARK_SET_VALUE)
		printf("--mark-set");
	else if (tmp == MARK_OR_VALUE)
		printf("--mark-or");
	else if (tmp == MARK_XOR_VALUE)
		printf("--mark-xor");
	else if (tmp == MARK_AND_VALUE)
		printf("--mark-and");
	else
		xtables_error(PARAMETER_PROBLEM, "Unknown mark action");

	printf(" 0x%lx", info->mark);
	tmp = info->target | ~EBT_VERDICT_BITS;
	printf(" --mark-target %s", ebt_target_name(tmp));
}

static void brmark_final_check(struct xt_fcheck_call *fc)
{
	if (!fc->xflags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify some option");
}

static const char* brmark_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brmark_xlate(struct xt_xlate *xl,
			const struct xt_xlate_tg_params *params)
{
	const struct ebt_mark_t_info *info = (const void*)params->target->data;
	int tmp;

	tmp = info->target & ~EBT_VERDICT_BITS;

	xt_xlate_add(xl, "meta mark set ");

	switch (tmp) {
	case MARK_SET_VALUE:
		break;
	case MARK_OR_VALUE:
		xt_xlate_add(xl, "meta mark or ");
		break;
	case MARK_XOR_VALUE:
		xt_xlate_add(xl, "meta mark xor ");
		break;
	case MARK_AND_VALUE:
		xt_xlate_add(xl, "meta mark and ");
		break;
	default:
		return 0;
	}

	tmp = info->target | ~EBT_VERDICT_BITS;
	xt_xlate_add(xl, "0x%lx %s ", info->mark, brmark_verdict(tmp));
	return 1;
}

static struct xtables_target brmark_target = {
	.name		= "mark",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_mark_t_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_mark_t_info)),
	.help		= brmark_print_help,
	.init		= brmark_init,
	.x6_parse	= brmark_parse,
	.x6_fcheck	= brmark_final_check,
	.print		= brmark_print,
	.xlate		= brmark_xlate,
	.x6_options	= brmark_opts,
};

void _init(void)
{
	xtables_register_target(&brmark_target);
}
