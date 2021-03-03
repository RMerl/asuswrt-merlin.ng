/*
 * Copyright (C)2006 USAGI/WIDE Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */
/*
 * Author:
 *	Masahide NAKAMURA @USAGI
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/ip.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tunnel.h>
#include <linux/ip6_tunnel.h>

#include "utils.h"
#include "tunnel.h"
#include "ip_common.h"

#define IP6_FLOWINFO_TCLASS	htonl(0x0FF00000)
#define IP6_FLOWINFO_FLOWLABEL	htonl(0x000FFFFF)

#define DEFAULT_TNL_HOP_LIMIT	(64)

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip -f inet6 tunnel { add | change | del | show } [ NAME ]\n"
		"          [ mode { ip6ip6 | ipip6 | ip6gre | vti6 | any } ]\n"
		"          [ remote ADDR local ADDR ] [ dev PHYS_DEV ]\n"
		"          [ encaplimit ELIM ]\n"
		"          [ hoplimit TTL ] [ tclass TCLASS ] [ flowlabel FLOWLABEL ]\n"
		"          [ dscp inherit ]\n"
		"          [ [no]allow-localremote ]\n"
		"          [ [i|o]seq ] [ [i|o]key KEY ] [ [i|o]csum ]\n"
		"\n"
		"Where: NAME      := STRING\n"
		"       ADDR      := IPV6_ADDRESS\n"
		"       ELIM      := { none | 0..255 }(default=%d)\n"
		"       TTL       := 0..255 (default=%d)\n"
		"       TCLASS    := { 0x0..0xff | inherit }\n"
		"       FLOWLABEL := { 0x0..0xfffff | inherit }\n"
		"       KEY       := { DOTTED_QUAD | NUMBER }\n",
		IPV6_DEFAULT_TNL_ENCAP_LIMIT,
		DEFAULT_TNL_HOP_LIMIT);
	exit(-1);
}

static void print_tunnel(const void *t)
{
	const struct ip6_tnl_parm2 *p = t;
	SPRINT_BUF(b1);

	/* Do not use format_host() for local addr,
	 * symbolic name will not be useful.
	 */
	open_json_object(NULL);
	print_color_string(PRINT_ANY, COLOR_IFNAME, "ifname", "%s: ", p->name);
	snprintf(b1, sizeof(b1), "%s/ipv6", tnl_strproto(p->proto));
	print_string(PRINT_ANY, "mode", "%s ", b1);
	print_string(PRINT_FP, NULL, "%s", "remote ");
	print_color_string(PRINT_ANY, COLOR_INET6, "remote", "%s ",
			   format_host_r(AF_INET6, 16, &p->raddr, b1, sizeof(b1)));
	print_string(PRINT_FP, NULL, "%s", "local ");
	print_color_string(PRINT_ANY, COLOR_INET6, "local", "%s",
			   rt_addr_n2a_r(AF_INET6, 16, &p->laddr, b1, sizeof(b1)));

	if (p->link) {
		const char *n = ll_index_to_name(p->link);

		if (n)
			print_string(PRINT_ANY, "link", " dev %s", n);
	}

	if (p->flags & IP6_TNL_F_IGN_ENCAP_LIMIT)
		print_null(PRINT_ANY, "ip6_tnl_f_ign_encap_limit",
			   " encaplimit none", NULL);
	else
		print_uint(PRINT_ANY, "encap_limit", " encaplimit %u",
			   p->encap_limit);

	if (p->hop_limit)
		print_uint(PRINT_ANY, "hoplimit", " hoplimit %u", p->hop_limit);
	else
		print_string(PRINT_FP, "hoplimit", " hoplimit %s", "inherit");

	if (p->flags & IP6_TNL_F_USE_ORIG_TCLASS) {
		print_null(PRINT_ANY, "ip6_tnl_f_use_orig_tclass",
			   " tclass inherit", NULL);
	} else {
		__u32 val = ntohl(p->flowinfo & IP6_FLOWINFO_TCLASS);

		snprintf(b1, sizeof(b1), "0x%02x", (__u8)(val >> 20));
		print_string(PRINT_ANY, "tclass", " tclass %s", b1);
	}

	if (p->flags & IP6_TNL_F_USE_ORIG_FLOWLABEL) {
		print_null(PRINT_ANY, "ip6_tnl_f_use_orig_flowlabel",
			   " flowlabel inherit", NULL);
	} else {
		__u32 val = ntohl(p->flowinfo & IP6_FLOWINFO_FLOWLABEL);

		snprintf(b1, sizeof(b1), "0x%05x", val);
		print_string(PRINT_ANY, "flowlabel", " flowlabel %s", b1);
	}

	snprintf(b1, sizeof(b1), "0x%08x", ntohl(p->flowinfo));
	print_string(PRINT_ANY, "flowinfo", " (flowinfo %s)", b1);

	if (p->flags & IP6_TNL_F_RCV_DSCP_COPY)
		print_null(PRINT_ANY, "ip6_tnl_f_rcv_dscp_copy",
			   " dscp inherit", NULL);

	if (p->flags & IP6_TNL_F_ALLOW_LOCAL_REMOTE)
		print_null(PRINT_ANY, "ip6_tnl_f_allow_local_remote",
			   " allow-localremote", NULL);

	tnl_print_gre_flags(p->proto, p->i_flags, p->o_flags,
			    p->i_key, p->o_key);

	close_json_object();
}

static int parse_args(int argc, char **argv, int cmd, struct ip6_tnl_parm2 *p)
{
	int count = 0;
	const char *medium = NULL;

	while (argc > 0) {
		if (strcmp(*argv, "mode") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ipv6/ipv6") == 0 ||
			    strcmp(*argv, "ip6ip6") == 0)
				p->proto = IPPROTO_IPV6;
			else if (strcmp(*argv, "vti6") == 0) {
				p->proto = IPPROTO_IPV6;
				p->i_flags |= VTI_ISVTI;
			} else if (strcmp(*argv, "ip/ipv6") == 0 ||
				 strcmp(*argv, "ipv4/ipv6") == 0 ||
				 strcmp(*argv, "ipip6") == 0 ||
				 strcmp(*argv, "ip4ip6") == 0)
				p->proto = IPPROTO_IPIP;
			else if (strcmp(*argv, "ip6gre") == 0 ||
				 strcmp(*argv, "gre/ipv6") == 0)
				p->proto = IPPROTO_GRE;
			else if (strcmp(*argv, "any/ipv6") == 0 ||
				 strcmp(*argv, "any") == 0)
				p->proto = 0;
			else {
				fprintf(stderr, "Unknown tunnel mode \"%s\"\n", *argv);
				exit(-1);
			}
		} else if (strcmp(*argv, "remote") == 0) {
			inet_prefix raddr;

			NEXT_ARG();
			get_addr(&raddr, *argv, AF_INET6);
			memcpy(&p->raddr, &raddr.data, sizeof(p->raddr));
		} else if (strcmp(*argv, "local") == 0) {
			inet_prefix laddr;

			NEXT_ARG();
			get_addr(&laddr, *argv, AF_INET6);
			memcpy(&p->laddr, &laddr.data, sizeof(p->laddr));
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			medium = *argv;
		} else if (strcmp(*argv, "encaplimit") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "none") == 0) {
				p->flags |= IP6_TNL_F_IGN_ENCAP_LIMIT;
			} else {
				__u8 uval;

				if (get_u8(&uval, *argv, 0) < -1)
					invarg("invalid ELIM", *argv);
				p->encap_limit = uval;
				p->flags &= ~IP6_TNL_F_IGN_ENCAP_LIMIT;
			}
		} else if (strcmp(*argv, "hoplimit") == 0 ||
			   strcmp(*argv, "ttl") == 0 ||
			   strcmp(*argv, "hlim") == 0) {
			__u8 uval;

			NEXT_ARG();
			if (get_u8(&uval, *argv, 0))
				invarg("invalid TTL", *argv);
			p->hop_limit = uval;
		} else if (strcmp(*argv, "tclass") == 0 ||
			   strcmp(*argv, "tc") == 0 ||
			   strcmp(*argv, "tos") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			__u8 uval;

			NEXT_ARG();
			p->flowinfo &= ~IP6_FLOWINFO_TCLASS;
			if (strcmp(*argv, "inherit") == 0)
				p->flags |= IP6_TNL_F_USE_ORIG_TCLASS;
			else {
				if (get_u8(&uval, *argv, 16))
					invarg("invalid TClass", *argv);
				p->flowinfo |= htonl((__u32)uval << 20) & IP6_FLOWINFO_TCLASS;
				p->flags &= ~IP6_TNL_F_USE_ORIG_TCLASS;
			}
		} else if (strcmp(*argv, "flowlabel") == 0 ||
			   strcmp(*argv, "fl") == 0) {
			__u32 uval;

			NEXT_ARG();
			p->flowinfo &= ~IP6_FLOWINFO_FLOWLABEL;
			if (strcmp(*argv, "inherit") == 0)
				p->flags |= IP6_TNL_F_USE_ORIG_FLOWLABEL;
			else {
				if (get_u32(&uval, *argv, 16))
					invarg("invalid Flowlabel", *argv);
				if (uval > 0xFFFFF)
					invarg("invalid Flowlabel", *argv);
				p->flowinfo |= htonl(uval) & IP6_FLOWINFO_FLOWLABEL;
				p->flags &= ~IP6_TNL_F_USE_ORIG_FLOWLABEL;
			}
		} else if (strcmp(*argv, "dscp") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0)
				invarg("not inherit", *argv);
			p->flags |= IP6_TNL_F_RCV_DSCP_COPY;
		} else if (strcmp(*argv, "allow-localremote") == 0) {
			p->flags |= IP6_TNL_F_ALLOW_LOCAL_REMOTE;
		} else if (strcmp(*argv, "noallow-localremote") == 0) {
			p->flags &= ~IP6_TNL_F_ALLOW_LOCAL_REMOTE;
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
		} else {
			if (strcmp(*argv, "name") == 0) {
				NEXT_ARG();
			} else if (matches(*argv, "help") == 0)
				usage();
			if (p->name[0])
				duparg2("name", *argv);
			if (get_ifname(p->name, *argv))
				invarg("\"name\" not a valid ifname", *argv);
			if (cmd == SIOCCHGTUNNEL && count == 0) {
				struct ip6_tnl_parm2 old_p = {};

				if (tnl_get_ioctl(*argv, &old_p))
					return -1;
				*p = old_p;
			}
		}
		count++;
		argc--; argv++;
	}
	if (medium) {
		p->link = ll_name_to_index(medium);
		if (!p->link)
			return nodev(medium);
	}
	return 0;
}

static void ip6_tnl_parm_init(struct ip6_tnl_parm2 *p, int apply_default)
{
	memset(p, 0, sizeof(*p));
	p->proto = IPPROTO_IPV6;
	if (apply_default) {
		p->hop_limit = DEFAULT_TNL_HOP_LIMIT;
		p->encap_limit = IPV6_DEFAULT_TNL_ENCAP_LIMIT;
	}
}

static void ip6_tnl_parm_initialize(const struct tnl_print_nlmsg_info *info)
{
	const struct ifinfomsg *ifi = info->ifi;
	const struct ip6_tnl_parm2 *p1 = info->p1;
	struct ip6_tnl_parm2 *p2 = info->p2;

	ip6_tnl_parm_init(p2, 0);
	if (ifi->ifi_type == ARPHRD_IP6GRE)
		p2->proto = IPPROTO_GRE;
	p2->link = ifi->ifi_index;
	strcpy(p2->name, p1->name);
}

static bool ip6_tnl_parm_match(const struct tnl_print_nlmsg_info *info)
{
	const struct ip6_tnl_parm2 *p1 = info->p1;
	const struct ip6_tnl_parm2 *p2 = info->p2;

	return ((!p1->link || p1->link == p2->link) &&
		(!p1->name[0] || strcmp(p1->name, p2->name) == 0) &&
		(IN6_IS_ADDR_UNSPECIFIED(&p1->laddr) ||
		 IN6_ARE_ADDR_EQUAL(&p1->laddr, &p2->laddr)) &&
		(IN6_IS_ADDR_UNSPECIFIED(&p1->raddr) ||
		 IN6_ARE_ADDR_EQUAL(&p1->raddr, &p2->raddr)) &&
		(!p1->proto || !p2->proto || p1->proto == p2->proto) &&
		(!p1->encap_limit || p1->encap_limit == p2->encap_limit) &&
		(!p1->hop_limit || p1->hop_limit == p2->hop_limit) &&
		(!(p1->flowinfo & IP6_FLOWINFO_TCLASS) ||
		 !((p1->flowinfo ^ p2->flowinfo) & IP6_FLOWINFO_TCLASS)) &&
		(!(p1->flowinfo & IP6_FLOWINFO_FLOWLABEL) ||
		 !((p1->flowinfo ^ p2->flowinfo) & IP6_FLOWINFO_FLOWLABEL)) &&
		(!p1->flags || (p1->flags & p2->flags)));
}

static int do_show(int argc, char **argv)
{
	struct ip6_tnl_parm2 p, p1;

	ip6_tnl_parm_init(&p, 0);
	p.proto = 0;  /* default to any */

	if (parse_args(argc, argv, SIOCGETTUNNEL, &p) < 0)
		return -1;

	if (!p.name[0] || show_stats) {
		struct tnl_print_nlmsg_info info = {
			.p1    = &p,
			.p2    = &p1,
			.init  = ip6_tnl_parm_initialize,
			.match = ip6_tnl_parm_match,
			.print = print_tunnel,
		};

		return do_tunnels_list(&info);
	}

	if (tnl_get_ioctl(p.name, &p))
		return -1;

	print_tunnel(&p);
	return 0;
}

static int do_add(int cmd, int argc, char **argv)
{
	struct ip6_tnl_parm2 p;
	const char *basedev = "ip6tnl0";

	ip6_tnl_parm_init(&p, 1);

	if (parse_args(argc, argv, cmd, &p) < 0)
		return -1;

	if (!*p.name)
		fprintf(stderr, "Tunnel interface name not specified\n");

	if (p.proto == IPPROTO_GRE)
		basedev = "ip6gre0";
	else if (p.i_flags & VTI_ISVTI)
		basedev = "ip6_vti0";

	return tnl_add_ioctl(cmd, basedev, p.name, &p);
}

static int do_del(int argc, char **argv)
{
	struct ip6_tnl_parm2 p;
	const char *basedev = "ip6tnl0";

	ip6_tnl_parm_init(&p, 1);

	if (parse_args(argc, argv, SIOCDELTUNNEL, &p) < 0)
		return -1;

	if (p.proto == IPPROTO_GRE)
		basedev = "ip6gre0";
	else if (p.i_flags & VTI_ISVTI)
		basedev = "ip6_vti0";

	return tnl_del_ioctl(basedev, p.name, &p);
}

int do_ip6tunnel(int argc, char **argv)
{
	switch (preferred_family) {
	case AF_UNSPEC:
		preferred_family = AF_INET6;
		break;
	case AF_INET6:
		break;
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
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return do_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip -f inet6 tunnel help\".\n", *argv);
	exit(-1);
}
