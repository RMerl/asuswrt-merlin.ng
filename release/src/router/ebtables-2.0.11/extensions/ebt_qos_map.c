/*
 *	ebt_qos_map
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_qos_map.h>

#define QOS_MAP_DSCP2PBIT  0x01
#define QOS_MAP_DSCP2Q     0x02

static struct option opts[] =
{
	{ "dscp2pbit" 	, required_argument, 0, QOS_MAP_DSCP2PBIT },
	{ "dscp2q"    	, required_argument, 0, QOS_MAP_DSCP2Q },
	{ 0 }
};

static void print_help()
{
	printf(
	"QOSMAP target options:\n"
	" --dscp2pbit value      : set pbit value based on dcsp field\n"
	" --dscp2q    value      : set egress queue based on dcsp field\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_qos_map_info *qosinfo =
	   (struct ebt_qos_map_info *)target->data;

	qosinfo->dscp2pbit = 0;
	qosinfo->dscp2q = 0;
}

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_qos_map_info *qosinfo = (struct ebt_qos_map_info *)(*target)->data;
	char *end;

	switch (c) {
	case QOS_MAP_DSCP2PBIT:
		ebt_check_option2(flags, QOS_MAP_DSCP2PBIT);
		qosinfo->dscp2pbit = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --dscp2pbit value '%s'", optarg);
			
		break;
				
	case QOS_MAP_DSCP2Q:
		ebt_check_option2(flags, QOS_MAP_DSCP2Q);
		qosinfo->dscp2q = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --dscp2q value '%s'", optarg);
			
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
	struct ebt_qos_map_info *qosinfo = (struct ebt_qos_map_info *)target->data;
		            
	if((qosinfo->dscp2pbit == QOS_MAP_DSCP2PBIT) || (qosinfo->dscp2q== QOS_MAP_DSCP2Q)) {
		if ((entry->ethproto != ETH_P_IPV6 && entry->ethproto != ETH_P_IP) || entry->invflags & EBT_IPROTO)
			ebt_print_error("QOSMAP must be used with -p IPv4/IPv6");
		
	}
			
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_qos_map_info *qosinfo = (struct ebt_qos_map_info *)target->data;

	printf(" --dscp2pbit %d", qosinfo->dscp2pbit);
	printf(" --dscp2q %d", qosinfo->dscp2q);
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	return 1;
}

static struct ebt_u_target qos_map_target =
{
	.name		= "QOSMAP",
	.size		= sizeof(struct ebt_qos_map_info),
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
	ebt_register_target(&qos_map_target);
}
