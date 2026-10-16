/* ebt_arpreply
 *
 * Authors:
 * Grzegorz Borowiak <grzes@gnu.univ.gda.pl>
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 *  August, 2003
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <netinet/ether.h>
#include <linux/netfilter_bridge/ebt_arpreply.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

enum {
	O_MAC,
	O_TARGET,
};

static const struct xt_option_entry brarpreply_opts[] = {
	{ .name = "arpreply-mac" ,    .id = O_MAC, .type = XTTYPE_ETHERMAC,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_arpreply_info, mac) },
	{ .name = "arpreply-target" , .id = O_TARGET, .type = XTTYPE_STRING },
	XTOPT_TABLEEND,
};

static void brarpreply_print_help(void)
{
	printf(
	"arpreply target options:\n"
	" --arpreply-mac address           : source MAC of generated reply\n"
	" --arpreply-target target         : ACCEPT, DROP, RETURN or CONTINUE\n"
	"                                    (standard target is DROP)\n");
}

static void brarpreply_init(struct xt_entry_target *target)
{
	struct ebt_arpreply_info *replyinfo = (void *)target->data;

	replyinfo->target = EBT_DROP;
}

static void brarpreply_parse(struct xt_option_call *cb)
{
	struct ebt_arpreply_info *replyinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->entry->id == O_TARGET &&
	    ebt_fill_target(cb->arg, (unsigned int *)&replyinfo->target))
		xtables_error(PARAMETER_PROBLEM,
			      "Illegal --arpreply-target target");
}

static void brarpreply_print(const void *ip, const struct xt_entry_target *t, int numeric)
{
	struct ebt_arpreply_info *replyinfo = (void *)t->data;

	printf("--arpreply-mac ");
	xtables_print_mac(replyinfo->mac);
	if (replyinfo->target == EBT_DROP)
		return;
	printf(" --arpreply-target %s", ebt_target_name(replyinfo->target));
}

static struct xtables_target arpreply_target = {
	.name		= "arpreply",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.init		= brarpreply_init,
	.size		= XT_ALIGN(sizeof(struct ebt_arpreply_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_arpreply_info)),
	.help		= brarpreply_print_help,
	.x6_parse	= brarpreply_parse,
	.print		= brarpreply_print,
	.x6_options	= brarpreply_opts,
};

void _init(void)
{
	xtables_register_target(&arpreply_target);
}
