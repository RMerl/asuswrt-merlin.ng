/* ebt_nat
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * June, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

enum {
	O_DST,
	O_TARGET,
};

static const struct xt_option_entry brdnat_opts[] =
{
	{ .name = "to-destination", .id = O_DST, .type = XTTYPE_ETHERMAC,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nat_info, mac) },
	{ .name = "to-dst"        , .id = O_DST, .type = XTTYPE_ETHERMAC,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nat_info, mac) },
	{ .name = "dnat-target"   , .id = O_TARGET, .type = XTTYPE_STRING },
	XTOPT_TABLEEND,
};

static void brdnat_print_help(void)
{
	printf(
	"dnat options:\n"
	" --to-dst address       : MAC address to map destination to\n"
	" --dnat-target target   : ACCEPT, DROP, RETURN or CONTINUE\n"
	"                          (standard target is ACCEPT)\n");
}

static void brdnat_init(struct xt_entry_target *target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	natinfo->target = EBT_ACCEPT;
}

static void brdnat_parse(struct xt_option_call *cb)
{
	struct ebt_nat_info *natinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->entry->id == O_TARGET &&
	    ebt_fill_target(cb->arg, (unsigned int *)&natinfo->target))
		xtables_error(PARAMETER_PROBLEM,
			      "Illegal --dnat-target target");
}

static void brdnat_final_check(struct xt_fcheck_call *fc)
{
	if (!fc->xflags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brdnat_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	printf("--to-dst ");
	xtables_print_mac(natinfo->mac);
	printf(" --dnat-target %s", ebt_target_name(natinfo->target));
}

static const char* brdnat_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brdnat_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nat_info *natinfo = (const void*)params->target->data;

	xt_xlate_add(xl, "ether daddr set %s %s ",
		     ether_ntoa((struct ether_addr *)natinfo->mac),
		     brdnat_verdict(natinfo->target));

	return 1;
}

static struct xtables_target brdnat_target =
{
	.name		= "dnat",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size           = XT_ALIGN(sizeof(struct ebt_nat_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nat_info)),
	.help		= brdnat_print_help,
	.init		= brdnat_init,
	.x6_parse	= brdnat_parse,
	.x6_fcheck	= brdnat_final_check,
	.print		= brdnat_print,
	.xlate		= brdnat_xlate,
	.x6_options	= brdnat_opts,
};

void _init(void)
{
	xtables_register_target(&brdnat_target);
}
