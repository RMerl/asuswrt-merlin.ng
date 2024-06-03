/*
 *	ebt_reject
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include "../include/linux/netfilter_bridge/ebt_reject.h"

#define EBT_REJECT_WITH  0x01

static struct option opts[] =
{
	{ "reject-with" , required_argument, 0, EBT_REJECT_WITH },
	{ 0 }
};

static void print_help()
{
	printf(
	"REJECT target options:\n"
	" --reject-with value      : reject reason\n");
}

static void init(struct ebt_entry_target *target)
{
    struct ebt_reject_info *reject_info = (struct ebt_reject_info *)target->data;

	reject_info->with = -1;
}

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_reject_info *reject_info = (struct ebt_reject_info *)(*target)->data;
	char *end;

	switch (c) {
	case EBT_REJECT_WITH:
		reject_info->with = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --reject-with value '%s'", optarg);
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
	return;
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_reject_info *reject_info = (struct ebt_reject_info *)target->data;

	printf(" --reject-with %d", reject_info->with);
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	return 1;
}

static struct ebt_u_target reject_target =
{
	.name		= "REJECT",
	.size		= sizeof(struct ebt_reject_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _INIT(void)
{
	ebt_register_target(&reject_target);
}
