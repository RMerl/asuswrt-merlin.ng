/* ebt_redirect
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * April, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_redirect.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

enum {
	O_TARGET,
};

static const struct xt_option_entry brredir_opts[] =
{
	{ .name = "redirect-target", .id = O_TARGET, .type = XTTYPE_STRING },
	XTOPT_TABLEEND,
};

static void brredir_print_help(void)
{
	printf(
	"redirect option:\n"
	" --redirect-target target   : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void brredir_init(struct xt_entry_target *target)
{
	struct ebt_redirect_info *redirectinfo =
	   (struct ebt_redirect_info *)target->data;

	redirectinfo->target = EBT_ACCEPT;
}

static void brredir_parse(struct xt_option_call *cb)
{
	struct ebt_redirect_info *redirectinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->entry->id == O_TARGET &&
	    ebt_fill_target(cb->arg, (unsigned int *)&redirectinfo->target))
		xtables_error(PARAMETER_PROBLEM,
			      "Illegal --redirect-target target");
}

static void brredir_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_redirect_info *redirectinfo =
	   (struct ebt_redirect_info *)target->data;

	if (redirectinfo->target == EBT_ACCEPT)
		return;
	printf("--redirect-target %s", ebt_target_name(redirectinfo->target));
}

static const char* brredir_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brredir_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_redirect_info *red = (const void*)params->target->data;

	xt_xlate_add(xl, "meta pkttype set host");
	if (red->target != EBT_CONTINUE)
		xt_xlate_add(xl, " %s ", brredir_verdict(red->target));
	return 1;
}

static struct xtables_target brredirect_target = {
	.name		= "redirect",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_redirect_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_redirect_info)),
	.help		= brredir_print_help,
	.init		= brredir_init,
	.x6_parse	= brredir_parse,
	.print		= brredir_print,
	.xlate		= brredir_xlate,
	.x6_options	= brredir_opts,
};

void _init(void)
{
	xtables_register_target(&brredirect_target);
}
