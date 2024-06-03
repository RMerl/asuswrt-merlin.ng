/* ebt_skb_priority
 *
 * Authors:
 * Broadcom
 *
 * June, 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_skb_priority.h>
#include <include/uapi/linux/pkt_sched.h>

#define SET_TARGET '1'
#define SET_SKB_PRIORITY '2'
static const struct option opts[] =
{
	{ "target", required_argument, 0, SET_TARGET },
	{ "set-class", required_argument, 0, SET_SKB_PRIORITY },
	{ 0 }
};

static void print_help()
{
	printf(
	"skbpriority option:\n"
	" --target target : ACCEPT, DROP, RETURN or CONTINUE\n"
	" --set-class MAJOR:MINOR   : Set skb->priority value (always hexadecimal!)\n"
	"             MAJOR: higher 16bits\n"
	"             MINOR: lower 16bits\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_skb_prioity_info *skb_priority_info =
	   (struct ebt_skb_prioity_info *)target->data;

	skb_priority_info->prio = 0;
	skb_priority_info->target = EBT_ACCEPT;
	return;
}

#define OPT_SET_SKB_TARGET 0x01
#define OPT_SET_SKB_PRIORITY  0x02

static int classify_string_to_priority(const char *s, unsigned int *p)
{
	unsigned int i, j;

	if (sscanf(s, "%x:%x", &i, &j) != 2)
		return 1;
	
	*p = TC_H_MAKE(i<<16, j);
	return 0;
}

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_skb_prioity_info *skb_priority_info =
	   (struct ebt_skb_prioity_info *)(*target)->data;

	switch (c) {
	case SET_TARGET:
		ebt_check_option2(flags, OPT_SET_SKB_TARGET);
		if (FILL_TARGET(optarg, skb_priority_info->target))
			ebt_print_error2("Illegal --target target");
		return 1;
	case SET_SKB_PRIORITY:
		ebt_check_option2(flags, OPT_SET_SKB_PRIORITY);
		if(classify_string_to_priority(optarg, &(skb_priority_info->prio)))
			ebt_print_error2("Bad --set-class value '%s'", optarg);
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_skb_prioity_info *skb_priority_info = (struct ebt_skb_prioity_info *)target->data;

	if (BASE_CHAIN && skb_priority_info->target == EBT_RETURN)
		ebt_print_error("--target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_skb_prioity_info *skb_priority_info = (struct ebt_skb_prioity_info *)target->data;
	int tmp = skb_priority_info->target;

	printf(" --set-class %x:%x", TC_H_MAJ(skb_priority_info->prio)>>16, TC_H_MIN(skb_priority_info->prio));
	printf(" --target %s", TARGET_NAME(tmp));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	return 1;
}

static struct ebt_u_target skb_priority_target =
{
	.name		= EBT_SKB_PRIORITY_TARGET,
	.size		= sizeof(struct ebt_skb_prioity_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _INIT(void)
{
	ebt_register_target(&skb_priority_target);
}
