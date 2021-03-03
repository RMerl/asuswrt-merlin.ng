/*
 * iptunnel.c	       "ip tunnel"
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "tunnel.h"

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip tunnel { add | change | del | show | prl | 6rd } [ NAME ]\n"
		"	 [ mode { ipip | gre | sit | isatap | vti } ] [ remote ADDR ] [ local ADDR ]\n"
		"	 [ [i|o]seq ] [ [i|o]key KEY ] [ [i|o]csum ]\n"
		"	 [ prl-default ADDR ] [ prl-nodefault ADDR ] [ prl-delete ADDR ]\n"
		"	 [ 6rd-prefix ADDR ] [ 6rd-relay_prefix ADDR ] [ 6rd-reset ]\n"
		"	 [ ttl TTL ] [ tos TOS ] [ [no]pmtudisc ] [ dev PHYS_DEV ]\n"
		"\n"
		"Where:	NAME := STRING\n"
		"	ADDR := { IP_ADDRESS | any }\n"
		"	TOS  := { STRING | 00..ff | inherit | inherit/STRING | inherit/00..ff }\n"
		"	TTL  := { 1..255 | inherit }\n"
		"	KEY  := { DOTTED_QUAD | NUMBER }\n");
	exit(-1);
}

static void set_tunnel_proto(struct ip_tunnel_parm *p, int proto)
{
	if (p->iph.protocol && p->iph.protocol != proto) {
		fprintf(stderr,
			"You managed to ask for more than one tunnel mode.\n");
		exit(-1);
	}
	p->iph.protocol = proto;
}

static int parse_args(int argc, char **argv, int cmd, struct ip_tunnel_parm *p)
{
	int count = 0;
	const char *medium = NULL;
	int isatap = 0;

	memset(p, 0, sizeof(*p));
	p->iph.version = 4;
	p->iph.ihl = 5;
#ifndef IP_DF
#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#endif
	p->iph.frag_off = htons(IP_DF);

	while (argc > 0) {
		if (strcmp(*argv, "mode") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ipip") == 0 ||
			    strcmp(*argv, "ip/ip") == 0) {
				set_tunnel_proto(p, IPPROTO_IPIP);
			} else if (strcmp(*argv, "gre") == 0 ||
				   strcmp(*argv, "gre/ip") == 0) {
				set_tunnel_proto(p, IPPROTO_GRE);
			} else if (strcmp(*argv, "sit") == 0 ||
				   strcmp(*argv, "ipv6/ip") == 0) {
				set_tunnel_proto(p, IPPROTO_IPV6);
			} else if (strcmp(*argv, "isatap") == 0) {
				set_tunnel_proto(p, IPPROTO_IPV6);
				isatap++;
			} else if (strcmp(*argv, "vti") == 0) {
				set_tunnel_proto(p, IPPROTO_IPIP);
				p->i_flags |= VTI_ISVTI;
			} else {
				fprintf(stderr,
					"Unknown tunnel mode \"%s\"\n", *argv);
				exit(-1);
			}
		} else if (strcmp(*argv, "key") == 0) {
			NEXT_ARG();
			p->i_flags |= GRE_KEY;
			p->o_flags |= GRE_KEY;
			p->i_key = p->o_key = tnl_parse_key("key", *argv);
		} else if (strcmp(*argv, "ikey") == 0) {
			NEXT_ARG();
			p->i_flags |= GRE_KEY;
			p->i_key = tnl_parse_key("ikey", *argv);
		} else if (strcmp(*argv, "okey") == 0) {
			NEXT_ARG();
			p->o_flags |= GRE_KEY;
			p->o_key = tnl_parse_key("okey", *argv);
		} else if (strcmp(*argv, "seq") == 0) {
			p->i_flags |= GRE_SEQ;
			p->o_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "iseq") == 0) {
			p->i_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "oseq") == 0) {
			p->o_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "csum") == 0) {
			p->i_flags |= GRE_CSUM;
			p->o_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "icsum") == 0) {
			p->i_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "ocsum") == 0) {
			p->o_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "nopmtudisc") == 0) {
			p->iph.frag_off = 0;
		} else if (strcmp(*argv, "pmtudisc") == 0) {
			p->iph.frag_off = htons(IP_DF);
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			p->iph.daddr = get_addr32(*argv);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			p->iph.saddr = get_addr32(*argv);
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			medium = *argv;
		} else if (strcmp(*argv, "ttl") == 0 ||
			   strcmp(*argv, "hoplimit") == 0 ||
			   strcmp(*argv, "hlim") == 0) {
			__u8 uval;

			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (get_u8(&uval, *argv, 0))
					invarg("invalid TTL\n", *argv);
				p->iph.ttl = uval;
			}
		} else if (strcmp(*argv, "tos") == 0 ||
			   strcmp(*argv, "tclass") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			char *dsfield;
			__u32 uval;

			NEXT_ARG();
			dsfield = *argv;
			strsep(&dsfield, "/");
			if (strcmp(*argv, "inherit") != 0) {
				dsfield = *argv;
				p->iph.tos = 0;
			} else
				p->iph.tos = 1;
			if (dsfield) {
				if (rtnl_dsfield_a2n(&uval, dsfield))
					invarg("bad TOS value", *argv);
				p->iph.tos |= uval;
			}
		} else {
			if (strcmp(*argv, "name") == 0)
				NEXT_ARG();
			else if (matches(*argv, "help") == 0)
				usage();

			if (p->name[0])
				duparg2("name", *argv);
			if (get_ifname(p->name, *argv))
				invarg("\"name\" not a valid ifname", *argv);
			if (cmd == SIOCCHGTUNNEL && count == 0) {
				struct ip_tunnel_parm old_p = {};

				if (tnl_get_ioctl(*argv, &old_p))
					return -1;
				*p = old_p;
			}
		}
		count++;
		argc--; argv++;
	}


	if (p->iph.protocol == 0) {
		if (memcmp(p->name, "gre", 3) == 0)
			p->iph.protocol = IPPROTO_GRE;
		else if (memcmp(p->name, "ipip", 4) == 0)
			p->iph.protocol = IPPROTO_IPIP;
		else if (memcmp(p->name, "sit", 3) == 0)
			p->iph.protocol = IPPROTO_IPV6;
		else if (memcmp(p->name, "isatap", 6) == 0) {
			p->iph.protocol = IPPROTO_IPV6;
			isatap++;
		} else if (memcmp(p->name, "vti", 3) == 0) {
			p->iph.protocol = IPPROTO_IPIP;
			p->i_flags |= VTI_ISVTI;
		}
	}

	if ((p->i_flags & GRE_KEY) || (p->o_flags & GRE_KEY)) {
		if (!(p->i_flags & VTI_ISVTI) &&
		    (p->iph.protocol != IPPROTO_GRE)) {
			fprintf(stderr, "Keys are not allowed with ipip and sit tunnels\n");
			return -1;
		}
	}

	if (medium) {
		p->link = ll_name_to_index(medium);
		if (!p->link)
			return nodev(medium);
	}

	if (p->i_key == 0 && IN_MULTICAST(ntohl(p->iph.daddr))) {
		p->i_key = p->iph.daddr;
		p->i_flags |= GRE_KEY;
	}
	if (p->o_key == 0 && IN_MULTICAST(ntohl(p->iph.daddr))) {
		p->o_key = p->iph.daddr;
		p->o_flags |= GRE_KEY;
	}
	if (IN_MULTICAST(ntohl(p->iph.daddr)) && !p->iph.saddr) {
		fprintf(stderr, "A broadcast tunnel requires a source address\n");
		return -1;
	}
	if (isatap)
		p->i_flags |= SIT_ISATAP;

	return 0;
}

static const char *tnl_defname(const struct ip_tunnel_parm *p)
{
	switch (p->iph.protocol) {
	case IPPROTO_IPIP:
		if (p->i_flags & VTI_ISVTI)
			return "ip_vti0";
		else
			return "tunl0";
	case IPPROTO_GRE:
		return "gre0";
	case IPPROTO_IPV6:
		return "sit0";
	}
	return NULL;
}

static int do_add(int cmd, int argc, char **argv)
{
	struct ip_tunnel_parm p;
	const char *basedev;

	if (parse_args(argc, argv, cmd, &p) < 0)
		return -1;

	if (p.iph.ttl && p.iph.frag_off == 0) {
		fprintf(stderr, "ttl != 0 and nopmtudisc are incompatible\n");
		return -1;
	}

	basedev = tnl_defname(&p);
	if (!basedev) {
		fprintf(stderr,
			"cannot determine tunnel mode (ipip, gre, vti or sit)\n");
		return -1;
	}

	return tnl_add_ioctl(cmd, basedev, p.name, &p);
}

static int do_del(int argc, char **argv)
{
	struct ip_tunnel_parm p;

	if (parse_args(argc, argv, SIOCDELTUNNEL, &p) < 0)
		return -1;

	return tnl_del_ioctl(tnl_defname(&p) ? : p.name, p.name, &p);
}

static void print_tunnel(const void *t)
{
	const struct ip_tunnel_parm *p = t;
	struct ip_tunnel_6rd ip6rd = {};
	SPRINT_BUF(b1);

	/* Do not use format_host() for local addr,
	 * symbolic name will not be useful.
	 */
	open_json_object(NULL);
	print_color_string(PRINT_ANY, COLOR_IFNAME, "ifname", "%s: ", p->name);
	snprintf(b1, sizeof(b1), "%s/ip", tnl_strproto(p->iph.protocol));
	print_string(PRINT_ANY, "mode", "%s ", b1);
	print_null(PRINT_FP, NULL, "remote ", NULL);
	print_color_string(PRINT_ANY, COLOR_INET, "remote", "%s ",
			   p->iph.daddr || is_json_context()
				? format_host_r(AF_INET, 4, &p->iph.daddr, b1, sizeof(b1))
				: "any");
	print_null(PRINT_FP, NULL, "local ", NULL);
	print_color_string(PRINT_ANY, COLOR_INET, "local", "%s",
			   p->iph.saddr || is_json_context()
				? rt_addr_n2a_r(AF_INET, 4, &p->iph.saddr, b1, sizeof(b1))
				: "any");

	if (p->iph.protocol == IPPROTO_IPV6 && (p->i_flags & SIT_ISATAP)) {
		struct ip_tunnel_prl prl[16] = {};
		int i;

		prl[0].datalen = sizeof(prl) - sizeof(prl[0]);
		prl[0].addr = htonl(INADDR_ANY);

		if (!tnl_prl_ioctl(SIOCGETPRL, p->name, prl)) {
			for (i = 1; i < ARRAY_SIZE(prl); i++) {
				if (prl[i].addr == htonl(INADDR_ANY))
					continue;
				if (prl[i].flags & PRL_DEFAULT)
					print_string(PRINT_ANY, "pdr",
						     " pdr %s",
						     format_host(AF_INET, 4, &prl[i].addr));
				else
					print_string(PRINT_ANY, "pr", " pr %s",
						     format_host(AF_INET, 4, &prl[i].addr));
			}
		}
	}

	if (p->link) {
		const char *n = ll_index_to_name(p->link);

		if (n)
			print_string(PRINT_ANY, "dev", " dev %s", n);
	}

	if (p->iph.ttl)
		print_uint(PRINT_ANY, "ttl", " ttl %u", p->iph.ttl);
	else
		print_string(PRINT_FP, "ttl", " ttl %s", "inherit");

	if (p->iph.tos) {
		SPRINT_BUF(b2);

		if (p->iph.tos != 1) {
			if (!is_json_context() && p->iph.tos & 1)
				snprintf(b2, sizeof(b2), "%s%s",
					 p->iph.tos & 1 ? "inherit/" : "",
					 rtnl_dsfield_n2a(p->iph.tos & ~1, b1, sizeof(b1)));
			else
				snprintf(b2, sizeof(b2), "%s",
					 rtnl_dsfield_n2a(p->iph.tos, b1, sizeof(b1)));
			print_string(PRINT_ANY, "tos", " tos %s", b2);
		} else {
			print_string(PRINT_FP, NULL, " tos %s", "inherit");
		}
	}

	if (!(p->iph.frag_off & htons(IP_DF)))
		print_null(PRINT_ANY, "nopmtudisc", " nopmtudisc", NULL);

	if (p->iph.protocol == IPPROTO_IPV6 && !tnl_ioctl_get_6rd(p->name, &ip6rd) && ip6rd.prefixlen) {
		print_string(PRINT_ANY, "6rd-prefix", " 6rd-prefix %s",
			     inet_ntop(AF_INET6, &ip6rd.prefix, b1, sizeof(b1)));
		print_uint(PRINT_ANY, "6rd-prefixlen", "/%u", ip6rd.prefixlen);
		if (ip6rd.relay_prefix) {
			print_string(PRINT_ANY, "6rd-relay_prefix",
				     " 6rd-relay_prefix %s",
				     format_host(AF_INET, 4, &ip6rd.relay_prefix));
			print_uint(PRINT_ANY, "6rd-relay_prefixlen", "/%u",
				   ip6rd.relay_prefixlen);
		}
	}

	tnl_print_gre_flags(p->iph.protocol, p->i_flags, p->o_flags,
			    p->i_key, p->o_key);

	close_json_object();
}


static void ip_tunnel_parm_initialize(const struct tnl_print_nlmsg_info *info)
{
	struct ip_tunnel_parm *p2 = info->p2;

	memset(p2, 0, sizeof(*p2));
}

static bool ip_tunnel_parm_match(const struct tnl_print_nlmsg_info *info)
{
	const struct ip_tunnel_parm *p1 = info->p1;
	const struct ip_tunnel_parm *p2 = info->p2;

	return ((!p1->link || p1->link == p2->link) &&
		(!p1->name[0] || strcmp(p1->name, p2->name) == 0) &&
		(!p1->iph.daddr || p1->iph.daddr == p2->iph.daddr) &&
		(!p1->iph.saddr || p1->iph.saddr == p2->iph.saddr) &&
		(!p1->i_key || p1->i_key == p2->i_key));
}

static int do_show(int argc, char **argv)
{
	struct ip_tunnel_parm p, p1;
	const char *basedev;

	if (parse_args(argc, argv, SIOCGETTUNNEL, &p) < 0)
		return -1;

	basedev = tnl_defname(&p);
	if (!basedev) {
		struct tnl_print_nlmsg_info info = {
			.p1    = &p,
			.p2    = &p1,
			.init  = ip_tunnel_parm_initialize,
			.match = ip_tunnel_parm_match,
			.print = print_tunnel,
		};

		return do_tunnels_list(&info);
	}

	if (tnl_get_ioctl(p.name[0] ? p.name : basedev, &p))
		return -1;

	print_tunnel(&p);
	fputc('\n', stdout);
	return 0;
}

static int do_prl(int argc, char **argv)
{
	struct ip_tunnel_prl p = {};
	int count = 0;
	int cmd = 0;
	const char *medium = NULL;

	while (argc > 0) {
		if (strcmp(*argv, "prl-default") == 0) {
			NEXT_ARG();
			cmd = SIOCADDPRL;
			p.addr = get_addr32(*argv);
			p.flags |= PRL_DEFAULT;
			count++;
		} else if (strcmp(*argv, "prl-nodefault") == 0) {
			NEXT_ARG();
			cmd = SIOCADDPRL;
			p.addr = get_addr32(*argv);
			count++;
		} else if (strcmp(*argv, "prl-delete") == 0) {
			NEXT_ARG();
			cmd = SIOCDELPRL;
			p.addr = get_addr32(*argv);
			count++;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"dev\" not a valid ifname", *argv);
			medium = *argv;
		} else {
			fprintf(stderr,
				"Invalid PRL parameter \"%s\"\n", *argv);
			exit(-1);
		}
		if (count > 1) {
			fprintf(stderr,
				"One PRL entry at a time\n");
			exit(-1);
		}
		argc--; argv++;
	}
	if (!medium) {
		fprintf(stderr, "Must specify device\n");
		exit(-1);
	}

	return tnl_prl_ioctl(cmd, medium, &p);
}

static int do_6rd(int argc, char **argv)
{
	struct ip_tunnel_6rd ip6rd = {};
	int cmd = 0;
	const char *medium = NULL;
	inet_prefix prefix;

	while (argc > 0) {
		if (strcmp(*argv, "6rd-prefix") == 0) {
			NEXT_ARG();
			if (get_prefix(&prefix, *argv, AF_INET6))
				invarg("invalid 6rd_prefix\n", *argv);
			cmd = SIOCADD6RD;
			memcpy(&ip6rd.prefix, prefix.data, 16);
			ip6rd.prefixlen = prefix.bitlen;
		} else if (strcmp(*argv, "6rd-relay_prefix") == 0) {
			NEXT_ARG();
			if (get_prefix(&prefix, *argv, AF_INET))
				invarg("invalid 6rd-relay_prefix\n", *argv);
			cmd = SIOCADD6RD;
			memcpy(&ip6rd.relay_prefix, prefix.data, 4);
			ip6rd.relay_prefixlen = prefix.bitlen;
		} else if (strcmp(*argv, "6rd-reset") == 0) {
			cmd = SIOCDEL6RD;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"dev\" not a valid ifname", *argv);
			medium = *argv;
		} else {
			fprintf(stderr,
				"Invalid 6RD parameter \"%s\"\n", *argv);
			exit(-1);
		}
		argc--; argv++;
	}
	if (!medium) {
		fprintf(stderr, "Must specify device\n");
		exit(-1);
	}

	return tnl_6rd_ioctl(cmd, medium, &ip6rd);
}

static int tunnel_mode_is_ipv6(char *tunnel_mode)
{
	static const char * const ipv6_modes[] = {
		"ipv6/ipv6", "ip6ip6",
		"vti6",
		"ip/ipv6", "ipv4/ipv6", "ipip6", "ip4ip6",
		"ip6gre", "gre/ipv6",
		"any/ipv6", "any"
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(ipv6_modes); i++) {
		if (strcmp(ipv6_modes[i], tunnel_mode) == 0)
			return 1;
	}
	return 0;
}

int do_iptunnel(int argc, char **argv)
{
	int i;

	for (i = 0; i < argc - 1; i++) {
		if (strcmp(argv[i], "mode") == 0) {
			if (tunnel_mode_is_ipv6(argv[i + 1]))
				preferred_family = AF_INET6;
			break;
		}
	}
	switch (preferred_family) {
	case AF_UNSPEC:
		preferred_family = AF_INET;
		break;
	case AF_INET:
		break;
	/*
	 * This is silly enough but we have no easy way to make it
	 * protocol-independent because of unarranged structure between
	 * IPv4 and IPv6.
	 */
	case AF_INET6:
		return do_ip6tunnel(argc, argv);
	default:
		fprintf(stderr, "Unsupported protocol family: %d\n", preferred_family);
		exit(-1);
	}

	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return do_add(SIOCADDTUNNEL, argc - 1, argv + 1);
		if (matches(*argv, "change") == 0)
			return do_add(SIOCCHGTUNNEL, argc - 1, argv + 1);
		if (matches(*argv, "delete") == 0)
			return do_del(argc - 1, argv + 1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return do_show(argc - 1, argv + 1);
		if (matches(*argv, "prl") == 0)
			return do_prl(argc - 1, argv + 1);
		if (matches(*argv, "6rd") == 0)
			return do_6rd(argc - 1, argv + 1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return do_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip tunnel help\"\n", *argv);
	exit(-1);
}
