/* ebt_ip6
 * 
 * Authors:
 * Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 * Manohar Castelino <manohar.castelino@intel.com>
 *
 * Summary:
 * This is just a modification of the IPv4 code written by 
 * Bart De Schuymer <bdschuym@pandora.be>
 * with the changes required to support IPv6
 *
 *  Extend by Broadcom at Jan 31, 2019
 */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <stdbool.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_ip6_extend.h>



#define IP_TCLASS_EXTEND '1'
#define IP_FLOWLABEL_EXTEND '2'
#define IP_RANGE_SRC     '3'
#define IP_RANGE_DST     '4'


static const struct option opts[] =
{
	{ "ip6-traffic-class-extend"    , required_argument, 0, IP_TCLASS_EXTEND },
	{ "ip6-tclass-extend"           , required_argument, 0, IP_TCLASS_EXTEND},
	{ "ip6-flow-label-extend"       , required_argument, 0,  IP_FLOWLABEL_EXTEND},
	{ "ip6-src-range"               , required_argument, 0, IP_RANGE_SRC},
	{ "ip6-dst-range"               , required_argument, 0, IP_RANGE_DST},
	{ 0 }
};

static uint8_t parse_tclass(const char *tclassstr)
{
	char *end;
	int tclass;

	tclass = strtol(tclassstr, &end, 16);
	if ((*end == '\0') || (*end == ':') || (*end == '/')) {
		if (tclass >= 0 && tclass <= 0xFF) {
            return tclass;
		}
	}

	ebt_print_error("Problem with specified tclass '%s'", tclassstr);
	return 0;
}

static int parse_tclass_mask(char *mask, unsigned char *mask2)
{
	char *end;

	*mask2 = (unsigned char)strtol(mask, &end, 16);

	if (*end != '\0')
		ebt_print_error("Problem with specified tlcass mask 0x%x", mask2);

	return 0;
}

static void
parse_tclass_range_mask(const char *tclassstring, uint8_t *tclass, uint8_t *mask)
{
	char *buffer;
	char *cp;
	char *p;

	buffer = strdup(tclassstring);
	p = strrchr(buffer, '/');
	cp = strchr(buffer, ':');

	if (cp == NULL) {
		tclass[0] = tclass[1] = parse_tclass(buffer);
	} else {
		*cp = '\0';
		cp++;
		tclass[0] = buffer[0] ? parse_tclass(buffer) : 0;
        if (ebt_errormsg[0] != '\0')
			return;
		tclass[1] = cp[0] ? parse_tclass(cp) : 0xFF;
		if (ebt_errormsg[0] != '\0')
			return;
		
		if (tclass[0] > tclass[1])
			ebt_print_error("Invalid tclass range (min > max)");
	}

	if (p != NULL) {
		parse_tclass_mask(p + 1, (unsigned char *)mask);
	} else {
		*mask = 0xFF;
	} 
   
	free(buffer);
}

static void
parse_flow_label(uint32_t input, uint8_t * flow_lbl)
{
	char szFlowLabel[10] = {0};
	char szTemp[3] = {0};
	int i;
	char *end;

	sprintf(szFlowLabel, "%06X", input);

	for (i = 0; i < 3; i++)
	{
		szTemp[0] = szFlowLabel[2*i];
		szTemp[1] = szFlowLabel[2*i+1];
		*(flow_lbl + i) = strtoul(szTemp, &end, 16);
	}
}

static bool
ip6_less_than(const struct in6_addr *a, const struct in6_addr *b)
{
	unsigned int i;

	for (i = 0; i < 4; ++i) {
		if (a->s6_addr32[i] != b->s6_addr32[i])
			return ntohl(a->s6_addr32[i]) < ntohl(b->s6_addr32[i]);
	}

	return 0;
}

static void
parse_ip6_range(const char *range_str, struct ebt_ip6range *range)
{
	char *buffer;
	char *p1;
	char *p2;
	struct in6_addr msk;

	buffer = strdup(range_str);
	p1 = buffer;
	p2 = strchr(buffer, '-');

	if (p2 == NULL) {
		ebt_print_error("Invalid ip range (missing -)");
	} else {
		*p2 = '\0';
		p2++;
		ebt_parse_ip6_address(p1, &range->ip_min, &msk);
		if (ebt_errormsg[0] != '\0')
			goto exit;

		ebt_parse_ip6_address(p2, &range->ip_max, &msk);
		if (ebt_errormsg[0] != '\0')
			goto exit;

		if (ip6_less_than(&range->ip_max, &range->ip_min))
			ebt_print_error("Invalid ip range (min > max)");
	}

exit:
	free(buffer);
}

static void print_tclass_range_mask(uint8_t *tclass, uint8_t mask)
{
	char str[128];
	int i;
	char * p = str;

	memset(str, 0, sizeof(str));
	if (tclass[0] == tclass[1])
		i = snprintf(str, sizeof(str), "0x%02X", tclass[0]);
	else
		i = snprintf(str, sizeof(str),"0x%02X:0x%02X", tclass[0], tclass[1]);

	if (mask != 0xFF)
		snprintf(p + i, sizeof(str) - i, "/0x%02X ", mask);
	else
		snprintf(p + i, sizeof(str) - i, " ");

	printf("%s", str);
}

static void print_ip6_range(const struct ebt_ip6range *range)
	{
		printf("%s-", ebt_ip6_to_numeric(&range->ip_min));
		printf("%s ", ebt_ip6_to_numeric(&range->ip_max));
	}

static void print_help()
{
	printf(
"ip6 options:\n"
"--ip6-tclass-extend [!] tclass[:tclass][/mask]        : ipv6 traffic class specification\n"
"--ip6-flow-label-extend [!] flowlabel                 : ipv6 flow label specification\n"
"--ip6-src-range     [!] ip[-ip]                       : ipv6 source range\n"
"--ip6-dst-range     [!] ip[-ip]                       : ipv6 destination range\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_ip6_extend_info *ipinfo = (struct ebt_ip6_extend_info *)match->data;

	ipinfo->invflags = 0;
	ipinfo->bitmask = 0;
}

#define OPT_TCLASS_EXTEND 0x01
#define OPT_FLOWLABEL_EXTEND 0x02
static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_ip6_extend_info *ipinfo = (struct ebt_ip6_extend_info *)(*match)->data;
	char *end;
	long int i;

	switch (c) {
	case IP_TCLASS_EXTEND:
		ebt_check_option2(flags, OPT_TCLASS_EXTEND);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_TCLASS_EXTEND;
        parse_tclass_range_mask(optarg, ipinfo->tclass, &ipinfo->tclassmsk);
		ipinfo->bitmask |= EBT_IP6_TCLASS_EXTEND;
		break;

	case IP_FLOWLABEL_EXTEND:
		ebt_check_option2(flags, OPT_FLOWLABEL_EXTEND);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_FLOWLABEL_EXTEND;
		i = strtoul(optarg, &end, 16);
		if (*end != '\0') {
			ebt_print_error2("Problem with specified IPv6 flow label");
		} else {
			parse_flow_label((i & 0xFFFFF), ipinfo->flow_lbl);
		}
		ipinfo->bitmask |= EBT_IP6_FLOWLABEL_EXTEND;
		break;

	case IP_RANGE_SRC:
		ebt_check_option2(flags, IP_RANGE_SRC);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_RANGE_SRC;
		parse_ip6_range(optarg, &ipinfo->range_src);
		ipinfo->bitmask |= EBT_IP6_RANGE_SRC;
		break;

	case IP_RANGE_DST:
		ebt_check_option2(flags, IP_RANGE_DST);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_RANGE_DST;
		parse_ip6_range(optarg, &ipinfo->range_dst);
		ipinfo->bitmask |= EBT_IP6_RANGE_DST;
		break;

	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
	if (entry->ethproto != ETH_P_IPV6) {
		ebt_print_error("For IPv6 filtering the protocol must be "
		            "specified as IPv6");
	} 
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_ip6_extend_info *ipinfo = (struct ebt_ip6_extend_info *)match->data;

	if (ipinfo->bitmask & EBT_IP6_TCLASS_EXTEND) {
		printf("--ip6-tclass-extend ");
		if (ipinfo->invflags & EBT_IP6_TCLASS_EXTEND)
			printf("! ");
        print_tclass_range_mask(ipinfo->tclass, ipinfo->tclassmsk);
	}
	if (ipinfo->bitmask & EBT_IP6_FLOWLABEL_EXTEND) {
		printf("--ip6-flow-label-extend ");
		if (ipinfo->invflags & EBT_IP6_FLOWLABEL_EXTEND)
			printf("! ");
		printf("0x%02X%02X%02X ", ipinfo->flow_lbl[0], ipinfo->flow_lbl[1], ipinfo->flow_lbl[2]);
	}

	if (ipinfo->bitmask & EBT_IP6_RANGE_SRC) {
		printf("--ip6-src-range ");
		if (ipinfo->invflags & EBT_IP6_RANGE_SRC)
			printf("! ");
		print_ip6_range(&ipinfo->range_src);
	}

	if (ipinfo->bitmask & EBT_IP6_RANGE_DST) {
		printf("--ip6-dst-range ");
		if (ipinfo->invflags & EBT_IP6_RANGE_DST)
			printf("! ");
		print_ip6_range(&ipinfo->range_dst);
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_ip6_extend_info *ipinfo1 = (struct ebt_ip6_extend_info *)m1->data;
	struct ebt_ip6_extend_info *ipinfo2 = (struct ebt_ip6_extend_info *)m2->data;

	if (ipinfo1->bitmask != ipinfo2->bitmask)
		return 0;
	if (ipinfo1->invflags != ipinfo2->invflags)
		return 0;
	if (ipinfo1->bitmask & EBT_IP6_TCLASS_EXTEND) {
		if (ipinfo1->tclass[0] != ipinfo2->tclass[0] ||
            ipinfo1->tclass[1] != ipinfo2->tclass[1] ||
            ipinfo1->tclassmsk != ipinfo2->tclassmsk)
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_FLOWLABEL_EXTEND) {
		if (ipinfo1->flow_lbl[0] != ipinfo2->flow_lbl[0] ||
			ipinfo1->flow_lbl[1] != ipinfo2->flow_lbl[1] ||
			ipinfo1->flow_lbl[2] != ipinfo2->flow_lbl[2])
			return 0;
	}

	if (ipinfo1->bitmask & EBT_IP6_RANGE_SRC) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->range_src.ip_min, &ipinfo2->range_src.ip_min) ||
			!IN6_ARE_ADDR_EQUAL(&ipinfo1->range_src.ip_max, &ipinfo2->range_src.ip_max))
			return 0;
	}

	if (ipinfo1->bitmask & EBT_IP6_RANGE_DST) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->range_dst.ip_min, &ipinfo2->range_dst.ip_min) ||
			!IN6_ARE_ADDR_EQUAL(&ipinfo1->range_dst.ip_max, &ipinfo2->range_dst.ip_max))
			return 0;
	}

	return 1;
}

static struct ebt_u_match ip6_extend_match =
{
	.name		= EBT_IP6_MATCH_EXTEND,
	.size		= sizeof(struct ebt_ip6_extend_info),
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
	ebt_register_match(&ip6_extend_match);
}
