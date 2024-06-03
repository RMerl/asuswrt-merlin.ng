/* ebt_ip
 * 
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * Changes:
 *    added ip-sport and ip-dport; parsing of port arguments is
 *    based on code from iptables-1.2.7a
 *    Innominate Security Technologies AG <mhopf@innominate.com>
 *    September, 2002
 *
 *  Extend by Broadcom at Jan 31, 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_ip_extend.h>


#define IP_TOS_EXTEND	'1' /* include/bits/in.h seems to already define IP_TOS */
#define IP_DSCP_EXTEND	'2' 
#define IP_RANGE_SRC	'3'
#define IP_RANGE_DST	'4'


static struct option opts[] =
{
	{ "ip-tos-extend"              , required_argument, 0, IP_TOS_EXTEND  },
	{ "ip-dscp-extend"             , required_argument, 0, IP_DSCP_EXTEND },
	{ "ip-src-range"               , required_argument, 0, IP_RANGE_SRC   },
	{ "ip-dst-range"               , required_argument, 0, IP_RANGE_DST   },
	{ 0 }
};

static uint8_t parse_tos(const char *tosstr)
{
	char *end;
	int tos;

	tos = strtol(tosstr, &end, 16);
	if ((*end == '\0') || (*end == ':') || (*end == '/')) {
		if (tos >= 0 && tos <= 0xFF) {
			return tos;
		}
	}

	ebt_print_error("Problem with specified tos '%s'", tosstr);
	return 0;
}

static int parse_tos_mask(char *mask, unsigned char *mask2)
{
	char *end;

	*mask2 = (unsigned char)strtol(mask, &end, 16);

	if (*end != '\0')
		ebt_print_error("Problem with specified tos mask 0x%x", mask2);

	return 0;
}

static void
parse_tos_range_mask(const char *tosstring, uint8_t *tos, uint8_t *mask)
{
	char *buffer;
	char *cp;
	char *p;

	buffer = strdup(tosstring);
	p = strrchr(buffer, '/');
	cp = strchr(buffer, ':');

	if (cp == NULL) {
		tos[0] = tos[1] = parse_tos(buffer);
	} else {
		*cp = '\0';
		cp++;
		tos[0] = buffer[0] ? parse_tos(buffer) : 0;
        if (ebt_errormsg[0] != '\0')
			return;
		tos[1] = cp[0] ? parse_tos(cp) : 0xFF;
		if (ebt_errormsg[0] != '\0')
			return;
		
		if (tos[0] > tos[1])
			ebt_print_error("Invalid tosrange (min > max)");
	}

	if (p != NULL) {
		parse_tos_mask(p + 1, (unsigned char *)mask);
	} else {
		*mask = 0xFF;
	} 

	free(buffer);
}

static void
parse_ip_range(const char *range_str, struct ebt_iprange *range)
{
	char *buffer;
	char *p1;
	char *p2;
	__be32 msk;

	buffer = strdup(range_str);
	p1 = buffer;
	p2 = strchr(buffer, '-');

	if (p2 == NULL) {
		ebt_print_error("Invalid ip range (missing -)");
	} else {
		*p2 = '\0';
		p2++;
		ebt_parse_ip_address(p1, &range->ip_min, &msk);
		if (ebt_errormsg[0] != '\0')
			goto exit;

		ebt_parse_ip_address(p2, &range->ip_max, &msk);
		if (ebt_errormsg[0] != '\0')
			goto exit;

		if (ntohl(range->ip_min) > ntohl(range->ip_max))
			ebt_print_error("Invalid ip range (min > max)");
	}

exit:
	free(buffer);
}

static void print_tos_range_mask(uint8_t *tos, uint8_t mask)
{
	char str[128];
	int i;
	char * p = str;

	memset(str, 0, sizeof(str));
	if (tos[0] == tos[1])
		i = snprintf(str, sizeof(str), "0x%02X", tos[0]);
	else
		i = snprintf(str, sizeof(str),"0x%02X:0x%02X", tos[0], tos[1]);

	if (mask != 0xFF)
		snprintf(p + i, sizeof(str) - i, "/0x%02X ", mask);
	else
		snprintf(p + i, sizeof(str) - i, " ");

	printf("%s", str);
}

static void print_ip_range(const struct ebt_iprange *range)
	{
		const unsigned char *byte_min, *byte_max;
	
		byte_min = (const unsigned char *)&range->ip_min;
		byte_max = (const unsigned char *)&range->ip_max;
		printf("%u.%u.%u.%u-%u.%u.%u.%u ",
			byte_min[0], byte_min[1], byte_min[2], byte_min[3],
			byte_max[0], byte_max[1], byte_max[2], byte_max[3]);
	}

static void print_help()
{
	printf(
"ip options:\n"
"--ip-tos-extend    [!] tos[:tos][/mask]   : ip tos specification\n"
"--ip-dscp-extend   [!] dscp               : ip dscp specification\n"
"--ip-src-range     [!] ip[-ip]            : ipv4 source range\n"
"--ip-dst-range     [!] ip[-ip]            : ipv4 destination range\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_ip_extend_info *ipinfo = (struct ebt_ip_extend_info *)match->data;

	ipinfo->invflags = 0;
	ipinfo->bitmask = 0;
}

#define OPT_TOS_EXTEND	0x01
#define OPT_DSCP_EXTEND	0x02

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_ip_extend_info *ipinfo = (struct ebt_ip_extend_info *)(*match)->data;
	char *end;
	long int i;

	switch (c) {
	case IP_TOS_EXTEND:
		ebt_check_option2(flags, OPT_TOS_EXTEND);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP_TOS_EXTEND;
        parse_tos_range_mask(optarg, ipinfo->tos, &ipinfo->tosmask);
		ipinfo->bitmask |= EBT_IP_TOS_EXTEND;
		break;
	case IP_DSCP_EXTEND:
		ebt_check_option2(flags, OPT_DSCP_EXTEND);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP_DSCP_EXTEND;
		i = strtol(optarg, &end, 16);
		if (i < 0 || i > 255 || (i & 0x3) || *end != '\0')
			ebt_print_error("Problem with specified IP dscp");
		ipinfo->dscp = i;
		ipinfo->bitmask |= EBT_IP_DSCP_EXTEND;
		break;

	case IP_RANGE_SRC:
		ebt_check_option2(flags, IP_RANGE_SRC);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP_RANGE_SRC;
		parse_ip_range(optarg, &ipinfo->range_src);
		ipinfo->bitmask |= EBT_IP_RANGE_SRC;
		break;

	case IP_RANGE_DST:
		ebt_check_option2(flags, IP_RANGE_DST);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP_RANGE_DST;
		parse_ip_range(optarg, &ipinfo->range_dst);
		ipinfo->bitmask |= EBT_IP_RANGE_DST;
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
	if (entry->ethproto != ETH_P_IP) {
		ebt_print_error("For IP filtering the protocol must be "
					"specified as IPv4");
	}
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_ip_extend_info *ipinfo = (struct ebt_ip_extend_info *)match->data;

	if (ipinfo->bitmask & EBT_IP_TOS_EXTEND) {
		printf("--ip-tos-extend ");
		if (ipinfo->invflags & EBT_IP_TOS_EXTEND)
			printf("! ");
		print_tos_range_mask(ipinfo->tos, ipinfo->tosmask);
	}
	if (ipinfo->bitmask & EBT_IP_DSCP_EXTEND) {
		printf("--ip-dscp-extend ");
		if (ipinfo->invflags & EBT_IP_DSCP_EXTEND)
			printf("! ");
		printf("0x%02X ", ipinfo->dscp);
	}

	if (ipinfo->bitmask & EBT_IP_RANGE_SRC) {
		printf("--ip-src-range ");
		if (ipinfo->invflags & EBT_IP_RANGE_SRC)
			printf("! ");
		print_ip_range(&ipinfo->range_src);
	}

	if (ipinfo->bitmask & EBT_IP_RANGE_DST) {
		printf("--ip-dst-range ");
		if (ipinfo->invflags & EBT_IP_RANGE_DST)
			printf("! ");
		print_ip_range(&ipinfo->range_dst);
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_ip_extend_info *ipinfo1 = (struct ebt_ip_extend_info *)m1->data;
	struct ebt_ip_extend_info *ipinfo2 = (struct ebt_ip_extend_info *)m2->data;

	if (ipinfo1->bitmask != ipinfo2->bitmask)
		return 0;
	if (ipinfo1->invflags != ipinfo2->invflags)
		return 0;
	if (ipinfo1->bitmask & EBT_IP_TOS_EXTEND) {
		if (ipinfo1->tos[0] != ipinfo2->tos[0] ||
            ipinfo1->tos[1] != ipinfo2->tos[1] ||
            ipinfo1->tosmask != ipinfo2->tosmask) 
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP_DSCP_EXTEND) {
		if (ipinfo1->dscp != ipinfo2->dscp)
			return 0;
	}

	if (ipinfo1->bitmask & EBT_IP_RANGE_SRC) {
		if (ipinfo1->range_src.ip_min != ipinfo2->range_src.ip_min ||
			ipinfo1->range_src.ip_max != ipinfo2->range_src.ip_max)
			return 0;
	}

	if (ipinfo1->bitmask & EBT_IP_RANGE_DST) {
		if (ipinfo1->range_dst.ip_min != ipinfo2->range_dst.ip_min ||
			ipinfo1->range_dst.ip_max != ipinfo2->range_dst.ip_max)
			return 0;
	}

	return 1;
}

static struct ebt_u_match ip_extend_match =
{
	.name		= EBT_IP_MATCH_EXTEND,
	.size		= sizeof(struct ebt_ip_extend_info),
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
	ebt_register_match(&ip_extend_match);
}
