/* ebt_vlan
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 * Nick Fedchik <nick@fedchik.org.ua>
 * June, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xtables.h>
#include <netinet/if_ether.h>
#include <linux/netfilter_bridge/ebt_vlan.h>
#include <linux/if_ether.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

static const struct xt_option_entry brvlan_opts[] =
{
	{ .name = "vlan-id", .id = EBT_VLAN_ID, .type = XTTYPE_UINT16,
	  .max = 4094, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_vlan_info, id) },
	{ .name = "vlan-prio", .id = EBT_VLAN_PRIO, .type = XTTYPE_UINT8,
	  .max = 7, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_vlan_info, prio) },
	{ .name = "vlan-encap", .id = EBT_VLAN_ENCAP, .type = XTTYPE_STRING,
	  .flags = XTOPT_INVERT },
	XTOPT_TABLEEND,
};

static void brvlan_print_help(void)
{
	printf(
"vlan options:\n"
"[!] --vlan-id id       : vlan-tagged frame identifier, 0,1-4096 (integer)\n"
"[!] --vlan-prio prio   : Priority-tagged frame's user priority, 0-7 (integer)\n"
"[!] --vlan-encap encap : Encapsulated frame protocol (hexadecimal or name)\n");
}

static void brvlan_parse(struct xt_option_call *cb)
{
	struct ebt_vlan_info *vlaninfo = cb->data;
	struct xt_ethertypeent *ethent;
	char *end;

	xtables_option_parse(cb);

	vlaninfo->bitmask |= cb->entry->id;
	if (cb->invert)
		vlaninfo->invflags |= cb->entry->id;

	if (cb->entry->id == EBT_VLAN_ENCAP) {
		vlaninfo->encap = strtoul(cb->arg, &end, 16);
		if (*end != '\0') {
			ethent = xtables_getethertypebyname(cb->arg);
			if (ethent == NULL)
				xtables_error(PARAMETER_PROBLEM,
					      "Unknown --vlan-encap value ('%s')",
					      cb->arg);
			vlaninfo->encap = ethent->e_ethertype;
		}
		if (vlaninfo->encap < ETH_ZLEN)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid --vlan-encap range ('%s')",
				      cb->arg);
		vlaninfo->encap = htons(vlaninfo->encap);
	}
}

static void brvlan_print(const void *ip, const struct xt_entry_match *match,
			 int numeric)
{
	struct ebt_vlan_info *vlaninfo = (struct ebt_vlan_info *) match->data;

	if (vlaninfo->bitmask & EBT_VLAN_ID) {
		printf("%s--vlan-id %d ",
		       (vlaninfo->invflags & EBT_VLAN_ID) ? "! " : "",
		       vlaninfo->id);
	}
	if (vlaninfo->bitmask & EBT_VLAN_PRIO) {
		printf("%s--vlan-prio %d ",
		       (vlaninfo->invflags & EBT_VLAN_PRIO) ? "! " : "",
		       vlaninfo->prio);
	}
	if (vlaninfo->bitmask & EBT_VLAN_ENCAP) {
		printf("%s--vlan-encap %4.4X ",
		       (vlaninfo->invflags & EBT_VLAN_ENCAP) ? "! " : "",
		       ntohs(vlaninfo->encap));
	}
}

static int brvlan_xlate(struct xt_xlate *xl,
			const struct xt_xlate_mt_params *params)
{
	const struct ebt_vlan_info *vlaninfo = (const void*)params->match->data;

	if (vlaninfo->bitmask & EBT_VLAN_ID)
		xt_xlate_add(xl, "vlan id %s%d ", (vlaninfo->invflags & EBT_VLAN_ID) ? "!= " : "", vlaninfo->id);

	if (vlaninfo->bitmask & EBT_VLAN_PRIO)
		xt_xlate_add(xl, "vlan pcp %s%d ", (vlaninfo->invflags & EBT_VLAN_PRIO) ? "!= " : "", vlaninfo->prio);

	if (vlaninfo->bitmask & EBT_VLAN_ENCAP)
		xt_xlate_add(xl, "vlan type %s0x%4.4x ", (vlaninfo->invflags & EBT_VLAN_ENCAP) ? "!= " : "", ntohs(vlaninfo->encap));

	return 1;
}

static struct xtables_match brvlan_match = {
	.name		= "vlan",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_vlan_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_vlan_info)),
	.help		= brvlan_print_help,
	.x6_parse	= brvlan_parse,
	.print		= brvlan_print,
	.xlate		= brvlan_xlate,
	.x6_options	= brvlan_opts,
};

void _init(void)
{
	xtables_register_match(&brvlan_match);
}
