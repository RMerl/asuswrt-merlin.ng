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
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat in 2015.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <inttypes.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_ip.h>

#include "libxt_icmp.h"

/* must correspond to the bit position in EBT_IP6_* defines */
enum {
	O_SOURCE = 0,
	O_DEST,
	O_TOS,
	O_PROTO,
	O_SPORT,
	O_DPORT,
	O_ICMP,
	O_IGMP,
	F_PORT = 1 << O_ICMP | 1 << O_IGMP,
	F_ICMP = 1 << O_SPORT | 1 << O_DPORT | 1 << O_IGMP,
	F_IGMP = 1 << O_SPORT | 1 << O_DPORT | 1 << O_ICMP,
};

static const struct xt_option_entry brip_opts[] = {
	{ .name = "ip-source",		.id = O_SOURCE, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip-src",		.id = O_SOURCE, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip-destination",	.id = O_DEST, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip-dst",		.id = O_DEST, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip-tos",		.id = O_TOS, .type = XTTYPE_UINT8,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, tos) },
	{ .name = "ip-protocol",	.id = O_PROTO, .type = XTTYPE_PROTOCOL,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, protocol) },
	{ .name = "ip-proto",		.id = O_PROTO, .type = XTTYPE_PROTOCOL,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, protocol) },
	{ .name = "ip-source-port",	.id = O_SPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, sport) },
	{ .name = "ip-sport",		.id = O_SPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, sport) },
	{ .name = "ip-destination-port",.id = O_DPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, dport) },
	{ .name = "ip-dport",		.id = O_DPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip_info, dport) },
	{ .name = "ip-icmp-type",       .id = O_ICMP, .type = XTTYPE_STRING,
	  .excl = F_ICMP, .flags = XTOPT_INVERT },
	{ .name = "ip-igmp-type",       .id = O_IGMP, .type = XTTYPE_STRING,
	  .excl = F_IGMP, .flags = XTOPT_INVERT },
	XTOPT_TABLEEND,
};

static void brip_print_help(void)
{
	printf(
"ip options:\n"
"[!] --ip-src    address[/mask]: ip source specification\n"
"[!] --ip-dst    address[/mask]: ip destination specification\n"
"[!] --ip-tos    tos           : ip tos specification\n"
"[!] --ip-proto  protocol      : ip protocol specification\n"
"[!] --ip-sport  port[:port]   : tcp/udp source port or port range\n"
"[!] --ip-dport  port[:port]   : tcp/udp destination port or port range\n"
"[!] --ip-icmp-type type[[:type]/code[:code]] : icmp type/code or type/code range\n"
"[!] --ip-igmp-type type[:type]               : igmp type or type range\n");

	printf("\nValid ICMP Types:\n");
	xt_print_icmp_types(icmp_codes, ARRAY_SIZE(icmp_codes));
	printf("\nValid IGMP Types:\n");
	xt_print_icmp_types(igmp_types, ARRAY_SIZE(igmp_types));
}

/* original code from ebtables: useful_functions.c */
static void print_icmp_code(uint8_t *code)
{
	if (!code)
		return;

	if (code[0] == code[1])
		printf("/%"PRIu8 " ", code[0]);
	else
		printf("/%"PRIu8":%"PRIu8 " ", code[0], code[1]);
}

static void ebt_print_icmp_type(const struct xt_icmp_names *codes,
				size_t n_codes, uint8_t *type, uint8_t *code)
{
	unsigned int i;

	if (type[0] != type[1]) {
		printf("%"PRIu8 ":%" PRIu8, type[0], type[1]);
		print_icmp_code(code);
		return;
	}

	for (i = 0; i < n_codes; i++) {
		if (codes[i].type != type[0])
			continue;

		if (!code || (codes[i].code_min == code[0] &&
			      codes[i].code_max == code[1])) {
			printf("%s ", codes[i].name);
			return;
		}
	}
	printf("%"PRIu8, type[0]);
	print_icmp_code(code);
}

static void brip_parse(struct xt_option_call *cb)
{
	struct ebt_ip_info *info = cb->data;

	xtables_option_parse(cb);

	info->bitmask |= 1 << cb->entry->id;
	info->invflags |= cb->invert ? 1 << cb->entry->id : 0;

	switch (cb->entry->id) {
	case O_SOURCE:
		cb->val.haddr.all[0] &= cb->val.hmask.all[0];
		info->saddr = cb->val.haddr.ip;
		info->smsk = cb->val.hmask.ip;
		break;
	case O_DEST:
		cb->val.haddr.all[0] &= cb->val.hmask.all[0];
		info->daddr = cb->val.haddr.ip;
		info->dmsk = cb->val.hmask.ip;
		break;
	case O_ICMP:
		ebt_parse_icmp(cb->arg, info->icmp_type, info->icmp_code);
		break;
	case O_IGMP:
		ebt_parse_igmp(cb->arg, info->igmp_type);
		break;
	}
}

static void brip_final_check(struct xt_fcheck_call *fc)
{
	if (!fc->xflags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void print_port_range(uint16_t *ports)
{
	if (ports[0] == ports[1])
		printf("%d ", ports[0]);
	else
		printf("%d:%d ", ports[0], ports[1]);
}

static void brip_print(const void *ip, const struct xt_entry_match *match,
		       int numeric)
{
	struct ebt_ip_info *info = (struct ebt_ip_info *)match->data;
	struct in_addr *addrp, *maskp;

	if (info->bitmask & EBT_IP_SOURCE) {
		if (info->invflags & EBT_IP_SOURCE)
			printf("! ");
		addrp = (struct in_addr *)&info->saddr;
		maskp = (struct in_addr *)&info->smsk;
		printf("--ip-src %s%s ",
		       xtables_ipaddr_to_numeric(addrp),
		       xtables_ipmask_to_numeric(maskp));
	}
	if (info->bitmask & EBT_IP_DEST) {
		if (info->invflags & EBT_IP_DEST)
			printf("! ");
		addrp = (struct in_addr *)&info->daddr;
		maskp = (struct in_addr *)&info->dmsk;
		printf("--ip-dst %s%s ",
		       xtables_ipaddr_to_numeric(addrp),
		       xtables_ipmask_to_numeric(maskp));
	}
	if (info->bitmask & EBT_IP_TOS) {
		if (info->invflags & EBT_IP_TOS)
			printf("! ");
		printf("--ip-tos 0x%02X ", info->tos);
	}
	if (info->bitmask & EBT_IP_PROTO) {
		struct protoent *pe;

		if (info->invflags & EBT_IP_PROTO)
			printf("! ");
		printf("--ip-proto ");
		pe = getprotobynumber(info->protocol);
		if (pe == NULL) {
			printf("%d ", info->protocol);
		} else {
			printf("%s ", pe->p_name);
		}
	}
	if (info->bitmask & EBT_IP_SPORT) {
		if (info->invflags & EBT_IP_SPORT)
			printf("! ");
		printf("--ip-sport ");
		print_port_range(info->sport);
	}
	if (info->bitmask & EBT_IP_DPORT) {
		if (info->invflags & EBT_IP_DPORT)
			printf("! ");
		printf("--ip-dport ");
		print_port_range(info->dport);
	}
	if (info->bitmask & EBT_IP_ICMP) {
		if (info->invflags & EBT_IP_ICMP)
			printf("! ");
		printf("--ip-icmp-type ");
		ebt_print_icmp_type(icmp_codes, ARRAY_SIZE(icmp_codes),
				    info->icmp_type, info->icmp_code);
	}
	if (info->bitmask & EBT_IP_IGMP) {
		if (info->invflags & EBT_IP_IGMP)
			printf("! ");
		printf("--ip-igmp-type ");
		ebt_print_icmp_type(igmp_types, ARRAY_SIZE(igmp_types),
				    info->igmp_type, NULL);
	}
}

static const char *brip_xlate_proto_to_name(uint8_t proto)
{
	switch (proto) {
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_UDP:
		return "udp";
	case IPPROTO_UDPLITE:
		return "udplite";
	case IPPROTO_SCTP:
		return "sctp";
	case IPPROTO_DCCP:
		return "dccp";
	default:
		return NULL;
	}
}

static void brip_xlate_icmp(struct xt_xlate *xl,
			    const struct ebt_ip_info *info, int bit)
{
	if ((info->bitmask & bit) == 0)
		return;

	xt_xlate_add(xl, "icmp type ");
	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");
	if (info->icmp_type[0] == info->icmp_type[1])
		xt_xlate_add(xl, "%d ", info->icmp_type[0]);
	else
		xt_xlate_add(xl, "%d-%d ", info->icmp_type[0],
					   info->icmp_type[1]);
	if (info->icmp_code[0] == 0 &&
	    info->icmp_code[1] == 0xff)
		return;

	xt_xlate_add(xl, "icmp code ");
	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");
	if (info->icmp_code[0] == info->icmp_code[1])
		xt_xlate_add(xl, "%d ", info->icmp_code[0]);
	else
		xt_xlate_add(xl, "%d-%d ", info->icmp_code[0],
					   info->icmp_code[1]);
}

static void brip_xlate_igmp(struct xt_xlate *xl,
			    const struct ebt_ip_info *info, int bit)
{
	if ((info->bitmask & bit) == 0)
		return;

	xt_xlate_add(xl, "@th,0,8 ");
	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");
	if (info->icmp_type[0] == info->icmp_type[1])
		xt_xlate_add(xl, "%d ", info->icmp_type[0]);
	else
		xt_xlate_add(xl, "%d-%d ", info->icmp_type[0],
					   info->icmp_type[1]);
}

static void brip_xlate_th(struct xt_xlate *xl,
			  const struct ebt_ip_info *info, int bit,
			  const char *pname)
{
	const uint16_t *ports;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP_SPORT:
		if (pname)
			xt_xlate_add(xl, "%s sport ", pname);
		else
			xt_xlate_add(xl, "@th,0,16 ");

		ports = info->sport;
		break;
	case EBT_IP_DPORT:
		if (pname)
			xt_xlate_add(xl, "%s dport ", pname);
		else
			xt_xlate_add(xl, "@th,16,16 ");

		ports = info->dport;
		break;
	default:
		return;
	}

	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");

	if (ports[0] == ports[1])
		xt_xlate_add(xl, "%d ", ports[0]);
	else
		xt_xlate_add(xl, "%d-%d ", ports[0], ports[1]);
}

static void brip_xlate_nh(struct xt_xlate *xl,
			  const struct ebt_ip_info *info, int bit)
{
	struct in_addr *addrp, *maskp;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP_SOURCE:
		xt_xlate_add(xl, "ip saddr ");
		addrp = (struct in_addr *)&info->saddr;
		maskp = (struct in_addr *)&info->smsk;
		break;
	case EBT_IP_DEST:
		xt_xlate_add(xl, "ip daddr ");
		addrp = (struct in_addr *)&info->daddr;
		maskp = (struct in_addr *)&info->dmsk;
		break;
	default:
		return;
	}

	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");

	xt_xlate_add(xl, "%s%s ", xtables_ipaddr_to_numeric(addrp),
				  xtables_ipmask_to_numeric(maskp));
}

static bool may_skip_ether_type_dep(uint8_t flags)
{
	/* these convert to "ip (s|d)addr" matches */
	if (flags & (EBT_IP_SOURCE | EBT_IP_DEST))
		return true;

	/* icmp match triggers implicit ether type dependency in nft */
	if (flags & EBT_IP_ICMP)
		return true;

	/* allow if "ip protocol" match is created by brip_xlate() */
	if (flags & EBT_IP_PROTO &&
	    !(flags & (EBT_IP_SPORT | EBT_IP_DPORT | EBT_IP_ICMP)))
		return true;

	return false;
}

static int brip_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct ebt_ip_info *info = (const void *)params->match->data;
	const char *pname = NULL;

	brip_xlate_nh(xl, info, EBT_IP_SOURCE);
	brip_xlate_nh(xl, info, EBT_IP_DEST);

	if (!may_skip_ether_type_dep(info->bitmask))
		xt_xlate_add(xl, "ether type ip ");

	if (info->bitmask & EBT_IP_TOS) {
		xt_xlate_add(xl, "@nh,8,8 ");
		if (info->invflags & EBT_IP_TOS)
			xt_xlate_add(xl, "!= ");
		xt_xlate_add(xl, "0x%02x ", info->tos);
	}
	if (info->bitmask & EBT_IP_PROTO) {
		struct protoent *pe;

		if (info->bitmask & (EBT_IP_SPORT|EBT_IP_DPORT|EBT_IP_ICMP) &&
		    (info->invflags & EBT_IP_PROTO) == 0) {
			/* port number or icmp given and not inverted, no need to print this */
			pname = brip_xlate_proto_to_name(info->protocol);
		} else {
			xt_xlate_add(xl, "ip protocol ");
			if (info->invflags & EBT_IP_PROTO)
				xt_xlate_add(xl, "!= ");
			pe = getprotobynumber(info->protocol);
			if (pe == NULL)
				xt_xlate_add(xl, "%d ", info->protocol);
			else
				xt_xlate_add(xl, "%s ", pe->p_name);
		}
	}

	brip_xlate_th(xl, info, EBT_IP_SPORT, pname);
	brip_xlate_th(xl, info, EBT_IP_DPORT, pname);

	brip_xlate_icmp(xl, info, EBT_IP_ICMP);
	brip_xlate_igmp(xl, info, EBT_IP_IGMP);

	return 1;
}

static struct xtables_match brip_match = {
	.name		= "ip",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_ip_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_ip_info)),
	.help		= brip_print_help,
	.x6_parse	= brip_parse,
	.x6_fcheck	= brip_final_check,
	.print		= brip_print,
	.xlate		= brip_xlate,
	.x6_options	= brip_opts,
};

void _init(void)
{
	xtables_register_match(&brip_match);
}
