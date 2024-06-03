
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter/xt_AUDIT.h>

#define AUDIT_TYPE  '1'
static struct option opts[] =
{
	{ "audit-type" , required_argument, 0, AUDIT_TYPE },
	{ 0 }
};

static void print_help()
{
	printf(
	"AUDIT target options:\n"
	" --audit-type TYPE          : Set action type to record.\n");
}

static void init(struct ebt_entry_target *target)
{
	struct xt_AUDIT_info *info = (struct xt_AUDIT_info *) target->data;

	info->type = 0;
}

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct xt_AUDIT_info *info = (struct xt_AUDIT_info *) (*target)->data;

	switch (c) {
	case AUDIT_TYPE:
		ebt_check_option2(flags, AUDIT_TYPE);

		if (!strcasecmp(optarg, "accept"))
			info->type = XT_AUDIT_TYPE_ACCEPT;
		else if (!strcasecmp(optarg, "drop"))
			info->type = XT_AUDIT_TYPE_DROP;
		else if (!strcasecmp(optarg, "reject"))
			info->type = XT_AUDIT_TYPE_REJECT;
		else
			ebt_print_error2("Bad action type value `%s'", optarg);

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
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	const struct xt_AUDIT_info *info =
		(const struct xt_AUDIT_info *) target->data;

	printf("--audit-type ");

	switch(info->type) {
	case XT_AUDIT_TYPE_ACCEPT:
		printf("accept");
		break;
	case XT_AUDIT_TYPE_DROP:
		printf("drop");
		break;
	case XT_AUDIT_TYPE_REJECT:
		printf("reject");
		break;
	}
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	const struct xt_AUDIT_info *info1 =
		(const struct xt_AUDIT_info *) t1->data;
	const struct xt_AUDIT_info *info2 =
		(const struct xt_AUDIT_info *) t2->data;

	return info1->type == info2->type;
}

static struct ebt_u_target AUDIT_target =
{
	.name		= "AUDIT",
	.size		= sizeof(struct xt_AUDIT_info),
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
	ebt_register_target(&AUDIT_target);
}
