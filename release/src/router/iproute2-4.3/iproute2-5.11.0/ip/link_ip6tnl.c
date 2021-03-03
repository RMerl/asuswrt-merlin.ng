/*
 * link_ip6tnl.c	ip6tnl driver module
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Nicolas Dichtel <nicolas.dichtel@6wind.com>
 *
 */

#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <linux/ip6_tunnel.h>
#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "tunnel.h"

#define IP6_FLOWINFO_TCLASS	htonl(0x0FF00000)
#define IP6_FLOWINFO_FLOWLABEL	htonl(0x000FFFFF)

#define DEFAULT_TNL_HOP_LIMIT	(64)

static void ip6tunnel_print_help(struct link_util *lu, int argc, char **argv,
				 FILE *f)
{
	fprintf(f,
		"Usage: ... %-6s	[ remote ADDR ]\n"
		"			[ local ADDR ]\n"
		"			[ encaplimit ELIM ]\n"
		"			[ hoplimit HLIM ]\n"
		"			[ tclass TCLASS ]\n"
		"			[ flowlabel FLOWLABEL ]\n"
		"			[ dscp inherit ]\n"
		"			[ [no]allow-localremote ]\n"
		"			[ dev PHYS_DEV ]\n"
		"			[ fwmark MARK ]\n"
		"			[ external ]\n"
		"			[ noencap ]\n"
		"			[ encap { fou | gue | none } ]\n"
		"			[ encap-sport PORT ]\n"
		"			[ encap-dport PORT ]\n"
		"			[ [no]encap-csum ]\n"
		"			[ [no]encap-csum6 ]\n"
		"			[ [no]encap-remcsum ]\n"
		"			[ mode { ip6ip6 | ipip6 | any } ]\n"
		"\n"
		"Where:	ADDR	  := IPV6_ADDRESS\n"
		"	ELIM	  := { none | 0..255 }(default=%d)\n"
		"	HLIM	  := 0..255 (default=%d)\n"
		"	TCLASS    := { 0x0..0xff | inherit }\n"
		"	FLOWLABEL := { 0x0..0xfffff | inherit }\n"
		"	MARK	  := { 0x0..0xffffffff | inherit }\n",
		lu->id,
		IPV6_DEFAULT_TNL_ENCAP_LIMIT, DEFAULT_TNL_HOP_LIMIT);
}

static int ip6tunnel_parse_opt(struct link_util *lu, int argc, char **argv,
			       struct nlmsghdr *n)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct {
		struct nlmsghdr n;
		struct ifinfomsg i;
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(*ifi)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETLINK,
		.i.ifi_family = preferred_family,
		.i.ifi_index = ifi->ifi_index,
	};
	struct nlmsghdr *answer;
	struct rtattr *tb[IFLA_MAX + 1];
	struct rtattr *linkinfo[IFLA_INFO_MAX+1];
	struct rtattr *iptuninfo[IFLA_IPTUN_MAX + 1];
	int len;
	inet_prefix saddr, daddr;
	__u8 hop_limit = DEFAULT_TNL_HOP_LIMIT;
	__u8 encap_limit = IPV6_DEFAULT_TNL_ENCAP_LIMIT;
	__u32 flowinfo = 0;
	__u32 flags = 0;
	__u8 proto = 0;
	__u32 link = 0;
	__u16 encaptype = 0;
	__u16 encapflags = TUNNEL_ENCAP_FLAG_CSUM6;
	__u16 encapsport = 0;
	__u16 encapdport = 0;
	__u8 metadata = 0;
	__u32 fwmark = 0;

	inet_prefix_reset(&saddr);
	inet_prefix_reset(&daddr);

	if (!(n->nlmsg_flags & NLM_F_CREATE)) {
		const struct rtattr *rta;

		if (rtnl_talk(&rth, &req.n, &answer) < 0) {
get_failed:
			fprintf(stderr,
				"Failed to get existing tunnel info.\n");
			return -1;
		}

		len = answer->nlmsg_len;
		len -= NLMSG_LENGTH(sizeof(*ifi));
		if (len < 0)
			goto get_failed;

		parse_rtattr(tb, IFLA_MAX, IFLA_RTA(NLMSG_DATA(answer)), len);

		if (!tb[IFLA_LINKINFO])
			goto get_failed;

		parse_rtattr_nested(linkinfo, IFLA_INFO_MAX, tb[IFLA_LINKINFO]);

		if (!linkinfo[IFLA_INFO_DATA])
			goto get_failed;

		parse_rtattr_nested(iptuninfo, IFLA_IPTUN_MAX,
				    linkinfo[IFLA_INFO_DATA]);

		rta = iptuninfo[IFLA_IPTUN_LOCAL];
		if (rta && get_addr_rta(&saddr, rta, AF_INET6))
			goto get_failed;

		rta = iptuninfo[IFLA_IPTUN_REMOTE];
		if (rta && get_addr_rta(&daddr, rta, AF_INET6))
			goto get_failed;

		if (iptuninfo[IFLA_IPTUN_TTL])
			hop_limit = rta_getattr_u8(iptuninfo[IFLA_IPTUN_TTL]);

		if (iptuninfo[IFLA_IPTUN_ENCAP_LIMIT])
			encap_limit = rta_getattr_u8(iptuninfo[IFLA_IPTUN_ENCAP_LIMIT]);

		if (iptuninfo[IFLA_IPTUN_FLOWINFO])
			flowinfo = rta_getattr_u32(iptuninfo[IFLA_IPTUN_FLOWINFO]);

		if (iptuninfo[IFLA_IPTUN_FLAGS])
			flags = rta_getattr_u32(iptuninfo[IFLA_IPTUN_FLAGS]);

		if (iptuninfo[IFLA_IPTUN_LINK])
			link = rta_getattr_u32(iptuninfo[IFLA_IPTUN_LINK]);

		if (iptuninfo[IFLA_IPTUN_PROTO])
			proto = rta_getattr_u8(iptuninfo[IFLA_IPTUN_PROTO]);
		if (iptuninfo[IFLA_IPTUN_COLLECT_METADATA])
			metadata = 1;

		if (iptuninfo[IFLA_IPTUN_FWMARK])
			fwmark = rta_getattr_u32(iptuninfo[IFLA_IPTUN_FWMARK]);

		free(answer);
	}

	while (argc > 0) {
		if (strcmp(*argv, "mode") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ipv6/ipv6") == 0 ||
			    strcmp(*argv, "ip6ip6") == 0)
				proto = IPPROTO_IPV6;
			else if (strcmp(*argv, "ip/ipv6") == 0 ||
				 strcmp(*argv, "ipv4/ipv6") == 0 ||
				 strcmp(*argv, "ipip6") == 0 ||
				 strcmp(*argv, "ip4ip6") == 0)
				proto = IPPROTO_IPIP;
			else if (strcmp(*argv, "any/ipv6") == 0 ||
				 strcmp(*argv, "any") == 0)
				proto = 0;
			else
				invarg("Cannot guess tunnel mode.", *argv);
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			get_addr(&daddr, *argv, AF_INET6);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			get_addr(&saddr, *argv, AF_INET6);
		} else if (matches(*argv, "dev") == 0) {
			NEXT_ARG();
			link = ll_name_to_index(*argv);
			if (!link)
				exit(nodev(*argv));
		} else if (strcmp(*argv, "ttl") == 0 ||
			   strcmp(*argv, "hoplimit") == 0 ||
			   strcmp(*argv, "hlim") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (get_u8(&hop_limit, *argv, 0))
					invarg("invalid HLIM\n", *argv);
			} else
				hop_limit = 0;
		} else if (strcmp(*argv, "encaplimit") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "none") == 0) {
				flags |= IP6_TNL_F_IGN_ENCAP_LIMIT;
			} else {
				__u8 uval;

				if (get_u8(&uval, *argv, 0) < -1)
					invarg("invalid ELIM", *argv);
				encap_limit = uval;
				flags &= ~IP6_TNL_F_IGN_ENCAP_LIMIT;
			}
		} else if (strcmp(*argv, "tos") == 0 ||
			   strcmp(*argv, "tclass") == 0 ||
			   strcmp(*argv, "tc") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			__u8 uval;

			NEXT_ARG();
			flowinfo &= ~IP6_FLOWINFO_TCLASS;
			if (strcmp(*argv, "inherit") == 0)
				flags |= IP6_TNL_F_USE_ORIG_TCLASS;
			else {
				if (get_u8(&uval, *argv, 16))
					invarg("invalid TClass", *argv);
				flowinfo |= htonl((__u32)uval << 20) & IP6_FLOWINFO_TCLASS;
				flags &= ~IP6_TNL_F_USE_ORIG_TCLASS;
			}
		} else if (strcmp(*argv, "flowlabel") == 0 ||
			   strcmp(*argv, "fl") == 0) {
			__u32 uval;

			NEXT_ARG();
			flowinfo &= ~IP6_FLOWINFO_FLOWLABEL;
			if (strcmp(*argv, "inherit") == 0)
				flags |= IP6_TNL_F_USE_ORIG_FLOWLABEL;
			else {
				if (get_u32(&uval, *argv, 16))
					invarg("invalid Flowlabel", *argv);
				if (uval > 0xFFFFF)
					invarg("invalid Flowlabel", *argv);
				flowinfo |= htonl(uval) & IP6_FLOWINFO_FLOWLABEL;
				flags &= ~IP6_TNL_F_USE_ORIG_FLOWLABEL;
			}
		} else if (strcmp(*argv, "dscp") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0)
				invarg("not inherit", *argv);
			flags |= IP6_TNL_F_RCV_DSCP_COPY;
		} else if (strcmp(*argv, "fwmark") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "inherit") == 0) {
				flags |= IP6_TNL_F_USE_ORIG_FWMARK;
				fwmark = 0;
			} else {
				if (get_u32(&fwmark, *argv, 0))
					invarg("invalid fwmark\n", *argv);
				flags &= ~IP6_TNL_F_USE_ORIG_FWMARK;
			}
		} else if (strcmp(*argv, "allow-localremote") == 0) {
			flags |= IP6_TNL_F_ALLOW_LOCAL_REMOTE;
		} else if (strcmp(*argv, "noallow-localremote") == 0) {
			flags &= ~IP6_TNL_F_ALLOW_LOCAL_REMOTE;
		} else if (strcmp(*argv, "noencap") == 0) {
			encaptype = TUNNEL_ENCAP_NONE;
		} else if (strcmp(*argv, "encap") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "fou") == 0)
				encaptype = TUNNEL_ENCAP_FOU;
			else if (strcmp(*argv, "gue") == 0)
				encaptype = TUNNEL_ENCAP_GUE;
			else if (strcmp(*argv, "none") == 0)
				encaptype = TUNNEL_ENCAP_NONE;
			else
				invarg("Invalid encap type.", *argv);
		} else if (strcmp(*argv, "encap-sport") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "auto") == 0)
				encapsport = 0;
			else if (get_u16(&encapsport, *argv, 0))
				invarg("Invalid source port.", *argv);
		} else if (strcmp(*argv, "encap-dport") == 0) {
			NEXT_ARG();
			if (get_u16(&encapdport, *argv, 0))
				invarg("Invalid destination port.", *argv);
		} else if (strcmp(*argv, "encap-csum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_CSUM;
		} else if (strcmp(*argv, "noencap-csum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_CSUM;
		} else if (strcmp(*argv, "encap-udp6-csum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_CSUM6;
		} else if (strcmp(*argv, "noencap-udp6-csum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_CSUM6;
		} else if (strcmp(*argv, "encap-remcsum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_REMCSUM;
		} else if (strcmp(*argv, "noencap-remcsum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_REMCSUM;
		} else if (strcmp(*argv, "external") == 0) {
			metadata = 1;
		} else {
			ip6tunnel_print_help(lu, argc, argv, stderr);
			return -1;
		}
		argc--, argv++;
	}

	addattr8(n, 1024, IFLA_IPTUN_PROTO, proto);
	if (metadata) {
		addattr_l(n, 1024, IFLA_IPTUN_COLLECT_METADATA, NULL, 0);
		return 0;
	}

	if (is_addrtype_inet_not_unspec(&saddr)) {
		addattr_l(n, 1024, IFLA_IPTUN_LOCAL,
			  saddr.data, saddr.bytelen);
	}
	if (is_addrtype_inet_not_unspec(&daddr)) {
		addattr_l(n, 1024, IFLA_IPTUN_REMOTE,
			  daddr.data, daddr.bytelen);
	}
	addattr8(n, 1024, IFLA_IPTUN_TTL, hop_limit);
	addattr8(n, 1024, IFLA_IPTUN_ENCAP_LIMIT, encap_limit);
	addattr32(n, 1024, IFLA_IPTUN_FLOWINFO, flowinfo);
	addattr32(n, 1024, IFLA_IPTUN_FLAGS, flags);
	addattr32(n, 1024, IFLA_IPTUN_LINK, link);
	addattr32(n, 1024, IFLA_IPTUN_FWMARK, fwmark);

	addattr16(n, 1024, IFLA_IPTUN_ENCAP_TYPE, encaptype);
	addattr16(n, 1024, IFLA_IPTUN_ENCAP_FLAGS, encapflags);
	addattr16(n, 1024, IFLA_IPTUN_ENCAP_SPORT, htons(encapsport));
	addattr16(n, 1024, IFLA_IPTUN_ENCAP_DPORT, htons(encapdport));

	return 0;
}

static void ip6tunnel_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	char s2[64];
	__u32 flags = 0;
	__u32 flowinfo = 0;
	__u8 ttl = 0;

	if (!tb)
		return;

	if (tb[IFLA_IPTUN_COLLECT_METADATA]) {
		print_bool(PRINT_ANY, "external", "external ", true);
		return;
	}

	if (tb[IFLA_IPTUN_FLAGS])
		flags = rta_getattr_u32(tb[IFLA_IPTUN_FLAGS]);

	if (tb[IFLA_IPTUN_FLOWINFO])
		flowinfo = rta_getattr_u32(tb[IFLA_IPTUN_FLOWINFO]);

	if (tb[IFLA_IPTUN_PROTO]) {
		switch (rta_getattr_u8(tb[IFLA_IPTUN_PROTO])) {
		case IPPROTO_IPIP:
			print_string(PRINT_ANY, "proto", "%s ", "ipip6");
			break;
		case IPPROTO_IPV6:
			print_string(PRINT_ANY, "proto", "%s ", "ip6ip6");
			break;
		case 0:
			print_string(PRINT_ANY, "proto", "%s ", "any");
			break;
		}
	}

	tnl_print_endpoint("remote", tb[IFLA_IPTUN_REMOTE], AF_INET6);
	tnl_print_endpoint("local", tb[IFLA_IPTUN_LOCAL], AF_INET6);

	if (tb[IFLA_IPTUN_LINK]) {
		__u32 link = rta_getattr_u32(tb[IFLA_IPTUN_LINK]);

		if (link) {
			print_string(PRINT_ANY, "link", "dev %s ",
				     ll_index_to_name(link));
		}
	}

	if (tb[IFLA_IPTUN_TTL])
		ttl = rta_getattr_u8(tb[IFLA_IPTUN_TTL]);
	if (is_json_context() || ttl)
		print_uint(PRINT_ANY, "ttl", "hoplimit %u ", ttl);
	else
		print_string(PRINT_FP, NULL, "hoplimit %s ", "inherit");

	if (flags & IP6_TNL_F_IGN_ENCAP_LIMIT) {
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_ign_encap_limit",
			   "encaplimit none ",
			   true);
	} else if (tb[IFLA_IPTUN_ENCAP_LIMIT]) {
		__u8 val = rta_getattr_u8(tb[IFLA_IPTUN_ENCAP_LIMIT]);

		print_uint(PRINT_ANY, "encap_limit", "encaplimit %u ", val);
	}

	if (flags & IP6_TNL_F_USE_ORIG_TCLASS) {
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_use_orig_tclass",
			   "tclass inherit ",
			   true);
	} else if (tb[IFLA_IPTUN_FLOWINFO]) {
		__u32 val = ntohl(flowinfo & IP6_FLOWINFO_TCLASS) >> 20;

		snprintf(s2, sizeof(s2), "0x%02x", val);
		print_string(PRINT_ANY, "tclass", "tclass %s ", s2);
	}

	if (flags & IP6_TNL_F_USE_ORIG_FLOWLABEL) {
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_use_orig_flowlabel",
			   "flowlabel inherit ",
			   true);
	} else if (tb[IFLA_IPTUN_FLOWINFO]) {
		__u32 val = ntohl(flowinfo & IP6_FLOWINFO_FLOWLABEL);

		snprintf(s2, sizeof(s2), "0x%05x", val);
		print_string(PRINT_ANY, "flowlabel", "flowlabel %s ", s2);
	}

	if (flags & IP6_TNL_F_RCV_DSCP_COPY)
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_rcv_dscp_copy",
			   "dscp inherit ",
			   true);

	if (flags & IP6_TNL_F_MIP6_DEV)
		print_bool(PRINT_ANY, "ip6_tnl_f_mip6_dev", "mip6 ", true);

	if (flags & IP6_TNL_F_ALLOW_LOCAL_REMOTE)
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_allow_local_remote",
			   "allow-localremote ",
			   true);

	if (flags & IP6_TNL_F_USE_ORIG_FWMARK) {
		print_bool(PRINT_ANY,
			   "ip6_tnl_f_use_orig_fwmark",
			   "fwmark inherit ",
			   true);
	} else if (tb[IFLA_IPTUN_FWMARK]) {
		__u32 fwmark = rta_getattr_u32(tb[IFLA_IPTUN_FWMARK]);

		if (fwmark) {
			print_0xhex(PRINT_ANY,
				    "fwmark", "fwmark %#llx ", fwmark);
		}
	}

	tnl_print_encap(tb,
			IFLA_IPTUN_ENCAP_TYPE,
			IFLA_IPTUN_ENCAP_FLAGS,
			IFLA_IPTUN_ENCAP_SPORT,
			IFLA_IPTUN_ENCAP_DPORT);
}

struct link_util ip6tnl_link_util = {
	.id = "ip6tnl",
	.maxattr = IFLA_IPTUN_MAX,
	.parse_opt = ip6tunnel_parse_opt,
	.print_opt = ip6tunnel_print_opt,
	.print_help = ip6tunnel_print_help,
};
