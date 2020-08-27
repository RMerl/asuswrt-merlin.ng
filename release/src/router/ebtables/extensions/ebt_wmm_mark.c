/*
 *  ebt_wmm_mark
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include "../include/linux/netfilter_bridge/ebt_wmm_mark_t.h"

//static int mark_supplied;
#define WMM_MARK_TARGET '1'
#define WMM_MARK_TAG  '2'
#define WMM_MARK_POS  '4'
#define WMM_MARK_SET  '8'

static struct option opts[] =
{
	{ "wmm-mark-target" 	, required_argument, 0, WMM_MARK_TARGET },
	{ "wmm-marktag"    	, required_argument, 0, WMM_MARK_TAG },
	{ "wmm-markpos"      , required_argument, 0, WMM_MARK_POS },
	{ "wmm-markset"    	, required_argument, 0, WMM_MARK_SET },	
	{ 0 }
};

static void print_help()
{
	printf(
	"wmm-mark target options:\n"
	" --wmm-mark-target target : ACCEPT, DROP, RETURN or CONTINUE\n"	
	" --wmm-marktag value      : set nfmark based on: dscp or vlan \n"
	" --wmm-markset value      : set nfmark regardless of the mark based on\n"	
	" --wmm-markpos            : bit offset of nfmark to set\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_wmm_mark_t_info *markinfo =
	   (struct ebt_wmm_mark_t_info *)target->data;

	markinfo->target = EBT_ACCEPT;
	markinfo->mark = WMM_MARK_DSCP;
	markinfo->markpos = PRIO_LOC_NFMARK;
	markinfo->markset = WMM_MARK_VALUE_NONE;	
//	mark_supplied = 0;
}

#define OPT_WMM_MARK_TARGET	0x01
#define OPT_WMM_MARK_TAG   	0x02
#define OPT_WMM_MARK_POS   	0x04
#define OPT_WMM_MARK_SET   	0x08

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_wmm_mark_t_info *markinfo =
	   (struct ebt_wmm_mark_t_info *)(*target)->data;
	char *end;

	//printf("c:%d, flags=%d\n", c, *flags);
	
	switch (c) {
	case WMM_MARK_TARGET:
		ebt_check_option2(flags, OPT_WMM_MARK_TARGET);
		if (FILL_TARGET(optarg, markinfo->target))
			ebt_print_error2("Illegal --wmm-mark-target target");
		break;		
		
	case WMM_MARK_POS:
		ebt_check_option2(flags, OPT_WMM_MARK_POS);
		markinfo->markpos = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --wmm-markpos value '%s'", optarg);
			
		//printf("--wmm-markpos %d\n", markinfo->markpos);

		break;

	case WMM_MARK_SET:
		ebt_check_option2(flags, OPT_WMM_MARK_SET);
		markinfo->markset = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --wmm-markset value '%s'", optarg);
			
		//printf("--wmm-markset %d\n", markinfo->markset);

		break;
				
	case WMM_MARK_TAG:
		ebt_check_option2(flags, OPT_WMM_MARK_TAG);
		if (optind > argc)
			ebt_print_error2("Missing wmm-marktag argument");
		
		if(!strcmp(argv[optind - 1], WMM_MARK_DSCP_STR)) {
			//printf("--wmm-marktag dscp\n");
			markinfo->mark = WMM_MARK_DSCP;
			//mark_supplied = 1;
		} else if(!strcmp(argv[optind - 1], WMM_MARK_8021D_STR)) {
			//printf("--wmm-marktag vlan\n");
			markinfo->mark = WMM_MARK_8021D;
			//mark_supplied = 1;		
		} else 
			ebt_print_error2("Bad --wmm-marktagt value '%s'", argv[optind - 1]);
							
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
	struct ebt_wmm_mark_t_info *markinfo =
	   (struct ebt_wmm_mark_t_info *)target->data;
		            
	if(markinfo->mark == WMM_MARK_DSCP) {
		if ((entry->ethproto != ETH_P_IPV6 && entry->ethproto != ETH_P_IP) || entry->invflags & EBT_IPROTO)
			ebt_print_error("wmm-mark dscp must be used with -p IPv4/IPv6");
		
	} else if (markinfo->mark == WMM_MARK_8021D) {
		if (entry->ethproto != ETH_P_8021Q || entry->invflags & EBT_IPROTO)
			ebt_print_error("wmm-mark vlan must be used with -p 802_1Q");	
	}
			
	if (BASE_CHAIN && markinfo->target == EBT_RETURN)
		ebt_print_error("--wmm-mark-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	
	struct ebt_wmm_mark_t_info *markinfo =
	   (struct ebt_wmm_mark_t_info *)target->data;

	printf(" --wmm-mark ");
	switch (markinfo->mark){
		case WMM_MARK_DSCP:
			printf("dscp");
			break;
		case WMM_MARK_8021D:
			printf("vlan");
			break;			
		default:
			printf("invalid");
					
	}
	
	printf(" --wmm-markpos %d", markinfo->markpos);	
	printf(" --wmm-markset %d", markinfo->markset);	
	printf(" --wmm-mark-target %s", TARGET_NAME(markinfo->target));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_wmm_mark_t_info *markinfo1 =
	   (struct ebt_wmm_mark_t_info *)t1->data;
	struct ebt_wmm_mark_t_info *markinfo2 =
	   (struct ebt_wmm_mark_t_info *)t2->data;

	return markinfo1->target == markinfo2->target &&
	   markinfo1->mark == markinfo2->mark &&	
	   markinfo1->markset == markinfo2->markset && 
	   markinfo1->markpos == markinfo2->markpos;
}

static struct ebt_u_target mark_target =
{
	.name		= EBT_WMM_MARK_TARGET,
	.size		= sizeof(struct ebt_wmm_mark_t_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

__attribute__((constructor)) static void extension_init(void)
{
	ebt_register_target(&mark_target);
}
