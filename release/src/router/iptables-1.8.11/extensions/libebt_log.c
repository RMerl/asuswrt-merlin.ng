/*
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Giuseppe Longo <giuseppelng@gmail.com> adapted the original code to the
 * xtables-compat environment in 2015.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_log.h>

#define LOG_DEFAULT_LEVEL LOG_INFO

struct code {
	char *c_name;
	int c_val;
};

static struct code eight_priority[] = {
	{ "emerg", LOG_EMERG },
	{ "alert", LOG_ALERT },
	{ "crit", LOG_CRIT },
	{ "error", LOG_ERR },
	{ "warning", LOG_WARNING },
	{ "notice", LOG_NOTICE },
	{ "info", LOG_INFO },
	{ "debug", LOG_DEBUG }
};

enum {
	/* first three must correspond with bit pos in respective EBT_LOG_* */
	O_LOG_IP = 0,
	O_LOG_ARP = 1,
	O_LOG_IP6 = 3,
	O_LOG_PREFIX,
	O_LOG_LEVEL,
	O_LOG_LOG,
};

static const struct xt_option_entry brlog_opts[] = {
	{ .name = "log-prefix", .id = O_LOG_PREFIX, .type = XTTYPE_STRING,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_log_info, prefix) },
	{ .name = "log-level", .id = O_LOG_LEVEL, .type = XTTYPE_SYSLOGLEVEL,
	  .flags = XTOPT_PUT, XTOPT_POINTER(struct ebt_log_info, loglevel) },
	{ .name = "log-arp",	.id = O_LOG_ARP,	.type = XTTYPE_NONE },
	{ .name = "log-ip",	.id = O_LOG_IP,		.type = XTTYPE_NONE },
	{ .name = "log",	.id = O_LOG_LOG,	.type = XTTYPE_NONE },
	{ .name = "log-ip6",	.id = O_LOG_IP6,	.type = XTTYPE_NONE },
	XTOPT_TABLEEND,
};

static void brlog_help(void)
{
	int i;

	printf(
"log options:\n"
"--log               : use this if you're not specifying anything\n"
"--log-level level   : level = [1-8] or a string\n"
"--log-prefix prefix : max. %d chars.\n"
"--log-ip            : put ip info. in the log for ip packets\n"
"--log-arp           : put (r)arp info. in the log for (r)arp packets\n"
"--log-ip6           : put ip6 info. in the log for ip6 packets\n"
	, EBT_LOG_PREFIX_SIZE - 1);
	for (i = 0; i < 8; i++)
		printf("%d = %s\n", eight_priority[i].c_val,
				    eight_priority[i].c_name);
}

static void brlog_init(struct xt_entry_target *t)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)t->data;

	loginfo->loglevel = LOG_NOTICE;
}

static void brlog_parse(struct xt_option_call *cb)
{
	struct ebt_log_info *loginfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_LOG_IP:
	case O_LOG_ARP:
	case O_LOG_IP6:
		loginfo->bitmask |= 1 << cb->entry->id;
		break;
	}
}

static void brlog_print(const void *ip, const struct xt_entry_target *target,
			int numeric)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)target->data;

	printf("--log-level %s", eight_priority[loginfo->loglevel].c_name);

	if (loginfo->prefix[0])
		printf(" --log-prefix \"%s\"", loginfo->prefix);

	if (loginfo->bitmask & EBT_LOG_IP)
		printf(" --log-ip");
	if (loginfo->bitmask & EBT_LOG_ARP)
		printf(" --log-arp");
	if (loginfo->bitmask & EBT_LOG_IP6)
		printf(" --log-ip6");
	printf(" ");
}

static int brlog_xlate(struct xt_xlate *xl,
		       const struct xt_xlate_tg_params *params)
{
	const struct ebt_log_info *loginfo = (const void *)params->target->data;

	xt_xlate_add(xl, "log");
	if (loginfo->prefix[0])
		xt_xlate_add(xl, " prefix \"%s\"", loginfo->prefix);

	if (loginfo->loglevel != LOG_DEFAULT_LEVEL)
		xt_xlate_add(xl, " level %s", eight_priority[loginfo->loglevel].c_name);

	/* ebt_log always decodes MAC header, nft_log always decodes upper header -
	 * so set flags ether and ignore EBT_LOG_IP, EBT_LOG_ARP and EBT_LOG_IP6 */
	xt_xlate_add(xl, " flags ether ");

	return 1;
}

static struct xtables_target brlog_target = {
	.name		= "log",
	.revision	= 0,
	.ext_flags	= XTABLES_EXT_WATCHER,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_log_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_log_info)),
	.init		= brlog_init,
	.help		= brlog_help,
	.x6_parse	= brlog_parse,
	.print		= brlog_print,
	.xlate		= brlog_xlate,
	.x6_options	= brlog_opts,
};

void _init(void)
{
	xtables_register_target(&brlog_target);
}
