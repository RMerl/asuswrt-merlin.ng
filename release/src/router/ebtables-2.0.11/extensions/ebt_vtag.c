/* ebt_vtag
 *
 * Authors:
 * Jack Po-chin Chang <jack.chang@broadcom.com>
 *
 * Feb, 2020 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_vtag_t.h>

#define VTAG_TARGET   0X0001
#define VTAG_SET      0X0002

static struct option opts[] =
{
   { "vtag-target" , required_argument, 0, VTAG_TARGET  },
   { "vtag-set"    , required_argument, 0, VTAG_SET     },
   { 0 }
};

static void print_help()
{
   printf(
   "vtag target options:\n"
   " --vtag-set value     : Set vlan tag value\n"
   " --vtag-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void init(struct ebt_entry_target *target)
{
   struct ebt_vtag_t_info *vtaginfo =
      (struct ebt_vtag_t_info *)target->data;

   vtaginfo->target = EBT_ACCEPT;
   vtaginfo->vtag = 0;
}

#define OPT_VTAG_TARGET   0X01
#define OPT_VTAG_SET      0X02

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
   struct ebt_vtag_t_info *vtaginfo =
      (struct ebt_vtag_t_info *)(*target)->data;
   char *end;

   switch(c) {
   case VTAG_TARGET:
      ebt_check_option2(flags, OPT_VTAG_TARGET);
      if (FILL_TARGET(optarg, vtaginfo->target))
         ebt_print_error2("Illegal --vtag-target target");
      break;

   case VTAG_SET:
      ebt_check_option2(flags, OPT_VTAG_SET);
      vtaginfo->vtag = strtoul(optarg, &end, 0);
      if (*end != '\0' || end == optarg)
         ebt_print_error2("Bad --vtag-set value '%s'", optarg);
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
	struct ebt_vtag_t_info *vtaginfo =
	   (struct ebt_vtag_t_info *)target->data;

	if (BASE_CHAIN && vtaginfo->target == EBT_RETURN)
		ebt_print_error("--vtag-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_vtag_t_info *vtaginfo =
	   (struct ebt_vtag_t_info *)target->data;

	printf(" --vtag-set 0x%x", vtaginfo->vtag);
	printf(" --vtag-target %s", TARGET_NAME(vtaginfo->target));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_vtag_t_info *vtaginfo1 =
	   (struct ebt_vtag_t_info *)t1->data;
	struct ebt_vtag_t_info *vtaginfo2 =
	   (struct ebt_vtag_t_info *)t2->data;

	return vtaginfo1->target == vtaginfo2->target &&
	   vtaginfo1->vtag == vtaginfo2->vtag;
}

static struct ebt_u_target vtag_target =
{
	.name		= "vtag",
	.size		= sizeof(struct ebt_vtag_t_info),
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
	ebt_register_target(&vtag_target);
}
