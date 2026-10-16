/* ebt_pkttype
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * April, 2003
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <xtables.h>
#include <linux/if_packet.h>
#include <linux/netfilter_bridge/ebt_pkttype.h>

static const char *classes[] = {
	"host",
	"broadcast",
	"multicast",
	"otherhost",
	"outgoing",
	"loopback",
	"fastroute",
};

enum {
	O_TYPE,
};

static const struct xt_option_entry brpkttype_opts[] = {
	{ .name = "pkttype-type", .id = O_TYPE, .type = XTTYPE_STRING,
	  .flags = XTOPT_INVERT },
	XTOPT_TABLEEND,
};

static void brpkttype_print_help(void)
{
	printf(
"pkttype options:\n"
"[!] --pkttype-type    type: class the packet belongs to\n"
"Possible values: broadcast, multicast, host, otherhost, or any other byte value (which would be pretty useless).\n");
}


static void brpkttype_parse(struct xt_option_call *cb)
{
	struct ebt_pkttype_info *ptinfo = cb->data;
	long int i;
	char *end;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_TYPE:
		ptinfo->invert = cb->invert;
		i = strtol(cb->arg, &end, 16);
		if (*end != '\0') {
			for (i = 0; i < ARRAY_SIZE(classes); i++) {
				if (!strcasecmp(cb->arg, classes[i]))
					break;
			}
			if (i >= ARRAY_SIZE(classes))
				xtables_error(PARAMETER_PROBLEM,
					      "Could not parse class '%s'",
					      cb->arg);
		}
		if (i < 0 || i > 255)
			xtables_error(PARAMETER_PROBLEM, "Problem with specified pkttype class");
		ptinfo->pkt_type = (uint8_t)i;
		break;
	}
}

static void brpkttype_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	struct ebt_pkttype_info *pt = (struct ebt_pkttype_info *)match->data;

	printf("%s--pkttype-type ", pt->invert ? "! " : "");

	if (pt->pkt_type < ARRAY_SIZE(classes))
		printf("%s ", classes[pt->pkt_type]);
	else
		printf("%d ", pt->pkt_type);
}

static int brpkttype_xlate(struct xt_xlate *xl,
			  const struct xt_xlate_mt_params *params)
{
	const struct ebt_pkttype_info *info = (const void*)params->match->data;

	xt_xlate_add(xl, "meta pkttype %s", info->invert ? "!= " : "");

	if (info->pkt_type < 3)
		xt_xlate_add(xl, "%s ", classes[info->pkt_type]);
	else if (info->pkt_type == 3)
		xt_xlate_add(xl, "other ");
	else
		xt_xlate_add(xl, "%d ", info->pkt_type);

	return 1;
}

static struct xtables_match brpkttype_match = {
	.name		= "pkttype",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_pkttype_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_pkttype_info)),
	.help		= brpkttype_print_help,
	.x6_parse	= brpkttype_parse,
	.print		= brpkttype_print,
	.xlate		= brpkttype_xlate,
	.x6_options	= brpkttype_opts,
};

void _init(void)
{
	xtables_register_match(&brpkttype_match);
}
