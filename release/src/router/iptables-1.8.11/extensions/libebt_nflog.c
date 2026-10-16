/* ebt_nflog
 *
 * Authors:
 * Peter Warasin <peter@endian.com>
 *
 *  February, 2008
 *
 * Based on:
 *  ebt_ulog.c, (C) 2004, Bart De Schuymer <bdschuym@pandora.be>
 *  libxt_NFLOG.c
 *
 * Adapted to libxtables for ebtables-compat in 2015 by
 * Arturo Borrero Gonzalez <arturo@debian.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"
#include <linux/netfilter_bridge/ebt_nflog.h>

enum {
	O_GROUP	= 0,
	O_PREFIX,
	O_RANGE,
	O_THRESHOLD,
	O_NFLOG,
};

static const struct xt_option_entry brnflog_opts[] = {
	{ .name = "nflog-group",     .id = O_GROUP, .type = XTTYPE_UINT16,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nflog_info, group) },
	{ .name = "nflog-prefix",    .id = O_PREFIX, .type = XTTYPE_STRING,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nflog_info, prefix) },
	{ .name = "nflog-range",     .id = O_RANGE, .type = XTTYPE_UINT32,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nflog_info, len) },
	{ .name = "nflog-threshold", .id = O_THRESHOLD, .type = XTTYPE_UINT16,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_nflog_info, threshold) },
	{ .name = "nflog",           .id = O_NFLOG, .type = XTTYPE_NONE },
	XTOPT_TABLEEND,
};

static void brnflog_help(void)
{
	printf("nflog options:\n"
	       "--nflog               : use the default nflog parameters\n"
	       "--nflog-prefix prefix : Prefix string for log message\n"
	       "--nflog-group group   : NETLINK group used for logging\n"
	       "--nflog-range range   : Number of byte to copy\n"
	       "--nflog-threshold     : Message threshold of"
	       "in-kernel queue\n");
}

static void brnflog_init(struct xt_entry_target *t)
{
	struct ebt_nflog_info *info = (struct ebt_nflog_info *)t->data;

	info->prefix[0]	= '\0';
	info->group	= EBT_NFLOG_DEFAULT_GROUP;
	info->threshold = EBT_NFLOG_DEFAULT_THRESHOLD;
}

static void
brnflog_print(const void *ip, const struct xt_entry_target *target,
	      int numeric)
{
	struct ebt_nflog_info *info = (struct ebt_nflog_info *)target->data;

	if (info->prefix[0] != '\0')
		printf("--nflog-prefix \"%s\" ", info->prefix);
	if (info->group)
		printf("--nflog-group %d ", info->group);
	if (info->len)
		printf("--nflog-range %d ", info->len);
	if (info->threshold != EBT_NFLOG_DEFAULT_THRESHOLD)
		printf("--nflog-threshold %d ", info->threshold);
}

static int brnflog_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nflog_info *info = (void *)params->target->data;

	xt_xlate_add(xl, "log ");
	if (info->prefix[0] != '\0')
		xt_xlate_add(xl, "prefix \"%s\" ", info->prefix);

	xt_xlate_add(xl, "group %u ", info->group);

	if (info->len)
		xt_xlate_add(xl, "snaplen %u ", info->len);
	if (info->threshold != EBT_NFLOG_DEFAULT_THRESHOLD)
		xt_xlate_add(xl, "queue-threshold %u ", info->threshold);

	return 1;
}

static struct xtables_target brnflog_watcher = {
	.name		= "nflog",
	.revision	= 0,
	.ext_flags	= XTABLES_EXT_WATCHER,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_nflog_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nflog_info)),
	.init		= brnflog_init,
	.help		= brnflog_help,
	.x6_parse	= xtables_option_parse,
	.print		= brnflog_print,
	.xlate		= brnflog_xlate,
	.x6_options	= brnflog_opts,
};

void _init(void)
{
	xtables_register_target(&brnflog_watcher);
}
