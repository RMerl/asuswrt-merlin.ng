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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_ip6.h>



#define IP_SOURCE '1'
#define IP_DEST   '2'
#define IP_TCLASS '3'
#define IP_PROTO  '4'
#define IP_SPORT  '5'
#define IP_DPORT  '6'
#define IP_ICMP6  '7'

static const struct option opts[] =
{
	{ "ip6-source"           , required_argument, 0, IP_SOURCE },
	{ "ip6-src"              , required_argument, 0, IP_SOURCE },
	{ "ip6-destination"      , required_argument, 0, IP_DEST   },
	{ "ip6-dst"              , required_argument, 0, IP_DEST   },
	{ "ip6-traffic-class"    , required_argument, 0, IP_TCLASS },
	{ "ip6-tclass"           , required_argument, 0, IP_TCLASS },
	{ "ip6-protocol"         , required_argument, 0, IP_PROTO  },
	{ "ip6-proto"            , required_argument, 0, IP_PROTO  },
	{ "ip6-source-port"      , required_argument, 0, IP_SPORT  },
	{ "ip6-sport"            , required_argument, 0, IP_SPORT  },
	{ "ip6-destination-port" , required_argument, 0, IP_DPORT  },
	{ "ip6-dport"            , required_argument, 0, IP_DPORT  },
	{ "ip6-icmp-type"	 , required_argument, 0, IP_ICMP6  },
	{ 0 }
};


static const struct ebt_icmp_names icmpv6_codes[] = {
	{ "destination-unreachable", 1, 0, 0xFF },
	{ "no-route", 1, 0, 0 },
	{ "communication-prohibited", 1, 1, 1 },
	{ "address-unreachable", 1, 3, 3 },
	{ "port-unreachable", 1, 4, 4 },

	{ "packet-too-big", 2, 0, 0xFF },

	{ "time-exceeded", 3, 0, 0xFF },
	/* Alias */ { "ttl-exceeded", 3, 0, 0xFF },
	{ "ttl-zero-during-transit", 3, 0, 0 },
	{ "ttl-zero-during-reassembly", 3, 1, 1 },

	{ "parameter-problem", 4, 0, 0xFF },
	{ "bad-header", 4, 0, 0 },
	{ "unknown-header-type", 4, 1, 1 },
	{ "unknown-option", 4, 2, 2 },

	{ "echo-request", 128, 0, 0xFF },
	/* Alias */ { "ping", 128, 0, 0xFF },

	{ "echo-reply", 129, 0, 0xFF },
	/* Alias */ { "pong", 129, 0, 0xFF },

	{ "router-solicitation", 133, 0, 0xFF },

	{ "router-advertisement", 134, 0, 0xFF },

	{ "neighbour-solicitation", 135, 0, 0xFF },
	/* Alias */ { "neighbor-solicitation", 135, 0, 0xFF },

	{ "neighbour-advertisement", 136, 0, 0xFF },
	/* Alias */ { "neighbor-advertisement", 136, 0, 0xFF },

	{ "redirect", 137, 0, 0xFF },
};

/* transform a protocol and service name into a port number */
static uint16_t parse_port(const char *protocol, const char *name)
{
	struct servent *service;
	char *end;
	int port;

	port = strtol(name, &end, 10);
	if (*end != '\0') {
		if (protocol && 
		    (service = getservbyname(name, protocol)) != NULL)
			return ntohs(service->s_port);
	}
	else if (port >= 0 || port <= 0xFFFF) {
		return port;
	}
	ebt_print_error("Problem with specified %s port '%s'", 
			protocol?protocol:"", name);
	return 0;
}

static void
parse_port_range(const char *protocol, const char *portstring, uint16_t *ports)
{
	char *buffer;
	char *cp;
	
	buffer = strdup(portstring);
	if ((cp = strchr(buffer, ':')) == NULL)
		ports[0] = ports[1] = parse_port(protocol, buffer);
	else {
		*cp = '\0';
		cp++;
		ports[0] = buffer[0] ? parse_port(protocol, buffer) : 0;
		if (ebt_errormsg[0] != '\0')
			return;
		ports[1] = cp[0] ? parse_port(protocol, cp) : 0xFFFF;
		if (ebt_errormsg[0] != '\0')
			return;
		
		if (ports[0] > ports[1])
			ebt_print_error("Invalid portrange (min > max)");
	}
	free(buffer);
}

static void print_port_range(uint16_t *ports)
{
	if (ports[0] == ports[1])
		printf("%d ", ports[0]);
	else
		printf("%d:%d ", ports[0], ports[1]);
}

static void print_help()
{
	printf(
"ip6 options:\n"
"--ip6-src    [!] address[/mask]: ipv6 source specification\n"
"--ip6-dst    [!] address[/mask]: ipv6 destination specification\n"
"--ip6-tclass [!] tclass        : ipv6 traffic class specification\n"
"--ip6-proto  [!] protocol      : ipv6 protocol specification\n"
"--ip6-sport  [!] port[:port]   : tcp/udp source port or port range\n"
"--ip6-dport  [!] port[:port]   : tcp/udp destination port or port range\n"
"--ip6-icmp-type [!] type[[:type]/code[:code]] : ipv6-icmp type/code or type/code range\n");

	printf("\nValid ICMPv6 Types:\n");
	ebt_print_icmp_types(icmpv6_codes, ARRAY_SIZE(icmpv6_codes));
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	ipinfo->invflags = 0;
	ipinfo->bitmask = 0;
	memset(ipinfo->saddr.s6_addr, 0, sizeof(ipinfo->saddr.s6_addr));
	memset(ipinfo->smsk.s6_addr, 0, sizeof(ipinfo->smsk.s6_addr));
	memset(ipinfo->daddr.s6_addr, 0, sizeof(ipinfo->daddr.s6_addr));
	memset(ipinfo->dmsk.s6_addr, 0, sizeof(ipinfo->dmsk.s6_addr));
}

#define OPT_SOURCE 0x01
#define OPT_DEST   0x02
#define OPT_TCLASS 0x04
#define OPT_PROTO  0x08
#define OPT_SPORT  0x10
#define OPT_DPORT  0x20
static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)(*match)->data;
	char *end;
	long int i;

	switch (c) {
	case IP_SOURCE:
		ebt_check_option2(flags, OPT_SOURCE);
		ipinfo->bitmask |= EBT_IP6_SOURCE;
		if (ebt_check_inverse2(optarg)) {
		    ipinfo->invflags |= EBT_IP6_SOURCE;
		}
		ebt_parse_ip6_address(optarg, &ipinfo->saddr, &ipinfo->smsk);
		break;

	case IP_DEST:
		ebt_check_option2(flags, OPT_DEST);
		ipinfo->bitmask |= EBT_IP6_DEST;
		if (ebt_check_inverse2(optarg)) {
			ipinfo->invflags |= EBT_IP6_DEST;
		}
		ebt_parse_ip6_address(optarg, &ipinfo->daddr, &ipinfo->dmsk);
		break;

	case IP_SPORT:
	case IP_DPORT:
		if (c == IP_SPORT) {
			ebt_check_option2(flags, OPT_SPORT);
			ipinfo->bitmask |= EBT_IP6_SPORT;
			if (ebt_check_inverse2(optarg))
				ipinfo->invflags |= EBT_IP6_SPORT;
		} else {
			ebt_check_option2(flags, OPT_DPORT);
			ipinfo->bitmask |= EBT_IP6_DPORT;
			if (ebt_check_inverse2(optarg))
				ipinfo->invflags |= EBT_IP6_DPORT;
		}
		if (c == IP_SPORT)
			parse_port_range(NULL, optarg, ipinfo->sport);
		else
			parse_port_range(NULL, optarg, ipinfo->dport);
		break;

	case IP_ICMP6:
		ebt_check_option2(flags, EBT_IP6_ICMP6);
		ipinfo->bitmask |= EBT_IP6_ICMP6;
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_ICMP6;
		if (ebt_parse_icmp(icmpv6_codes, ARRAY_SIZE(icmpv6_codes),
				   optarg, ipinfo->icmpv6_type,
				   ipinfo->icmpv6_code))
			return 0;
		break;

	case IP_TCLASS:
		ebt_check_option2(flags, OPT_TCLASS);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_TCLASS;
		i = strtol(optarg, &end, 16);
		if (i < 0 || i > 255 || *end != '\0')
			ebt_print_error2("Problem with specified IPv6 traffic class");
		ipinfo->tclass = i;
		ipinfo->bitmask |= EBT_IP6_TCLASS;
		break;

	case IP_PROTO:
		ebt_check_option2(flags, OPT_PROTO);
		if (ebt_check_inverse2(optarg))
			ipinfo->invflags |= EBT_IP6_PROTO;
		i = strtoul(optarg, &end, 10);
		if (*end != '\0') {
			struct protoent *pe;

			pe = getprotobyname(optarg);
			if (pe == NULL)
				ebt_print_error("Unknown specified IP protocol - %s", argv[optind - 1]);
			ipinfo->protocol = pe->p_proto;
		} else {
			ipinfo->protocol = (unsigned char) i;
		}
		ipinfo->bitmask |= EBT_IP6_PROTO;
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
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (entry->ethproto != ETH_P_IPV6 || entry->invflags & EBT_IPROTO) {
		ebt_print_error("For IPv6 filtering the protocol must be "
		            "specified as IPv6");
	} else if (ipinfo->bitmask & (EBT_IP6_SPORT|EBT_IP6_DPORT) &&
		(!(ipinfo->bitmask & EBT_IP6_PROTO) ||
		ipinfo->invflags & EBT_IP6_PROTO ||
		(ipinfo->protocol!=IPPROTO_TCP &&
		 ipinfo->protocol!=IPPROTO_UDP &&
		 ipinfo->protocol!=IPPROTO_SCTP &&
		 ipinfo->protocol!=IPPROTO_DCCP)))
		ebt_print_error("For port filtering the IP protocol must be "
				"either 6 (tcp), 17 (udp), 33 (dccp) or "
				"132 (sctp)");
	if ((ipinfo->bitmask & EBT_IP6_ICMP6) &&
	  (!(ipinfo->bitmask & EBT_IP6_PROTO) ||
	     ipinfo->invflags & EBT_IP6_PROTO ||
	     ipinfo->protocol != IPPROTO_ICMPV6))
		ebt_print_error("For ipv6-icmp filtering the IP protocol must be "
				"58 (ipv6-icmp)");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (ipinfo->bitmask & EBT_IP6_SOURCE) {
		printf("--ip6-src ");
		if (ipinfo->invflags & EBT_IP6_SOURCE)
			printf("! ");
		printf("%s", ebt_ip6_to_numeric(&ipinfo->saddr));
		printf("%s ", ebt_ip6_mask_to_string(&ipinfo->smsk));
	}
	if (ipinfo->bitmask & EBT_IP6_DEST) {
		printf("--ip6-dst ");
		if (ipinfo->invflags & EBT_IP6_DEST)
			printf("! ");
		printf("%s", ebt_ip6_to_numeric(&ipinfo->daddr));
		printf("%s ", ebt_ip6_mask_to_string(&ipinfo->dmsk));
	}
	if (ipinfo->bitmask & EBT_IP6_TCLASS) {
		printf("--ip6-tclass ");
		if (ipinfo->invflags & EBT_IP6_TCLASS)
			printf("! ");
		printf("0x%02X ", ipinfo->tclass);
	}
	if (ipinfo->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		printf("--ip6-proto ");
		if (ipinfo->invflags & EBT_IP6_PROTO)
			printf("! ");
		pe = getprotobynumber(ipinfo->protocol);
		if (pe == NULL) {
			printf("%d ", ipinfo->protocol);
		} else {
			printf("%s ", pe->p_name);
		}
	}
	if (ipinfo->bitmask & EBT_IP6_SPORT) {
		printf("--ip6-sport ");
		if (ipinfo->invflags & EBT_IP6_SPORT)
			printf("! ");
		print_port_range(ipinfo->sport);
	}
	if (ipinfo->bitmask & EBT_IP6_DPORT) {
		printf("--ip6-dport ");
		if (ipinfo->invflags & EBT_IP6_DPORT)
			printf("! ");
		print_port_range(ipinfo->dport);
	}
	if (ipinfo->bitmask & EBT_IP6_ICMP6) {
		printf("--ip6-icmp-type ");
		if (ipinfo->invflags & EBT_IP6_ICMP6)
			printf("! ");
		ebt_print_icmp_type(icmpv6_codes, ARRAY_SIZE(icmpv6_codes),
				    ipinfo->icmpv6_type, ipinfo->icmpv6_code);
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_ip6_info *ipinfo1 = (struct ebt_ip6_info *)m1->data;
	struct ebt_ip6_info *ipinfo2 = (struct ebt_ip6_info *)m2->data;

	if (ipinfo1->bitmask != ipinfo2->bitmask)
		return 0;
	if (ipinfo1->invflags != ipinfo2->invflags)
		return 0;
	if (ipinfo1->bitmask & EBT_IP6_SOURCE) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->saddr, &ipinfo2->saddr))
			return 0;
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->smsk, &ipinfo2->smsk))
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_DEST) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->daddr, &ipinfo2->daddr))
			return 0;
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->dmsk, &ipinfo2->dmsk))
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_TCLASS) {
		if (ipinfo1->tclass != ipinfo2->tclass)
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_PROTO) {
		if (ipinfo1->protocol != ipinfo2->protocol)
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_SPORT) {
		if (ipinfo1->sport[0] != ipinfo2->sport[0] ||
		   ipinfo1->sport[1] != ipinfo2->sport[1])
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_DPORT) {
		if (ipinfo1->dport[0] != ipinfo2->dport[0] ||
		   ipinfo1->dport[1] != ipinfo2->dport[1])
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_ICMP6) {
		if (ipinfo1->icmpv6_type[0] != ipinfo2->icmpv6_type[0] ||
		    ipinfo1->icmpv6_type[1] != ipinfo2->icmpv6_type[1] ||
		    ipinfo1->icmpv6_code[0] != ipinfo2->icmpv6_code[0] ||
		    ipinfo1->icmpv6_code[1] != ipinfo2->icmpv6_code[1])
			return 0;
	}
	return 1;
}

static struct ebt_u_match ip6_match =
{
	.name		= EBT_IP6_MATCH,
	.size		= sizeof(struct ebt_ip6_info),
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
	ebt_register_match(&ip6_match);
}
