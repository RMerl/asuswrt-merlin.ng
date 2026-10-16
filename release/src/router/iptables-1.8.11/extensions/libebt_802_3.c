/* 802_3
 *
 * Author:
 * Chris Vitale <csv@bluetail.com>
 *
 * May 2003
 *
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_802_3.h>

static const struct xt_option_entry br802_3_opts[] =
{
	{ .name = "802_3-sap", .id = EBT_802_3_SAP,
	  .type = XTTYPE_UINT8, .base = 16,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_802_3_info, sap) },
	{ .name = "802_3-type", .id = EBT_802_3_TYPE,
	  .type = XTTYPE_UINT16, .base = 16,
	  .flags = XTOPT_INVERT | XTOPT_PUT | XTOPT_NBO,
	  XTOPT_POINTER(struct ebt_802_3_info, type) },
	XTOPT_TABLEEND,
};

static void br802_3_print_help(void)
{
	printf(
"802_3 options:\n"
"[!] --802_3-sap protocol       : 802.3 DSAP/SSAP- 1 byte value (hex)\n"
"  DSAP and SSAP are always the same.  One SAP applies to both fields\n"
"[!] --802_3-type protocol      : 802.3 SNAP Type- 2 byte value (hex)\n"
"  Type implies SAP value 0xaa\n");
}

static void br802_3_parse(struct xt_option_call *cb)
{
	struct ebt_802_3_info *info = cb->data;

	xtables_option_parse(cb);
	info->bitmask |= cb->entry->id;
	if (cb->invert)
		info->invflags |= cb->entry->id;
}

static void br802_3_print(const void *ip, const struct xt_entry_match *match,
			  int numeric)
{
	struct ebt_802_3_info *info = (struct ebt_802_3_info *)match->data;

	if (info->bitmask & EBT_802_3_SAP) {
		if (info->invflags & EBT_802_3_SAP)
			printf("! ");
		printf("--802_3-sap 0x%.2x ", info->sap);
	}
	if (info->bitmask & EBT_802_3_TYPE) {
		if (info->invflags & EBT_802_3_TYPE)
			printf("! ");
		printf("--802_3-type 0x%.4x ", ntohs(info->type));
	}
}

static struct xtables_match br802_3_match =
{
	.name		= "802_3",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_802_3_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_802_3_info)),
	.help		= br802_3_print_help,
	.x6_parse	= br802_3_parse,
	.print		= br802_3_print,
	.x6_options	= br802_3_opts,
};

void _init(void)
{
	xtables_register_match(&br802_3_match);
}
