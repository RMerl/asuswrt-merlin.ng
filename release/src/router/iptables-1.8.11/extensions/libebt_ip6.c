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
 */

#include <errno.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_ip6.h>

#include "libxt_icmp.h"

/* must correspond to the bit position in EBT_IP6_* defines */
enum {
	O_SOURCE = 0,
	O_DEST,
	O_TCLASS,
	O_PROTO,
	O_SPORT,
	O_DPORT,
	O_ICMP6,
	F_PORT = 1 << O_ICMP6,
	F_ICMP6 = 1 << O_SPORT | 1 << O_DPORT,
};

static const struct xt_option_entry brip6_opts[] = {
	{ .name = "ip6-source",		.id = O_SOURCE, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip6-src",		.id = O_SOURCE, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip6-destination",	.id = O_DEST, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip6-dst",		.id = O_DEST, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_INVERT },
	{ .name = "ip6-tclass",		.id = O_TCLASS, .type = XTTYPE_UINT8,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, tclass) },
	{ .name = "ip6-protocol",	.id = O_PROTO, .type = XTTYPE_PROTOCOL,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, protocol) },
	{ .name = "ip6-proto",		.id = O_PROTO, .type = XTTYPE_PROTOCOL,
	  .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, protocol) },
	{ .name = "ip6-source-port",	.id = O_SPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, sport) },
	{ .name = "ip6-sport",		.id = O_SPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, sport) },
	{ .name = "ip6-destination-port",.id = O_DPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, dport) },
	{ .name = "ip6-dport",		.id = O_DPORT, .type = XTTYPE_PORTRC,
	  .excl = F_PORT, .flags = XTOPT_INVERT | XTOPT_PUT,
	  XTOPT_POINTER(struct ebt_ip6_info, dport) },
	{ .name = "ip6-icmp-type",	.id = O_ICMP6, .type = XTTYPE_STRING,
	  .excl = F_ICMP6, .flags = XTOPT_INVERT },
	XTOPT_TABLEEND,
};

static void print_port_range(uint16_t *ports)
{
	if (ports[0] == ports[1])
		printf("%d ", ports[0]);
	else
		printf("%d:%d ", ports[0], ports[1]);
}

static void print_icmp_code(uint8_t *code)
{
	if (code[0] == code[1])
		printf("/%"PRIu8 " ", code[0]);
	else
		printf("/%"PRIu8":%"PRIu8 " ", code[0], code[1]);
}

static void print_icmp_type(uint8_t *type, uint8_t *code)
{
	unsigned int i;

	if (type[0] != type[1]) {
		printf("%"PRIu8 ":%" PRIu8, type[0], type[1]);
		print_icmp_code(code);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(icmpv6_codes); i++) {
		if (icmpv6_codes[i].type != type[0])
			continue;

		if (icmpv6_codes[i].code_min == code[0] &&
		    icmpv6_codes[i].code_max == code[1]) {
			printf("%s ", icmpv6_codes[i].name);
			return;
		}
	}
	printf("%"PRIu8, type[0]);
	print_icmp_code(code);
}

static void brip6_print_help(void)
{
	printf(
"ip6 options:\n"
"[!] --ip6-src    address[/mask]: ipv6 source specification\n"
"[!] --ip6-dst    address[/mask]: ipv6 destination specification\n"
"[!] --ip6-tclass tclass        : ipv6 traffic class specification\n"
"[!] --ip6-proto  protocol      : ipv6 protocol specification\n"
"[!] --ip6-sport  port[:port]   : tcp/udp source port or port range\n"
"[!] --ip6-dport  port[:port]   : tcp/udp destination port or port range\n"
"[!] --ip6-icmp-type type[[:type]/code[:code]] : ipv6-icmp type/code or type/code range\n");
	printf("Valid ICMPv6 Types:");
	xt_print_icmp_types(icmpv6_codes, ARRAY_SIZE(icmpv6_codes));
}

static void brip6_parse(struct xt_option_call *cb)
{
	struct ebt_ip6_info *info = cb->data;
	unsigned int i;

	/* XXX: overriding afinfo family is dangerous, but
	 *      required for XTTYPE_HOSTMASK parsing */
	xtables_set_nfproto(NFPROTO_IPV6);
	xtables_option_parse(cb);
	xtables_set_nfproto(NFPROTO_BRIDGE);

	info->bitmask |= 1 << cb->entry->id;
	info->invflags |= cb->invert ? 1 << cb->entry->id : 0;

	switch (cb->entry->id) {
	case O_SOURCE:
		for (i = 0; i < ARRAY_SIZE(cb->val.haddr.all); i++)
			cb->val.haddr.all[i] &= cb->val.hmask.all[i];
		info->saddr = cb->val.haddr.in6;
		info->smsk = cb->val.hmask.in6;
		break;
	case O_DEST:
		for (i = 0; i < ARRAY_SIZE(cb->val.haddr.all); i++)
			cb->val.haddr.all[i] &= cb->val.hmask.all[i];
		info->daddr = cb->val.haddr.in6;
		info->dmsk = cb->val.hmask.in6;
		break;
	case O_ICMP6:
		ebt_parse_icmpv6(cb->arg, info->icmpv6_type, info->icmpv6_code);
		break;
	}
}

static void brip6_final_check(struct xt_fcheck_call *fc)
{
	if (!fc->xflags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brip6_print(const void *ip, const struct xt_entry_match *match,
		       int numeric)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (ipinfo->bitmask & EBT_IP6_SOURCE) {
		if (ipinfo->invflags & EBT_IP6_SOURCE)
			printf("! ");
		printf("--ip6-src ");
		printf("%s", xtables_ip6addr_to_numeric(&ipinfo->saddr));
		printf("%s ", xtables_ip6mask_to_numeric(&ipinfo->smsk));
	}
	if (ipinfo->bitmask & EBT_IP6_DEST) {
		if (ipinfo->invflags & EBT_IP6_DEST)
			printf("! ");
		printf("--ip6-dst ");
		printf("%s", xtables_ip6addr_to_numeric(&ipinfo->daddr));
		printf("%s ", xtables_ip6mask_to_numeric(&ipinfo->dmsk));
	}
	if (ipinfo->bitmask & EBT_IP6_TCLASS) {
		if (ipinfo->invflags & EBT_IP6_TCLASS)
			printf("! ");
		printf("--ip6-tclass 0x%02X ", ipinfo->tclass);
	}
	if (ipinfo->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		if (ipinfo->invflags & EBT_IP6_PROTO)
			printf("! ");
		printf("--ip6-proto ");
		pe = getprotobynumber(ipinfo->protocol);
		if (pe == NULL) {
			printf("%d ", ipinfo->protocol);
		} else {
			printf("%s ", pe->p_name);
		}
	}
	if (ipinfo->bitmask & EBT_IP6_SPORT) {
		if (ipinfo->invflags & EBT_IP6_SPORT)
			printf("! ");
		printf("--ip6-sport ");
		print_port_range(ipinfo->sport);
	}
	if (ipinfo->bitmask & EBT_IP6_DPORT) {
		if (ipinfo->invflags & EBT_IP6_DPORT)
			printf("! ");
		printf("--ip6-dport ");
		print_port_range(ipinfo->dport);
	}
	if (ipinfo->bitmask & EBT_IP6_ICMP6) {
		if (ipinfo->invflags & EBT_IP6_ICMP6)
			printf("! ");
		printf("--ip6-icmp-type ");
		print_icmp_type(ipinfo->icmpv6_type, ipinfo->icmpv6_code);
	}
}

static void brip_xlate_th(struct xt_xlate *xl,
			  const struct ebt_ip6_info *info, int bit,
			  const char *pname)
{
	const uint16_t *ports;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP6_SPORT:
		if (pname)
			xt_xlate_add(xl, "%s sport ", pname);
		else
			xt_xlate_add(xl, "@th,0,16 ");

		ports = info->sport;
		break;
	case EBT_IP6_DPORT:
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
			  const struct ebt_ip6_info *info, int bit)
{
	struct in6_addr *addrp, *maskp;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP6_SOURCE:
		xt_xlate_add(xl, "ip6 saddr ");
		addrp = (struct in6_addr *)&info->saddr;
		maskp = (struct in6_addr *)&info->smsk;
		break;
	case EBT_IP6_DEST:
		xt_xlate_add(xl, "ip6 daddr ");
		addrp = (struct in6_addr *)&info->daddr;
		maskp = (struct in6_addr *)&info->dmsk;
		break;
	default:
		return;
	}

	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");

	xt_xlate_add(xl, "%s%s ", xtables_ip6addr_to_numeric(addrp),
				  xtables_ip6mask_to_numeric(maskp));
}

static const char *brip6_xlate_proto_to_name(uint8_t proto)
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

static int brip6_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct ebt_ip6_info *info = (const void *)params->match->data;
	const char *pname = NULL;

	if ((info->bitmask & (EBT_IP6_SOURCE|EBT_IP6_DEST|EBT_IP6_ICMP6|EBT_IP6_TCLASS)) == 0)
		xt_xlate_add(xl, "ether type ip6 ");

	brip_xlate_nh(xl, info, EBT_IP6_SOURCE);
	brip_xlate_nh(xl, info, EBT_IP6_DEST);

	if (info->bitmask & EBT_IP6_TCLASS) {
		xt_xlate_add(xl, "ip6 dscp ");
		if (info->invflags & EBT_IP6_TCLASS)
			xt_xlate_add(xl, "!= ");
		xt_xlate_add(xl, "0x%02x ", info->tclass & 0x3f); /* remove ECN bits */
	}

	if (info->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		if (info->bitmask & (EBT_IP6_SPORT|EBT_IP6_DPORT|EBT_IP6_ICMP6) &&
		    (info->invflags & EBT_IP6_PROTO) == 0) {
			/* port number given and not inverted, no need to
			 * add explicit 'meta l4proto'.
			 */
			pname = brip6_xlate_proto_to_name(info->protocol);
		} else {
			xt_xlate_add(xl, "meta l4proto ");
			if (info->invflags & EBT_IP6_PROTO)
				xt_xlate_add(xl, "!= ");
			pe = getprotobynumber(info->protocol);
			if (pe == NULL)
				xt_xlate_add(xl, "%d ", info->protocol);
			else
				xt_xlate_add(xl, "%s ", pe->p_name);
		}
	}

	brip_xlate_th(xl, info, EBT_IP6_SPORT, pname);
	brip_xlate_th(xl, info, EBT_IP6_DPORT, pname);

	if (info->bitmask & EBT_IP6_ICMP6) {
		xt_xlate_add(xl, "icmpv6 type ");
		if (info->invflags & EBT_IP6_ICMP6)
			xt_xlate_add(xl, "!= ");

		if (info->icmpv6_type[0] == info->icmpv6_type[1])
			xt_xlate_add(xl, "%d ", info->icmpv6_type[0]);
		else
			xt_xlate_add(xl, "%d-%d ", info->icmpv6_type[0],
						   info->icmpv6_type[1]);

		if (info->icmpv6_code[0] == 0 &&
		    info->icmpv6_code[1] == 0xff)
			return 1;

		xt_xlate_add(xl, "icmpv6 code ");
		if (info->invflags & EBT_IP6_ICMP6)
			xt_xlate_add(xl, "!= ");

		if (info->icmpv6_code[0] == info->icmpv6_code[1])
			xt_xlate_add(xl, "%d ", info->icmpv6_code[0]);
		else
			xt_xlate_add(xl, "%d-%d ", info->icmpv6_code[0],
						   info->icmpv6_code[1]);
	}

	return 1;
}

static struct xtables_match brip6_match = {
	.name		= "ip6",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_ip6_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_ip6_info)),
	.help		= brip6_print_help,
	.x6_parse	= brip6_parse,
	.x6_fcheck	= brip6_final_check,
	.print		= brip6_print,
	.xlate		= brip6_xlate,
	.x6_options	= brip6_opts,
};

void _init(void)
{
	xtables_register_match(&brip6_match);
}
