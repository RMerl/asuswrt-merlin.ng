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
	O_SRC,
	O_TARGET,
	O_ARP,
};

static const struct xt_option_entry brsnat_opts[] =
{
	{ .name = "to-source", .id = O_SRC, .type = XTTYPE_ETHERMAC,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nat_info, mac) },
	{ .name = "to-src",    .id = O_SRC, .type = XTTYPE_ETHERMAC,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nat_info, mac) },
	{ .name = "snat-target", .id = O_TARGET, .type = XTTYPE_STRING },
	{ .name = "snat-arp", .id = O_ARP, .type = XTTYPE_NONE },
	XTOPT_TABLEEND,
};

static void brsnat_print_help(void)
{
	printf(
	"snat options:\n"
	" --to-src address       : MAC address to map source to\n"
	" --snat-target target   : ACCEPT, DROP, RETURN or CONTINUE\n"
	" --snat-arp             : also change src address in arp msg\n");
}

static void brsnat_init(struct xt_entry_target *target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	natinfo->target = EBT_ACCEPT;
}

static void brsnat_parse(struct xt_option_call *cb)
{
	struct ebt_nat_info *natinfo = cb->data;
	unsigned int tmp;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TARGET:
		if (ebt_fill_target(cb->arg, &tmp))
			xtables_error(PARAMETER_PROBLEM,
				      "Illegal --snat-target target");
		natinfo->target &= ~EBT_VERDICT_BITS;
		natinfo->target |= tmp & EBT_VERDICT_BITS;
		break;
	case O_ARP:
		natinfo->target ^= NAT_ARP_BIT;
		break;
	}
}

static void brsnat_final_check(struct xt_fcheck_call *fc)
{
	if (!fc->xflags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brsnat_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	printf("--to-src ");
	xtables_print_mac(natinfo->mac);
	if (!(natinfo->target&NAT_ARP_BIT))
		printf(" --snat-arp");
	printf(" --snat-target %s", ebt_target_name((natinfo->target|~EBT_VERDICT_BITS)));
}

static const char* brsnat_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brsnat_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nat_info *natinfo = (const void*)params->target->data;

	xt_xlate_add(xl, "ether saddr set %s ",
		     ether_ntoa((struct ether_addr *)natinfo->mac));

	/* NAT_ARP_BIT set -> no arp mangling, not set -> arp mangling (yes, its inverted) */
	if (!(natinfo->target&NAT_ARP_BIT))
		return 0;

	xt_xlate_add(xl, "%s ", brsnat_verdict(natinfo->target | ~EBT_VERDICT_BITS));
	return 1;
}

static struct xtables_target brsnat_target =
{
	.name		= "snat",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size           = XT_ALIGN(sizeof(struct ebt_nat_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nat_info)),
	.help		= brsnat_print_help,
	.init		= brsnat_init,
	.x6_parse	= brsnat_parse,
	.x6_fcheck	= brsnat_final_check,
	.print		= brsnat_print,
	.xlate		= brsnat_xlate,
	.x6_options	= brsnat_opts,
};

void _init(void)
{
	xtables_register_target(&brsnat_target);
}
