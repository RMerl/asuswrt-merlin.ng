/*
 * iproute.c		"ip route".
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/in_route.h>
#include <linux/icmpv6.h>
#include <errno.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

#ifndef RTAX_RTTVAR
#define RTAX_RTTVAR RTAX_HOPS
#endif

enum list_action {
	IPROUTE_LIST,
	IPROUTE_FLUSH,
	IPROUTE_SAVE,
};
static const char *mx_names[RTAX_MAX+1] = {
	[RTAX_MTU]			= "mtu",
	[RTAX_WINDOW]			= "window",
	[RTAX_RTT]			= "rtt",
	[RTAX_RTTVAR]			= "rttvar",
	[RTAX_SSTHRESH]			= "ssthresh",
	[RTAX_CWND]			= "cwnd",
	[RTAX_ADVMSS]			= "advmss",
	[RTAX_REORDERING]		= "reordering",
	[RTAX_HOPLIMIT]			= "hoplimit",
	[RTAX_INITCWND]			= "initcwnd",
	[RTAX_FEATURES]			= "features",
	[RTAX_RTO_MIN]			= "rto_min",
	[RTAX_INITRWND]			= "initrwnd",
	[RTAX_QUICKACK]			= "quickack",
	[RTAX_CC_ALGO]			= "congctl",
	[RTAX_FASTOPEN_NO_COOKIE]	= "fastopen_no_cookie"
};
static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip route { list | flush } SELECTOR\n"
		"       ip route save SELECTOR\n"
		"       ip route restore\n"
		"       ip route showdump\n"
		"       ip route get [ ROUTE_GET_FLAGS ] ADDRESS\n"
		"                            [ from ADDRESS iif STRING ]\n"
		"                            [ oif STRING ] [ tos TOS ]\n"
		"                            [ mark NUMBER ] [ vrf NAME ]\n"
		"                            [ uid NUMBER ] [ ipproto PROTOCOL ]\n"
		"                            [ sport NUMBER ] [ dport NUMBER ]\n"
		"       ip route { add | del | change | append | replace } ROUTE\n"
		"SELECTOR := [ root PREFIX ] [ match PREFIX ] [ exact PREFIX ]\n"
		"            [ table TABLE_ID ] [ vrf NAME ] [ proto RTPROTO ]\n"
		"            [ type TYPE ] [ scope SCOPE ]\n"
		"ROUTE := NODE_SPEC [ INFO_SPEC ]\n"
		"NODE_SPEC := [ TYPE ] PREFIX [ tos TOS ]\n"
		"             [ table TABLE_ID ] [ proto RTPROTO ]\n"
		"             [ scope SCOPE ] [ metric METRIC ]\n"
		"             [ ttl-propagate { enabled | disabled } ]\n"
		"INFO_SPEC := { NH | nhid ID } OPTIONS FLAGS [ nexthop NH ]...\n"
		"NH := [ encap ENCAPTYPE ENCAPHDR ] [ via [ FAMILY ] ADDRESS ]\n"
		"	    [ dev STRING ] [ weight NUMBER ] NHFLAGS\n"
		"FAMILY := [ inet | inet6 | mpls | bridge | link ]\n"
		"OPTIONS := FLAGS [ mtu NUMBER ] [ advmss NUMBER ] [ as [ to ] ADDRESS ]\n"
		"           [ rtt TIME ] [ rttvar TIME ] [ reordering NUMBER ]\n"
		"           [ window NUMBER ] [ cwnd NUMBER ] [ initcwnd NUMBER ]\n"
		"           [ ssthresh NUMBER ] [ realms REALM ] [ src ADDRESS ]\n"
		"           [ rto_min TIME ] [ hoplimit NUMBER ] [ initrwnd NUMBER ]\n"
		"           [ features FEATURES ] [ quickack BOOL ] [ congctl NAME ]\n"
		"           [ pref PREF ] [ expires TIME ] [ fastopen_no_cookie BOOL ]\n"
		"TYPE := { unicast | local | broadcast | multicast | throw |\n"
		"          unreachable | prohibit | blackhole | nat }\n"
		"TABLE_ID := [ local | main | default | all | NUMBER ]\n"
		"SCOPE := [ host | link | global | NUMBER ]\n"
		"NHFLAGS := [ onlink | pervasive ]\n"
		"RTPROTO := [ kernel | boot | static | NUMBER ]\n"
		"PREF := [ low | medium | high ]\n"
		"TIME := NUMBER[s|ms]\n"
		"BOOL := [1|0]\n"
		"FEATURES := ecn\n"
		"ENCAPTYPE := [ mpls | ip | ip6 | seg6 | seg6local | rpl ]\n"
		"ENCAPHDR := [ MPLSLABEL | SEG6HDR ]\n"
		"SEG6HDR := [ mode SEGMODE ] segs ADDR1,ADDRi,ADDRn [hmac HMACKEYID] [cleanup]\n"
		"SEGMODE := [ encap | inline ]\n"
		"ROUTE_GET_FLAGS := [ fibmatch ]\n");
	exit(-1);
}


static struct
{
	unsigned int tb;
	int cloned;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int protocol, protocolmask;
	int scope, scopemask;
	__u64 typemask;
	int tos, tosmask;
	int iif, iifmask;
	int oif, oifmask;
	int mark, markmask;
	int realm, realmmask;
	__u32 metric, metricmask;
	inet_prefix rprefsrc;
	inet_prefix rvia;
	inet_prefix rdst;
	inet_prefix mdst;
	inet_prefix rsrc;
	inet_prefix msrc;
} filter;

static int flush_update(void)
{
	if (rtnl_send_check(&rth, filter.flushb, filter.flushp) < 0) {
		perror("Failed to send flush request");
		return -2;
	}
	filter.flushp = 0;
	return 0;
}

static int filter_nlmsg(struct nlmsghdr *n, struct rtattr **tb, int host_len)
{
	struct rtmsg *r = NLMSG_DATA(n);
	inet_prefix dst = { .family = r->rtm_family };
	inet_prefix src = { .family = r->rtm_family };
	inet_prefix via = { .family = r->rtm_family };
	inet_prefix prefsrc = { .family = r->rtm_family };
	__u32 table;
	static int ip6_multiple_tables;

	table = rtm_get_table(r, tb);

	if (preferred_family != AF_UNSPEC && r->rtm_family != preferred_family)
		return 0;

	if (r->rtm_family == AF_INET6 && table != RT_TABLE_MAIN)
		ip6_multiple_tables = 1;

	if (filter.cloned == !(r->rtm_flags & RTM_F_CLONED))
		return 0;

	if (r->rtm_family == AF_INET6 && !ip6_multiple_tables) {
		if (filter.tb) {
			if (filter.tb == RT_TABLE_LOCAL) {
				if (r->rtm_type != RTN_LOCAL)
					return 0;
			} else if (filter.tb == RT_TABLE_MAIN) {
				if (r->rtm_type == RTN_LOCAL)
					return 0;
			} else {
				return 0;
			}
		}
	} else {
		if (filter.tb > 0 && filter.tb != table)
			return 0;
	}
	if ((filter.protocol^r->rtm_protocol)&filter.protocolmask)
		return 0;
	if ((filter.scope^r->rtm_scope)&filter.scopemask)
		return 0;

	if (filter.typemask && !(filter.typemask & (1 << r->rtm_type)))
		return 0;
	if ((filter.tos^r->rtm_tos)&filter.tosmask)
		return 0;
	if (filter.rdst.family) {
		if (r->rtm_family != filter.rdst.family ||
		    filter.rdst.bitlen > r->rtm_dst_len)
			return 0;
	} else if (filter.rdst.flags & PREFIXLEN_SPECIFIED) {
		if (filter.rdst.bitlen > r->rtm_dst_len)
			return 0;
	}
	if (filter.mdst.family) {
		if (r->rtm_family != filter.mdst.family ||
		    (filter.mdst.bitlen >= 0 &&
		     filter.mdst.bitlen < r->rtm_dst_len))
			return 0;
	} else if (filter.mdst.flags & PREFIXLEN_SPECIFIED) {
		if (filter.mdst.bitlen >= 0 &&
		    filter.mdst.bitlen < r->rtm_dst_len)
			return 0;
	}
	if (filter.rsrc.family) {
		if (r->rtm_family != filter.rsrc.family ||
		    filter.rsrc.bitlen > r->rtm_src_len)
			return 0;
	} else if (filter.rsrc.flags & PREFIXLEN_SPECIFIED) {
		if (filter.rsrc.bitlen > r->rtm_src_len)
			return 0;
	}
	if (filter.msrc.family) {
		if (r->rtm_family != filter.msrc.family ||
		    (filter.msrc.bitlen >= 0 &&
		     filter.msrc.bitlen < r->rtm_src_len))
			return 0;
	} else if (filter.msrc.flags & PREFIXLEN_SPECIFIED) {
		if (filter.msrc.bitlen >= 0 &&
		    filter.msrc.bitlen < r->rtm_src_len)
			return 0;
	}
	if (filter.rvia.family) {
		int family = r->rtm_family;

		if (tb[RTA_VIA]) {
			struct rtvia *via = RTA_DATA(tb[RTA_VIA]);

			family = via->rtvia_family;
		}
		if (family != filter.rvia.family)
			return 0;
	}
	if (filter.rprefsrc.family && r->rtm_family != filter.rprefsrc.family)
		return 0;

	if (tb[RTA_DST])
		memcpy(&dst.data, RTA_DATA(tb[RTA_DST]), (r->rtm_dst_len+7)/8);
	if (filter.rsrc.family || filter.msrc.family ||
	    filter.rsrc.flags & PREFIXLEN_SPECIFIED ||
	    filter.msrc.flags & PREFIXLEN_SPECIFIED) {
		if (tb[RTA_SRC])
			memcpy(&src.data, RTA_DATA(tb[RTA_SRC]), (r->rtm_src_len+7)/8);
	}
	if (filter.rvia.bitlen > 0) {
		if (tb[RTA_GATEWAY])
			memcpy(&via.data, RTA_DATA(tb[RTA_GATEWAY]), host_len/8);
		if (tb[RTA_VIA]) {
			size_t len = RTA_PAYLOAD(tb[RTA_VIA]) - 2;
			struct rtvia *rtvia = RTA_DATA(tb[RTA_VIA]);

			via.family = rtvia->rtvia_family;
			memcpy(&via.data, rtvia->rtvia_addr, len);
		}
	}
	if (filter.rprefsrc.bitlen > 0) {
		if (tb[RTA_PREFSRC])
			memcpy(&prefsrc.data, RTA_DATA(tb[RTA_PREFSRC]), host_len/8);
	}

	if ((filter.rdst.family || filter.rdst.flags & PREFIXLEN_SPECIFIED) &&
	    inet_addr_match(&dst, &filter.rdst, filter.rdst.bitlen))
		return 0;
	if ((filter.mdst.family || filter.mdst.flags & PREFIXLEN_SPECIFIED) &&
	    inet_addr_match(&dst, &filter.mdst, r->rtm_dst_len))
		return 0;

	if ((filter.rsrc.family || filter.rsrc.flags & PREFIXLEN_SPECIFIED) &&
	    inet_addr_match(&src, &filter.rsrc, filter.rsrc.bitlen))
		return 0;
	if ((filter.msrc.family || filter.msrc.flags & PREFIXLEN_SPECIFIED) &&
	    filter.msrc.bitlen >= 0 &&
	    inet_addr_match(&src, &filter.msrc, r->rtm_src_len))
		return 0;

	if (filter.rvia.family && inet_addr_match(&via, &filter.rvia, filter.rvia.bitlen))
		return 0;
	if (filter.rprefsrc.family && inet_addr_match(&prefsrc, &filter.rprefsrc, filter.rprefsrc.bitlen))
		return 0;
	if (filter.realmmask) {
		__u32 realms = 0;

		if (tb[RTA_FLOW])
			realms = rta_getattr_u32(tb[RTA_FLOW]);
		if ((realms^filter.realm)&filter.realmmask)
			return 0;
	}
	if (filter.iifmask) {
		int iif = 0;

		if (tb[RTA_IIF])
			iif = rta_getattr_u32(tb[RTA_IIF]);
		if ((iif^filter.iif)&filter.iifmask)
			return 0;
	}
	if (filter.oifmask) {
		int oif = 0;

		if (tb[RTA_OIF])
			oif = rta_getattr_u32(tb[RTA_OIF]);
		if ((oif^filter.oif)&filter.oifmask)
			return 0;
	}
	if (filter.markmask) {
		int mark = 0;

		if (tb[RTA_MARK])
			mark = rta_getattr_u32(tb[RTA_MARK]);
		if ((mark ^ filter.mark) & filter.markmask)
			return 0;
	}
	if (filter.metricmask) {
		__u32 metric = 0;

		if (tb[RTA_PRIORITY])
			metric = rta_getattr_u32(tb[RTA_PRIORITY]);
		if ((metric ^ filter.metric) & filter.metricmask)
			return 0;
	}
	if (filter.flushb &&
	    r->rtm_family == AF_INET6 &&
	    r->rtm_dst_len == 0 &&
	    r->rtm_type == RTN_UNREACHABLE &&
	    tb[RTA_PRIORITY] &&
	    rta_getattr_u32(tb[RTA_PRIORITY]) == -1)
		return 0;

	return 1;
}

static void print_rtax_features(FILE *fp, unsigned int features)
{
	unsigned int of = features;

	if (features & RTAX_FEATURE_ECN) {
		print_null(PRINT_ANY, "ecn", "ecn ", NULL);
		features &= ~RTAX_FEATURE_ECN;
	}

	if (features)
		print_0xhex(PRINT_ANY,
			    "features", "%#llx ", of);
}

void print_rt_flags(FILE *fp, unsigned int flags)
{
	open_json_array(PRINT_JSON,
			is_json_context() ?  "flags" : "");

	if (flags & RTNH_F_DEAD)
		print_string(PRINT_ANY, NULL, "%s ", "dead");
	if (flags & RTNH_F_ONLINK)
		print_string(PRINT_ANY, NULL, "%s ", "onlink");
	if (flags & RTNH_F_PERVASIVE)
		print_string(PRINT_ANY, NULL, "%s ", "pervasive");
	if (flags & RTNH_F_OFFLOAD)
		print_string(PRINT_ANY, NULL, "%s ", "offload");
	if (flags & RTNH_F_TRAP)
		print_string(PRINT_ANY, NULL, "%s ", "trap");
	if (flags & RTM_F_NOTIFY)
		print_string(PRINT_ANY, NULL, "%s ", "notify");
	if (flags & RTNH_F_LINKDOWN)
		print_string(PRINT_ANY, NULL, "%s ", "linkdown");
	if (flags & RTNH_F_UNRESOLVED)
		print_string(PRINT_ANY, NULL, "%s ", "unresolved");
	if (flags & RTM_F_OFFLOAD)
		print_string(PRINT_ANY, NULL, "%s ", "rt_offload");
	if (flags & RTM_F_TRAP)
		print_string(PRINT_ANY, NULL, "%s ", "rt_trap");
	if (flags & RTM_F_OFFLOAD_FAILED)
		print_string(PRINT_ANY, NULL, "%s ", "rt_offload_failed");

	close_json_array(PRINT_JSON, NULL);
}

static void print_rt_pref(FILE *fp, unsigned int pref)
{

	switch (pref) {
	case ICMPV6_ROUTER_PREF_LOW:
		print_string(PRINT_ANY,
			     "pref", "pref %s", "low");
		break;
	case ICMPV6_ROUTER_PREF_MEDIUM:
		print_string(PRINT_ANY,
			     "pref", "pref %s", "medium");
		break;
	case ICMPV6_ROUTER_PREF_HIGH:
		print_string(PRINT_ANY,
			     "pref", "pref %s", "high");
		break;
	default:
		print_uint(PRINT_ANY,
			   "pref", "%u", pref);
	}
}

void print_rta_if(FILE *fp, const struct rtattr *rta, const char *prefix)
{
	const char *ifname = ll_index_to_name(rta_getattr_u32(rta));

	if (is_json_context())
		print_string(PRINT_JSON, prefix, NULL, ifname);
	else {
		fprintf(fp, "%s ", prefix);
		color_fprintf(fp, COLOR_IFNAME, "%s ", ifname);
	}
}

static void print_cache_flags(FILE *fp, __u32 flags)
{
	json_writer_t *jw = get_json_writer();
	flags &= ~0xFFFF;

	if (jw) {
		jsonw_name(jw, "cache");
		jsonw_start_array(jw);
	} else {
		fprintf(fp, "%s    cache ", _SL_);
		if (flags == 0)
			return;
		putc('<', fp);
	}

#define PRTFL(fl, flname)						\
	if (flags & RTCF_##fl) {					\
		flags &= ~RTCF_##fl;					\
		if (jw)							\
			jsonw_string(jw, flname);			\
		else							\
			fprintf(fp, "%s%s", flname, flags ? "," : "> "); \
	}

	PRTFL(LOCAL, "local");
	PRTFL(REJECT, "reject");
	PRTFL(MULTICAST, "mc");
	PRTFL(BROADCAST, "brd");
	PRTFL(DNAT, "dst-nat");
	PRTFL(SNAT, "src-nat");
	PRTFL(MASQ, "masq");
	PRTFL(DIRECTDST, "dst-direct");
	PRTFL(DIRECTSRC, "src-direct");
	PRTFL(REDIRECTED, "redirected");
	PRTFL(DOREDIRECT, "redirect");
	PRTFL(FAST, "fastroute");
	PRTFL(NOTIFY, "notify");
	PRTFL(TPROXY, "proxy");
#undef PRTFL

	if (flags)
		print_hex(PRINT_ANY, "flags", "%x>", flags);

	if (jw)
		jsonw_end_array(jw);
}

static void print_rta_cacheinfo(FILE *fp, const struct rta_cacheinfo *ci)
{
	static int hz;

	if (!hz)
		hz = get_user_hz();

	if (ci->rta_expires != 0)
		print_int(PRINT_ANY, "expires",
			   "expires %dsec ", ci->rta_expires/hz);
	if (ci->rta_error != 0)
		print_uint(PRINT_ANY, "error",
			   "error %u ", ci->rta_error);

	if (show_stats) {
		if (ci->rta_clntref)
			print_uint(PRINT_ANY, "users",
				   "users %u ", ci->rta_clntref);
		if (ci->rta_used != 0)
			print_uint(PRINT_ANY, "used",
				   "used %u ", ci->rta_used);
		if (ci->rta_lastuse != 0)
			print_uint(PRINT_ANY, "age",
				   "age %usec ", ci->rta_lastuse/hz);
	}
	if (ci->rta_id)
		print_0xhex(PRINT_ANY, "ipid",
			    "ipid 0x%04llx ", ci->rta_id);
	if (ci->rta_ts || ci->rta_tsage) {
		print_0xhex(PRINT_ANY, "ts",
			    "ts 0x%llx", ci->rta_ts);
		print_uint(PRINT_ANY, "tsage",
			   "tsage %usec ", ci->rta_tsage);
	}
}

static void print_rta_flow(FILE *fp, const struct rtattr *rta)
{
	__u32 to = rta_getattr_u32(rta);
	__u32 from = to >> 16;
	SPRINT_BUF(b1);

	to &= 0xFFFF;
	if (is_json_context()) {
		open_json_object("flow");

		if (from)
			print_string(PRINT_JSON, "from", NULL,
				     rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
		print_string(PRINT_JSON, "to", NULL,
			     rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
		close_json_object();
	} else {
		fprintf(fp, "realm%s ", from ? "s" : "");

		if (from)
			print_string(PRINT_FP, NULL, "%s/",
				     rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
		print_string(PRINT_FP, NULL, "%s ",
			     rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
	}
}

static void print_rta_newdst(FILE *fp, const struct rtmsg *r,
			     const struct rtattr *rta)
{
	const char *newdst = format_host_rta(r->rtm_family, rta);

	if (is_json_context())
		print_string(PRINT_JSON, "to", NULL, newdst);
	else {
		fprintf(fp, "as to ");
		print_color_string(PRINT_FP,
				   ifa_family_color(r->rtm_family),
				   NULL, "%s ", newdst);
	}
}

void print_rta_gateway(FILE *fp, unsigned char family, const struct rtattr *rta)
{
	const char *gateway = format_host_rta(family, rta);

	if (is_json_context())
		print_string(PRINT_JSON, "gateway", NULL, gateway);
	else {
		fprintf(fp, "via ");
		print_color_string(PRINT_FP,
				   ifa_family_color(family),
				   NULL, "%s ", gateway);
	}
}

static void print_rta_via(FILE *fp, const struct rtattr *rta)
{
	size_t len = RTA_PAYLOAD(rta) - 2;
	const struct rtvia *via = RTA_DATA(rta);

	if (is_json_context()) {
		open_json_object("via");
		print_string(PRINT_JSON, "family", NULL,
			     family_name(via->rtvia_family));
		print_string(PRINT_JSON, "host", NULL,
			     format_host(via->rtvia_family, len,
					 via->rtvia_addr));
		close_json_object();
	} else {
		print_string(PRINT_FP, NULL, "via %s ",
			     family_name(via->rtvia_family));
		print_color_string(PRINT_FP,
				   ifa_family_color(via->rtvia_family),
				   NULL, "%s ",
				   format_host(via->rtvia_family,
					       len, via->rtvia_addr));
	}
}

static void print_rta_metrics(FILE *fp, const struct rtattr *rta)
{
	struct rtattr *mxrta[RTAX_MAX+1];
	unsigned int mxlock = 0;
	int i;

	open_json_array(PRINT_JSON, "metrics");
	open_json_object(NULL);

	parse_rtattr(mxrta, RTAX_MAX, RTA_DATA(rta), RTA_PAYLOAD(rta));

	if (mxrta[RTAX_LOCK])
		mxlock = rta_getattr_u32(mxrta[RTAX_LOCK]);

	for (i = 2; i <= RTAX_MAX; i++) {
		__u32 val = 0U;

		if (mxrta[i] == NULL && !(mxlock & (1 << i)))
			continue;

		if (mxrta[i] != NULL && i != RTAX_CC_ALGO)
			val = rta_getattr_u32(mxrta[i]);

		if (i == RTAX_HOPLIMIT && (int)val == -1)
			continue;

		if (!is_json_context()) {
			if (i < sizeof(mx_names)/sizeof(char *) && mx_names[i])
				fprintf(fp, "%s ", mx_names[i]);
			else
				fprintf(fp, "metric %d ", i);

			if (mxlock & (1<<i))
				fprintf(fp, "lock ");
		}

		switch (i) {
		case RTAX_FEATURES:
			print_rtax_features(fp, val);
			break;
		default:
			print_uint(PRINT_ANY, mx_names[i], "%u ", val);
			break;

		case RTAX_RTT:
		case RTAX_RTTVAR:
		case RTAX_RTO_MIN:
			if (i == RTAX_RTT)
				val /= 8;
			else if (i == RTAX_RTTVAR)
				val /= 4;

			if (is_json_context())
				print_uint(PRINT_JSON, mx_names[i],
					   NULL, val);
			else {
				if (val >= 1000)
					fprintf(fp, "%gs ", val/1e3);
				else
					fprintf(fp, "%ums ", val);
			}
			break;
		case RTAX_CC_ALGO:
			print_string(PRINT_ANY, "congestion",
				     "%s ", rta_getattr_str(mxrta[i]));
			break;
		}
	}

	close_json_object();
	close_json_array(PRINT_JSON, NULL);
}

static void print_rta_multipath(FILE *fp, const struct rtmsg *r,
				struct rtattr *rta)
{
	const struct rtnexthop *nh = RTA_DATA(rta);
	int len = RTA_PAYLOAD(rta);
	int first = 1;

	open_json_array(PRINT_JSON, "nexthops");

	while (len >= sizeof(*nh)) {
		struct rtattr *tb[RTA_MAX + 1];

		if (nh->rtnh_len > len)
			break;

		open_json_object(NULL);

		if ((r->rtm_flags & RTM_F_CLONED) &&
		    r->rtm_type == RTN_MULTICAST) {
			if (first) {
				print_string(PRINT_FP, NULL, "Oifs: ", NULL);
				first = 0;
			} else {
				print_string(PRINT_FP, NULL, " ", NULL);
			}
		} else
			print_string(PRINT_FP, NULL, "%s\tnexthop ", _SL_);

		if (nh->rtnh_len > sizeof(*nh)) {
			parse_rtattr(tb, RTA_MAX, RTNH_DATA(nh),
				     nh->rtnh_len - sizeof(*nh));

			if (tb[RTA_ENCAP])
				lwt_print_encap(fp,
						tb[RTA_ENCAP_TYPE],
						tb[RTA_ENCAP]);
			if (tb[RTA_NEWDST])
				print_rta_newdst(fp, r, tb[RTA_NEWDST]);
			if (tb[RTA_GATEWAY])
				print_rta_gateway(fp, r->rtm_family,
						  tb[RTA_GATEWAY]);
			if (tb[RTA_VIA])
				print_rta_via(fp, tb[RTA_VIA]);
			if (tb[RTA_FLOW])
				print_rta_flow(fp, tb[RTA_FLOW]);
		}

		if ((r->rtm_flags & RTM_F_CLONED) &&
		    r->rtm_type == RTN_MULTICAST) {
			print_string(PRINT_ANY, "dev",
				     "%s", ll_index_to_name(nh->rtnh_ifindex));

			if (nh->rtnh_hops != 1)
				print_int(PRINT_ANY, "ttl", "(ttl>%d)", nh->rtnh_hops);

			print_string(PRINT_FP, NULL, " ", NULL);
		} else {
			print_string(PRINT_ANY, "dev",
				     "dev %s ", ll_index_to_name(nh->rtnh_ifindex));

			if (r->rtm_family != AF_MPLS)
				print_int(PRINT_ANY, "weight",
					  "weight %d ", nh->rtnh_hops + 1);
		}

		print_rt_flags(fp, nh->rtnh_flags);

		len -= NLMSG_ALIGN(nh->rtnh_len);
		nh = RTNH_NEXT(nh);

		close_json_object();
	}
	close_json_array(PRINT_JSON, NULL);
}

int print_route(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[RTA_MAX+1];
	int family, color, host_len;
	__u32 table;
	int ret;

	SPRINT_BUF(b1);

	if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
		fprintf(stderr, "Not a route: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return -1;
	}
	if (filter.flushb && n->nlmsg_type != RTM_NEWROUTE)
		return 0;
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	host_len = af_bit_len(r->rtm_family);

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	table = rtm_get_table(r, tb);

	if (!filter_nlmsg(n, tb, host_len))
		return 0;

	if (filter.flushb) {
		struct nlmsghdr *fn;

		if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
			ret = flush_update();
			if (ret < 0)
				return ret;
		}
		fn = (struct nlmsghdr *)(filter.flushb + NLMSG_ALIGN(filter.flushp));
		memcpy(fn, n, n->nlmsg_len);
		fn->nlmsg_type = RTM_DELROUTE;
		fn->nlmsg_flags = NLM_F_REQUEST;
		fn->nlmsg_seq = ++rth.seq;
		filter.flushp = (((char *)fn) + n->nlmsg_len) - filter.flushb;
		filter.flushed++;
		if (show_stats < 2)
			return 0;
	}

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELROUTE)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if ((r->rtm_type != RTN_UNICAST || show_details > 0) &&
	    (!filter.typemask || (filter.typemask & (1 << r->rtm_type))))
		print_string(PRINT_ANY, "type", "%s ",
			     rtnl_rtntype_n2a(r->rtm_type, b1, sizeof(b1)));

	color = COLOR_NONE;
	if (tb[RTA_DST]) {
		family = get_real_family(r->rtm_type, r->rtm_family);
		color = ifa_family_color(family);

		if (r->rtm_dst_len != host_len) {
			snprintf(b1, sizeof(b1),
				 "%s/%u", rt_addr_n2a_rta(family, tb[RTA_DST]),
				 r->rtm_dst_len);
		} else {
			format_host_rta_r(family, tb[RTA_DST],
					  b1, sizeof(b1));

		}
	} else if (r->rtm_dst_len) {
		snprintf(b1, sizeof(b1), "0/%d ", r->rtm_dst_len);
	} else {
		strncpy(b1, "default", sizeof(b1));
	}
	print_color_string(PRINT_ANY, color,
			   "dst", "%s ", b1);

	if (tb[RTA_SRC]) {
		family = get_real_family(r->rtm_type, r->rtm_family);
		color = ifa_family_color(family);

		if (r->rtm_src_len != host_len) {
			snprintf(b1, sizeof(b1),
				 "%s/%u",
				 rt_addr_n2a_rta(family, tb[RTA_SRC]),
				 r->rtm_src_len);
		} else {
			format_host_rta_r(family, tb[RTA_SRC],
					  b1, sizeof(b1));
		}
		print_color_string(PRINT_ANY, color,
				   "from", "from %s ", b1);
	} else if (r->rtm_src_len) {
		snprintf(b1, sizeof(b1), "0/%u", r->rtm_src_len);

		print_string(PRINT_ANY, "src", "from %s ", b1);
	}

	if (tb[RTA_NH_ID])
		print_uint(PRINT_ANY, "nhid", "nhid %u ",
			   rta_getattr_u32(tb[RTA_NH_ID]));

	if (tb[RTA_NEWDST])
		print_rta_newdst(fp, r, tb[RTA_NEWDST]);

	if (tb[RTA_ENCAP])
		lwt_print_encap(fp, tb[RTA_ENCAP_TYPE], tb[RTA_ENCAP]);

	if (r->rtm_tos && filter.tosmask != -1) {
		print_string(PRINT_ANY, "tos", "tos %s ",
			     rtnl_dsfield_n2a(r->rtm_tos, b1, sizeof(b1)));
	}

	if (tb[RTA_GATEWAY] && filter.rvia.bitlen != host_len)
		print_rta_gateway(fp, r->rtm_family, tb[RTA_GATEWAY]);

	if (tb[RTA_VIA])
		print_rta_via(fp, tb[RTA_VIA]);

	if (tb[RTA_OIF] && filter.oifmask != -1)
		print_rta_if(fp, tb[RTA_OIF], "dev");

	if (table && (table != RT_TABLE_MAIN || show_details > 0) && !filter.tb)
		print_string(PRINT_ANY,
			     "table", "table %s ",
			     rtnl_rttable_n2a(table, b1, sizeof(b1)));

	if (!(r->rtm_flags & RTM_F_CLONED)) {
		if ((r->rtm_protocol != RTPROT_BOOT || show_details > 0) &&
		    filter.protocolmask != -1)
			print_string(PRINT_ANY,
				     "protocol", "proto %s ",
				     rtnl_rtprot_n2a(r->rtm_protocol,
						     b1, sizeof(b1)));

		if ((r->rtm_scope != RT_SCOPE_UNIVERSE || show_details > 0) &&
		    filter.scopemask != -1)
			print_string(PRINT_ANY,
				     "scope", "scope %s ",
				     rtnl_rtscope_n2a(r->rtm_scope,
						      b1, sizeof(b1)));
	}

	if (tb[RTA_PREFSRC] && filter.rprefsrc.bitlen != host_len) {
		const char *psrc
			= rt_addr_n2a_rta(r->rtm_family, tb[RTA_PREFSRC]);

		/* Do not use format_host(). It is our local addr
		   and symbolic name will not be useful.
		*/
		if (is_json_context())
			print_string(PRINT_JSON, "prefsrc", NULL, psrc);
		else {
			fprintf(fp, "src ");
			print_color_string(PRINT_FP,
					   ifa_family_color(r->rtm_family),
					   NULL, "%s ", psrc);
		}

	}

	if (tb[RTA_PRIORITY] && filter.metricmask != -1)
		print_uint(PRINT_ANY, "metric", "metric %u ",
			   rta_getattr_u32(tb[RTA_PRIORITY]));

	print_rt_flags(fp, r->rtm_flags);

	if (tb[RTA_MARK]) {
		unsigned int mark = rta_getattr_u32(tb[RTA_MARK]);

		if (mark) {
			if (is_json_context())
				print_uint(PRINT_JSON, "mark", NULL, mark);
			else if (mark >= 16)
				print_0xhex(PRINT_FP, NULL,
					    "mark 0x%llx ", mark);
			else
				print_uint(PRINT_FP, NULL,
					   "mark %u ", mark);
		}
	}

	if (tb[RTA_FLOW] && filter.realmmask != ~0U)
		print_rta_flow(fp, tb[RTA_FLOW]);

	if (tb[RTA_UID])
		print_uint(PRINT_ANY, "uid", "uid %u ",
			   rta_getattr_u32(tb[RTA_UID]));

	if (r->rtm_family == AF_INET) {
		if (r->rtm_flags & RTM_F_CLONED)
			print_cache_flags(fp, r->rtm_flags);

		if (tb[RTA_CACHEINFO])
			print_rta_cacheinfo(fp, RTA_DATA(tb[RTA_CACHEINFO]));
	} else if (r->rtm_family == AF_INET6) {
		if (tb[RTA_CACHEINFO])
			print_rta_cacheinfo(fp, RTA_DATA(tb[RTA_CACHEINFO]));
	}

	if (tb[RTA_METRICS])
		print_rta_metrics(fp, tb[RTA_METRICS]);

	if (tb[RTA_IIF] && filter.iifmask != -1)
		print_rta_if(fp, tb[RTA_IIF], "iif");

	if (tb[RTA_PREF])
		print_rt_pref(fp, rta_getattr_u8(tb[RTA_PREF]));

	if (tb[RTA_TTL_PROPAGATE]) {
		bool propagate = rta_getattr_u8(tb[RTA_TTL_PROPAGATE]);

		if (is_json_context())
			print_bool(PRINT_JSON, "ttl-propogate", NULL,
				   propagate);
		else
			print_string(PRINT_FP, NULL,
				     "ttl-propogate %s",
				     propagate ? "enabled" : "disabled");
	}

	if (tb[RTA_MULTIPATH])
		print_rta_multipath(fp, r, tb[RTA_MULTIPATH]);

	/* If you are adding new route RTA_XXXX then place it above
	 * the RTA_MULTIPATH else it will appear that the last nexthop
	 * in the ECMP has new attributes
	 */

	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();
	fflush(fp);
	return 0;
}

static int parse_one_nh(struct nlmsghdr *n, struct rtmsg *r,
			struct rtattr *rta, size_t len, struct rtnexthop *rtnh,
			int *argcp, char ***argvp)
{
	int argc = *argcp;
	char **argv = *argvp;

	while (++argv, --argc > 0) {
		if (strcmp(*argv, "via") == 0) {
			inet_prefix addr;
			int family;

			NEXT_ARG();
			family = read_family(*argv);
			if (family == AF_UNSPEC)
				family = r->rtm_family;
			else
				NEXT_ARG();
			get_addr(&addr, *argv, family);
			if (r->rtm_family == AF_UNSPEC)
				r->rtm_family = addr.family;
			if (addr.family == r->rtm_family) {
				if (rta_addattr_l(rta, len, RTA_GATEWAY,
						  &addr.data, addr.bytelen))
					return -1;
				rtnh->rtnh_len += sizeof(struct rtattr)
						  + addr.bytelen;
			} else {
				if (rta_addattr_l(rta, len, RTA_VIA,
						  &addr.family, addr.bytelen + 2))
					return -1;
				rtnh->rtnh_len += RTA_SPACE(addr.bytelen + 2);
			}
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			rtnh->rtnh_ifindex = ll_name_to_index(*argv);
			if (!rtnh->rtnh_ifindex)
				return nodev(*argv);
		} else if (strcmp(*argv, "weight") == 0) {
			unsigned int w;

			NEXT_ARG();
			if (get_unsigned(&w, *argv, 0) || w == 0 || w > 256)
				invarg("\"weight\" is invalid\n", *argv);
			rtnh->rtnh_hops = w - 1;
		} else if (strcmp(*argv, "onlink") == 0) {
			rtnh->rtnh_flags |= RTNH_F_ONLINK;
		} else if (matches(*argv, "realms") == 0) {
			__u32 realm;

			NEXT_ARG();
			if (get_rt_realms_or_raw(&realm, *argv))
				invarg("\"realm\" value is invalid\n", *argv);
			if (rta_addattr32(rta, len, RTA_FLOW, realm))
				return -1;
			rtnh->rtnh_len += sizeof(struct rtattr) + 4;
		} else if (strcmp(*argv, "encap") == 0) {
			int old_len = rta->rta_len;

			if (lwt_parse_encap(rta, len, &argc, &argv,
					    RTA_ENCAP, RTA_ENCAP_TYPE))
				return -1;
			rtnh->rtnh_len += rta->rta_len - old_len;
		} else if (strcmp(*argv, "as") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (strcmp(*argv, "to") == 0)
				NEXT_ARG();
			get_addr(&addr, *argv, r->rtm_family);
			if (rta_addattr_l(rta, len, RTA_NEWDST,
					  &addr.data, addr.bytelen))
				return -1;
			rtnh->rtnh_len += sizeof(struct rtattr) + addr.bytelen;
		} else
			break;
	}
	*argcp = argc;
	*argvp = argv;
	return 0;
}

static int parse_nexthops(struct nlmsghdr *n, struct rtmsg *r,
			  int argc, char **argv)
{
	char buf[4096];
	struct rtattr *rta = (void *)buf;
	struct rtnexthop *rtnh;

	rta->rta_type = RTA_MULTIPATH;
	rta->rta_len = RTA_LENGTH(0);
	rtnh = RTA_DATA(rta);

	while (argc > 0) {
		if (strcmp(*argv, "nexthop") != 0) {
			fprintf(stderr, "Error: \"nexthop\" or end of line is expected instead of \"%s\"\n", *argv);
			exit(-1);
		}
		if (argc <= 1) {
			fprintf(stderr, "Error: unexpected end of line after \"nexthop\"\n");
			exit(-1);
		}
		memset(rtnh, 0, sizeof(*rtnh));
		rtnh->rtnh_len = sizeof(*rtnh);
		rta->rta_len += rtnh->rtnh_len;
		if (parse_one_nh(n, r, rta, 4096, rtnh, &argc, &argv)) {
			fprintf(stderr, "Error: cannot parse nexthop\n");
			exit(-1);
		}
		rtnh = RTNH_NEXT(rtnh);
	}

	if (rta->rta_len > RTA_LENGTH(0))
		return addattr_l(n, 4096, RTA_MULTIPATH,
				 RTA_DATA(rta), RTA_PAYLOAD(rta));
	return 0;
}

static int iproute_modify(int cmd, unsigned int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct rtmsg		r;
		char			buf[4096];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.r.rtm_family = preferred_family,
		.r.rtm_table = RT_TABLE_MAIN,
		.r.rtm_scope = RT_SCOPE_NOWHERE,
	};
	char  mxbuf[256];
	struct rtattr *mxrta = (void *)mxbuf;
	unsigned int mxlock = 0;
	char  *d = NULL;
	int gw_ok = 0;
	int dst_ok = 0;
	int nhs_ok = 0;
	int scope_ok = 0;
	int table_ok = 0;
	int raw = 0;
	int type_ok = 0;
	__u32 nhid = 0;

	if (cmd != RTM_DELROUTE) {
		req.r.rtm_protocol = RTPROT_BOOT;
		req.r.rtm_scope = RT_SCOPE_UNIVERSE;
		req.r.rtm_type = RTN_UNICAST;
	}

	mxrta->rta_type = RTA_METRICS;
	mxrta->rta_len = RTA_LENGTH(0);

	while (argc > 0) {
		if (strcmp(*argv, "src") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			get_addr(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			addattr_l(&req.n, sizeof(req),
				  RTA_PREFSRC, &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "as") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			get_addr(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			addattr_l(&req.n, sizeof(req),
				  RTA_NEWDST, &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "via") == 0) {
			inet_prefix addr;
			int family;

			if (gw_ok) {
				invarg("use nexthop syntax to specify multiple via\n",
				       *argv);
			}
			gw_ok = 1;
			NEXT_ARG();
			family = read_family(*argv);
			if (family == AF_UNSPEC)
				family = req.r.rtm_family;
			else
				NEXT_ARG();
			get_addr(&addr, *argv, family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			if (addr.family == req.r.rtm_family)
				addattr_l(&req.n, sizeof(req), RTA_GATEWAY,
					  &addr.data, addr.bytelen);
			else
				addattr_l(&req.n, sizeof(req), RTA_VIA,
					  &addr.family, addr.bytelen+2);
		} else if (strcmp(*argv, "from") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			get_prefix(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			if (addr.bytelen)
				addattr_l(&req.n, sizeof(req), RTA_SRC, &addr.data, addr.bytelen);
			req.r.rtm_src_len = addr.bitlen;
		} else if (strcmp(*argv, "tos") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("\"tos\" value is invalid\n", *argv);
			req.r.rtm_tos = tos;
		} else if (strcmp(*argv, "expires") == 0) {
			__u32 expires;

			NEXT_ARG();
			if (get_u32(&expires, *argv, 0))
				invarg("\"expires\" value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), RTA_EXPIRES, expires);
		} else if (matches(*argv, "metric") == 0 ||
			   matches(*argv, "priority") == 0 ||
			   strcmp(*argv, "preference") == 0) {
			__u32 metric;

			NEXT_ARG();
			if (get_u32(&metric, *argv, 0))
				invarg("\"metric\" value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), RTA_PRIORITY, metric);
		} else if (strcmp(*argv, "scope") == 0) {
			__u32 scope = 0;

			NEXT_ARG();
			if (rtnl_rtscope_a2n(&scope, *argv))
				invarg("invalid \"scope\" value\n", *argv);
			req.r.rtm_scope = scope;
			scope_ok = 1;
		} else if (strcmp(*argv, "mtu") == 0) {
			unsigned int mtu;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_MTU);
				NEXT_ARG();
			}
			if (get_unsigned(&mtu, *argv, 0))
				invarg("\"mtu\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_MTU, mtu);
		} else if (strcmp(*argv, "hoplimit") == 0) {
			unsigned int hoplimit;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_HOPLIMIT);
				NEXT_ARG();
			}
			if (get_unsigned(&hoplimit, *argv, 0) || hoplimit > 255)
				invarg("\"hoplimit\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_HOPLIMIT, hoplimit);
		} else if (strcmp(*argv, "advmss") == 0) {
			unsigned int mss;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_ADVMSS);
				NEXT_ARG();
			}
			if (get_unsigned(&mss, *argv, 0))
				invarg("\"mss\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_ADVMSS, mss);
		} else if (matches(*argv, "reordering") == 0) {
			unsigned int reord;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_REORDERING);
				NEXT_ARG();
			}
			if (get_unsigned(&reord, *argv, 0))
				invarg("\"reordering\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_REORDERING, reord);
		} else if (strcmp(*argv, "rtt") == 0) {
			unsigned int rtt;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_RTT);
				NEXT_ARG();
			}
			if (get_time_rtt(&rtt, *argv, &raw))
				invarg("\"rtt\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_RTT,
				(raw) ? rtt : rtt * 8);
		} else if (strcmp(*argv, "rto_min") == 0) {
			unsigned int rto_min;

			NEXT_ARG();
			mxlock |= (1<<RTAX_RTO_MIN);
			if (get_time_rtt(&rto_min, *argv, &raw))
				invarg("\"rto_min\" value is invalid\n",
				       *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_RTO_MIN,
				      rto_min);
		} else if (matches(*argv, "window") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_WINDOW);
				NEXT_ARG();
			}
			if (get_unsigned(&win, *argv, 0))
				invarg("\"window\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_WINDOW, win);
		} else if (matches(*argv, "cwnd") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_CWND);
				NEXT_ARG();
			}
			if (get_unsigned(&win, *argv, 0))
				invarg("\"cwnd\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_CWND, win);
		} else if (matches(*argv, "initcwnd") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_INITCWND);
				NEXT_ARG();
			}
			if (get_unsigned(&win, *argv, 0))
				invarg("\"initcwnd\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf),
				      RTAX_INITCWND, win);
		} else if (matches(*argv, "initrwnd") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_INITRWND);
				NEXT_ARG();
			}
			if (get_unsigned(&win, *argv, 0))
				invarg("\"initrwnd\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf),
				      RTAX_INITRWND, win);
		} else if (matches(*argv, "features") == 0) {
			unsigned int features = 0;

			while (argc > 0) {
				NEXT_ARG();

				if (strcmp(*argv, "ecn") == 0)
					features |= RTAX_FEATURE_ECN;
				else
					invarg("\"features\" value not valid\n", *argv);
				break;
			}

			rta_addattr32(mxrta, sizeof(mxbuf),
				      RTAX_FEATURES, features);
		} else if (matches(*argv, "quickack") == 0) {
			unsigned int quickack;

			NEXT_ARG();
			if (get_unsigned(&quickack, *argv, 0))
				invarg("\"quickack\" value is invalid\n", *argv);
			if (quickack != 1 && quickack != 0)
				invarg("\"quickack\" value should be 0 or 1\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf),
				      RTAX_QUICKACK, quickack);
		} else if (matches(*argv, "congctl") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= 1 << RTAX_CC_ALGO;
				NEXT_ARG();
			}
			rta_addattr_l(mxrta, sizeof(mxbuf), RTAX_CC_ALGO, *argv,
				      strlen(*argv));
		} else if (matches(*argv, "rttvar") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_RTTVAR);
				NEXT_ARG();
			}
			if (get_time_rtt(&win, *argv, &raw))
				invarg("\"rttvar\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_RTTVAR,
				(raw) ? win : win * 4);
		} else if (matches(*argv, "ssthresh") == 0) {
			unsigned int win;

			NEXT_ARG();
			if (strcmp(*argv, "lock") == 0) {
				mxlock |= (1<<RTAX_SSTHRESH);
				NEXT_ARG();
			}
			if (get_unsigned(&win, *argv, 0))
				invarg("\"ssthresh\" value is invalid\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_SSTHRESH, win);
		} else if (matches(*argv, "realms") == 0) {
			__u32 realm;

			NEXT_ARG();
			if (get_rt_realms_or_raw(&realm, *argv))
				invarg("\"realm\" value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), RTA_FLOW, realm);
		} else if (strcmp(*argv, "onlink") == 0) {
			req.r.rtm_flags |= RTNH_F_ONLINK;
		} else if (strcmp(*argv, "nexthop") == 0) {
			nhs_ok = 1;
			break;
		} else if (!strcmp(*argv, "nhid")) {
			NEXT_ARG();
			if (get_u32(&nhid, *argv, 0))
				invarg("\"id\" value is invalid\n", *argv);
			addattr32(&req.n, sizeof(req), RTA_NH_ID, nhid);
		} else if (matches(*argv, "protocol") == 0) {
			__u32 prot;

			NEXT_ARG();
			if (rtnl_rtprot_a2n(&prot, *argv))
				invarg("\"protocol\" value is invalid\n", *argv);
			req.r.rtm_protocol = prot;
		} else if (matches(*argv, "table") == 0) {
			__u32 tid;

			NEXT_ARG();
			if (rtnl_rttable_a2n(&tid, *argv))
				invarg("\"table\" value is invalid\n", *argv);
			if (tid < 256)
				req.r.rtm_table = tid;
			else {
				req.r.rtm_table = RT_TABLE_UNSPEC;
				addattr32(&req.n, sizeof(req), RTA_TABLE, tid);
			}
			table_ok = 1;
		} else if (matches(*argv, "vrf") == 0) {
			__u32 tid;

			NEXT_ARG();
			tid = ipvrf_get_table(*argv);
			if (tid == 0)
				invarg("Invalid VRF\n", *argv);
			if (tid < 256)
				req.r.rtm_table = tid;
			else {
				req.r.rtm_table = RT_TABLE_UNSPEC;
				addattr32(&req.n, sizeof(req), RTA_TABLE, tid);
			}
			table_ok = 1;
		} else if (strcmp(*argv, "dev") == 0 ||
			   strcmp(*argv, "oif") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (matches(*argv, "pref") == 0) {
			__u8 pref;

			NEXT_ARG();
			if (strcmp(*argv, "low") == 0)
				pref = ICMPV6_ROUTER_PREF_LOW;
			else if (strcmp(*argv, "medium") == 0)
				pref = ICMPV6_ROUTER_PREF_MEDIUM;
			else if (strcmp(*argv, "high") == 0)
				pref = ICMPV6_ROUTER_PREF_HIGH;
			else if (get_u8(&pref, *argv, 0))
				invarg("\"pref\" value is invalid\n", *argv);
			addattr8(&req.n, sizeof(req), RTA_PREF, pref);
		} else if (strcmp(*argv, "encap") == 0) {
			char buf[1024];
			struct rtattr *rta = (void *)buf;

			rta->rta_type = RTA_ENCAP;
			rta->rta_len = RTA_LENGTH(0);

			lwt_parse_encap(rta, sizeof(buf), &argc, &argv,
					RTA_ENCAP, RTA_ENCAP_TYPE);

			if (rta->rta_len > RTA_LENGTH(0))
				addraw_l(&req.n, 1024
					 , RTA_DATA(rta), RTA_PAYLOAD(rta));
		} else if (strcmp(*argv, "ttl-propagate") == 0) {
			__u8 ttl_prop;

			NEXT_ARG();
			if (matches(*argv, "enabled") == 0)
				ttl_prop = 1;
			else if (matches(*argv, "disabled") == 0)
				ttl_prop = 0;
			else
				invarg("\"ttl-propagate\" value is invalid\n",
				       *argv);

			addattr8(&req.n, sizeof(req), RTA_TTL_PROPAGATE,
				 ttl_prop);
		} else if (matches(*argv, "fastopen_no_cookie") == 0) {
			unsigned int fastopen_no_cookie;

			NEXT_ARG();
			if (get_unsigned(&fastopen_no_cookie, *argv, 0))
				invarg("\"fastopen_no_cookie\" value is invalid\n", *argv);
			if (fastopen_no_cookie != 1 && fastopen_no_cookie != 0)
				invarg("\"fastopen_no_cookie\" value should be 0 or 1\n", *argv);
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_FASTOPEN_NO_COOKIE, fastopen_no_cookie);
		} else {
			int type;
			inet_prefix dst;

			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if ((**argv < '0' || **argv > '9') &&
			    rtnl_rtntype_a2n(&type, *argv) == 0) {
				NEXT_ARG();
				req.r.rtm_type = type;
				type_ok = 1;
			}

			if (matches(*argv, "help") == 0)
				usage();
			if (dst_ok)
				duparg2("to", *argv);
			get_prefix(&dst, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = dst.family;
			req.r.rtm_dst_len = dst.bitlen;
			dst_ok = 1;
			if (dst.bytelen)
				addattr_l(&req.n, sizeof(req),
					  RTA_DST, &dst.data, dst.bytelen);
		}
		argc--; argv++;
	}

	if (!dst_ok)
		usage();

	if (d) {
		int idx = ll_name_to_index(d);

		if (!idx)
			return nodev(d);
		addattr32(&req.n, sizeof(req), RTA_OIF, idx);
	}

	if (mxrta->rta_len > RTA_LENGTH(0)) {
		if (mxlock)
			rta_addattr32(mxrta, sizeof(mxbuf), RTAX_LOCK, mxlock);
		addattr_l(&req.n, sizeof(req), RTA_METRICS, RTA_DATA(mxrta), RTA_PAYLOAD(mxrta));
	}

	if (nhs_ok && parse_nexthops(&req.n, &req.r, argc, argv))
		return -1;

	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = AF_INET;

	if (!table_ok) {
		if (req.r.rtm_type == RTN_LOCAL ||
		    req.r.rtm_type == RTN_BROADCAST ||
		    req.r.rtm_type == RTN_NAT ||
		    req.r.rtm_type == RTN_ANYCAST)
			req.r.rtm_table = RT_TABLE_LOCAL;
	}
	if (!scope_ok) {
		if (req.r.rtm_family == AF_INET6 ||
		    req.r.rtm_family == AF_MPLS)
			req.r.rtm_scope = RT_SCOPE_UNIVERSE;
		else if (req.r.rtm_type == RTN_LOCAL ||
			 req.r.rtm_type == RTN_NAT)
			req.r.rtm_scope = RT_SCOPE_HOST;
		else if (req.r.rtm_type == RTN_BROADCAST ||
			 req.r.rtm_type == RTN_MULTICAST ||
			 req.r.rtm_type == RTN_ANYCAST)
			req.r.rtm_scope = RT_SCOPE_LINK;
		else if (req.r.rtm_type == RTN_UNICAST ||
			 req.r.rtm_type == RTN_UNSPEC) {
			if (cmd == RTM_DELROUTE)
				req.r.rtm_scope = RT_SCOPE_NOWHERE;
			else if (!gw_ok && !nhs_ok && !nhid)
				req.r.rtm_scope = RT_SCOPE_LINK;
		}
	}

	if (!type_ok && req.r.rtm_family == AF_MPLS)
		req.r.rtm_type = RTN_UNICAST;

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int iproute_flush_cache(void)
{
#define ROUTE_FLUSH_PATH "/proc/sys/net/ipv4/route/flush"

	int len;
	int flush_fd = open(ROUTE_FLUSH_PATH, O_WRONLY);
	char *buffer = "-1";

	if (flush_fd < 0) {
		fprintf(stderr, "Cannot open \"%s\": %s\n",
				ROUTE_FLUSH_PATH, strerror(errno));
		return -1;
	}

	len = strlen(buffer);

	if ((write(flush_fd, (void *)buffer, len)) < len) {
		fprintf(stderr, "Cannot flush routing cache\n");
		close(flush_fd);
		return -1;
	}
	close(flush_fd);
	return 0;
}

static __u32 route_dump_magic = 0x45311224;

static int save_route(struct nlmsghdr *n, void *arg)
{
	int ret;
	int len = n->nlmsg_len;
	struct rtmsg *r = NLMSG_DATA(n);
	struct rtattr *tb[RTA_MAX+1];
	int host_len;

	host_len = af_bit_len(r->rtm_family);
	len -= NLMSG_LENGTH(sizeof(*r));
	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

	if (!filter_nlmsg(n, tb, host_len))
		return 0;

	ret = write(STDOUT_FILENO, n, n->nlmsg_len);
	if ((ret > 0) && (ret != n->nlmsg_len)) {
		fprintf(stderr, "Short write while saving nlmsg\n");
		ret = -EIO;
	}

	return ret == n->nlmsg_len ? 0 : ret;
}

static int save_route_prep(void)
{
	int ret;

	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "Not sending a binary stream to stdout\n");
		return -1;
	}

	ret = write(STDOUT_FILENO, &route_dump_magic, sizeof(route_dump_magic));
	if (ret != sizeof(route_dump_magic)) {
		fprintf(stderr, "Can't write magic to dump file\n");
		return -1;
	}

	return 0;
}

static int iproute_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	struct rtmsg *rtm = NLMSG_DATA(nlh);
	int err;

	rtm->rtm_protocol = filter.protocol;
	if (filter.cloned)
		rtm->rtm_flags |= RTM_F_CLONED;

	if (filter.tb) {
		err = addattr32(nlh, reqlen, RTA_TABLE, filter.tb);
		if (err)
			return err;
	}

	if (filter.oif) {
		err = addattr32(nlh, reqlen, RTA_OIF, filter.oif);
		if (err)
			return err;
	}

	return 0;
}

static int iproute_flush(int family, rtnl_filter_t filter_fn)
{
	time_t start = time(0);
	char flushb[4096-512];
	int round = 0;
	int ret;

	if (filter.cloned) {
		if (family != AF_INET6) {
			iproute_flush_cache();
			if (show_stats)
				printf("*** IPv4 routing cache is flushed.\n");
		}
		if (family == AF_INET)
			return 0;
	}

	filter.flushb = flushb;
	filter.flushp = 0;
	filter.flushe = sizeof(flushb);

	for (;;) {
		if (rtnl_routedump_req(&rth, family, iproute_dump_filter) < 0) {
			perror("Cannot send dump request");
			return -2;
		}
		filter.flushed = 0;
		if (rtnl_dump_filter(&rth, filter_fn, stdout) < 0) {
			fprintf(stderr, "Flush terminated\n");
			return -2;
		}
		if (filter.flushed == 0) {
			if (show_stats) {
				if (round == 0 &&
				    (!filter.cloned || family == AF_INET6))
					printf("Nothing to flush.\n");
				else
					printf("*** Flush is complete after %d round%s ***\n",
					       round, round > 1 ? "s" : "");
			}
			fflush(stdout);
			return 0;
		}
		round++;
		ret = flush_update();
		if (ret < 0)
			return ret;

		if (time(0) - start > 30) {
			printf("\n*** Flush not completed after %ld seconds, %d entries remain ***\n",
			       (long)(time(0) - start), filter.flushed);
			return -1;
		}

		if (show_stats) {
			printf("\n*** Round %d, deleting %d entries ***\n",
			       round, filter.flushed);
			fflush(stdout);
		}
	}
}

static int iproute_list_flush_or_save(int argc, char **argv, int action)
{
	int dump_family = preferred_family;
	char *id = NULL;
	char *od = NULL;
	unsigned int mark = 0;
	rtnl_filter_t filter_fn;

	if (action == IPROUTE_SAVE) {
		if (save_route_prep())
			return -1;

		filter_fn = save_route;
	} else
		filter_fn = print_route;

	iproute_reset_filter(0);
	filter.tb = RT_TABLE_MAIN;

	if ((action == IPROUTE_FLUSH) && argc <= 0) {
		fprintf(stderr, "\"ip route flush\" requires arguments.\n");
		return -1;
	}

	while (argc > 0) {
		if (matches(*argv, "table") == 0) {
			__u32 tid;

			NEXT_ARG();
			if (rtnl_rttable_a2n(&tid, *argv)) {
				if (strcmp(*argv, "all") == 0) {
					filter.tb = 0;
				} else if (strcmp(*argv, "cache") == 0) {
					filter.cloned = 1;
				} else if (strcmp(*argv, "help") == 0) {
					usage();
				} else {
					invarg("table id value is invalid\n", *argv);
				}
			} else
				filter.tb = tid;
		} else if (matches(*argv, "vrf") == 0) {
			__u32 tid;

			NEXT_ARG();
			tid = ipvrf_get_table(*argv);
			if (tid == 0)
				invarg("Invalid VRF\n", *argv);
			filter.tb = tid;
			filter.typemask = ~(1 << RTN_LOCAL | 1<<RTN_BROADCAST);
		} else if (matches(*argv, "cached") == 0 ||
			   matches(*argv, "cloned") == 0) {
			filter.cloned = 1;
		} else if (strcmp(*argv, "tos") == 0 ||
			   matches(*argv, "dsfield") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("TOS value is invalid\n", *argv);
			filter.tos = tos;
			filter.tosmask = -1;
		} else if (matches(*argv, "protocol") == 0) {
			__u32 prot = 0;

			NEXT_ARG();
			filter.protocolmask = -1;
			if (rtnl_rtprot_a2n(&prot, *argv)) {
				if (strcmp(*argv, "all") != 0)
					invarg("invalid \"protocol\"\n", *argv);
				prot = 0;
				filter.protocolmask = 0;
			}
			filter.protocol = prot;
		} else if (matches(*argv, "scope") == 0) {
			__u32 scope = 0;

			NEXT_ARG();
			filter.scopemask = -1;
			if (rtnl_rtscope_a2n(&scope, *argv)) {
				if (strcmp(*argv, "all") != 0)
					invarg("invalid \"scope\"\n", *argv);
				scope = RT_SCOPE_NOWHERE;
				filter.scopemask = 0;
			}
			filter.scope = scope;
		} else if (matches(*argv, "type") == 0) {
			int type;

			NEXT_ARG();
			if (rtnl_rtntype_a2n(&type, *argv))
				invarg("node type value is invalid\n", *argv);
			filter.typemask = (1<<type);
		} else if (strcmp(*argv, "dev") == 0 ||
			   strcmp(*argv, "oif") == 0) {
			NEXT_ARG();
			od = *argv;
		} else if (strcmp(*argv, "iif") == 0) {
			NEXT_ARG();
			id = *argv;
		} else if (strcmp(*argv, "mark") == 0) {
			NEXT_ARG();
			if (get_unsigned(&mark, *argv, 0))
				invarg("invalid mark value", *argv);
			filter.markmask = -1;
		} else if (matches(*argv, "metric") == 0 ||
			   matches(*argv, "priority") == 0 ||
			   strcmp(*argv, "preference") == 0) {
			__u32 metric;

			NEXT_ARG();
			if (get_u32(&metric, *argv, 0))
				invarg("\"metric\" value is invalid\n", *argv);
			filter.metric = metric;
			filter.metricmask = -1;
		} else if (strcmp(*argv, "via") == 0) {
			int family;

			NEXT_ARG();
			family = read_family(*argv);
			if (family == AF_UNSPEC)
				family = dump_family;
			else
				NEXT_ARG();
			get_prefix(&filter.rvia, *argv, family);
		} else if (strcmp(*argv, "src") == 0) {
			NEXT_ARG();
			get_prefix(&filter.rprefsrc, *argv, dump_family);
		} else if (matches(*argv, "realms") == 0) {
			__u32 realm;

			NEXT_ARG();
			if (get_rt_realms_or_raw(&realm, *argv))
				invarg("invalid realms\n", *argv);
			filter.realm = realm;
			filter.realmmask = ~0U;
			if ((filter.realm&0xFFFF) == 0 &&
			    (*argv)[strlen(*argv) - 1] == '/')
				filter.realmmask &= ~0xFFFF;
			if ((filter.realm&0xFFFF0000U) == 0 &&
			    (strchr(*argv, '/') == NULL ||
			     (*argv)[0] == '/'))
				filter.realmmask &= ~0xFFFF0000U;
		} else if (matches(*argv, "from") == 0) {
			NEXT_ARG();
			if (matches(*argv, "root") == 0) {
				NEXT_ARG();
				get_prefix(&filter.rsrc, *argv, dump_family);
			} else if (matches(*argv, "match") == 0) {
				NEXT_ARG();
				get_prefix(&filter.msrc, *argv, dump_family);
			} else {
				if (matches(*argv, "exact") == 0) {
					NEXT_ARG();
				}
				get_prefix(&filter.msrc, *argv, dump_family);
				filter.rsrc = filter.msrc;
			}
		} else {
			if (matches(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "root") == 0) {
				NEXT_ARG();
				get_prefix(&filter.rdst, *argv, dump_family);
			} else if (matches(*argv, "match") == 0) {
				NEXT_ARG();
				get_prefix(&filter.mdst, *argv, dump_family);
			} else {
				if (matches(*argv, "exact") == 0) {
					NEXT_ARG();
				}
				get_prefix(&filter.mdst, *argv, dump_family);
				filter.rdst = filter.mdst;
			}
		}
		argc--; argv++;
	}

	if (dump_family == AF_UNSPEC && filter.tb)
		dump_family = AF_INET;

	if (id || od)  {
		int idx;

		if (id) {
			idx = ll_name_to_index(id);
			if (!idx)
				return nodev(id);
			filter.iif = idx;
			filter.iifmask = -1;
		}
		if (od) {
			idx = ll_name_to_index(od);
			if (!idx)
				return nodev(od);
			filter.oif = idx;
			filter.oifmask = -1;
		}
	}
	filter.mark = mark;

	if (action == IPROUTE_FLUSH)
		return iproute_flush(dump_family, filter_fn);

	if (rtnl_routedump_req(&rth, dump_family, iproute_dump_filter) < 0) {
		perror("Cannot send dump request");
		return -2;
	}

	new_json_obj(json);

	if (rtnl_dump_filter(&rth, filter_fn, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return -2;
	}

	delete_json_obj();
	fflush(stdout);
	return 0;
}


static int iproute_get(int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct rtmsg		r;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETROUTE,
		.r.rtm_family = preferred_family,
	};
	char  *idev = NULL;
	char  *odev = NULL;
	struct nlmsghdr *answer;
	int connected = 0;
	int fib_match = 0;
	int from_ok = 0;
	unsigned int mark = 0;
	bool address_found = false;

	iproute_reset_filter(0);
	filter.cloned = 2;

	while (argc > 0) {
		if (strcmp(*argv, "tos") == 0 ||
		    matches(*argv, "dsfield") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("TOS value is invalid\n", *argv);
			req.r.rtm_tos = tos;
		} else if (matches(*argv, "from") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (matches(*argv, "help") == 0)
				usage();
			from_ok = 1;
			get_prefix(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			if (addr.bytelen)
				addattr_l(&req.n, sizeof(req), RTA_SRC,
					  &addr.data, addr.bytelen);
			req.r.rtm_src_len = addr.bitlen;
		} else if (matches(*argv, "iif") == 0) {
			NEXT_ARG();
			idev = *argv;
		} else if (matches(*argv, "mark") == 0) {
			NEXT_ARG();
			if (get_unsigned(&mark, *argv, 0))
				invarg("invalid mark value", *argv);
		} else if (matches(*argv, "oif") == 0 ||
			   strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			odev = *argv;
		} else if (matches(*argv, "notify") == 0) {
			req.r.rtm_flags |= RTM_F_NOTIFY;
		} else if (matches(*argv, "connected") == 0) {
			connected = 1;
		} else if (matches(*argv, "vrf") == 0) {
			NEXT_ARG();
			if (!name_is_vrf(*argv))
				invarg("Invalid VRF\n", *argv);
			odev = *argv;
		} else if (matches(*argv, "uid") == 0) {
			uid_t uid;

			NEXT_ARG();
			if (get_unsigned(&uid, *argv, 0))
				invarg("invalid UID\n", *argv);
			addattr32(&req.n, sizeof(req), RTA_UID, uid);
		} else if (matches(*argv, "fibmatch") == 0) {
			fib_match = 1;
		} else if (strcmp(*argv, "as") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (strcmp(*argv, "to") == 0)
				NEXT_ARG();
			get_addr(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			addattr_l(&req.n, sizeof(req), RTA_NEWDST,
				  &addr.data, addr.bytelen);
		} else if (matches(*argv, "sport") == 0) {
			__be16 sport;

			NEXT_ARG();
			if (get_be16(&sport, *argv, 0))
				invarg("invalid sport\n", *argv);
			addattr16(&req.n, sizeof(req), RTA_SPORT, sport);
		} else if (matches(*argv, "dport") == 0) {
			__be16 dport;

			NEXT_ARG();
			if (get_be16(&dport, *argv, 0))
				invarg("invalid dport\n", *argv);
			addattr16(&req.n, sizeof(req), RTA_DPORT, dport);
		} else if (matches(*argv, "ipproto") == 0) {
			int ipproto;

			NEXT_ARG();
			ipproto = inet_proto_a2n(*argv);
			if (ipproto < 0)
				invarg("Invalid \"ipproto\" value\n",
				       *argv);
			addattr8(&req.n, sizeof(req), RTA_IP_PROTO, ipproto);
		} else {
			inet_prefix addr;

			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0)
				usage();
			get_prefix(&addr, *argv, req.r.rtm_family);
			if (req.r.rtm_family == AF_UNSPEC)
				req.r.rtm_family = addr.family;
			if (addr.bytelen)
				addattr_l(&req.n, sizeof(req),
					  RTA_DST, &addr.data, addr.bytelen);
			if (req.r.rtm_family == AF_INET && addr.bitlen != 32) {
				fprintf(stderr,
					"Warning: /%u as prefix is invalid, only /32 (or none) is supported.\n",
					addr.bitlen);
				req.r.rtm_dst_len = 32;
			} else if (req.r.rtm_family == AF_INET6 && addr.bitlen != 128) {
				fprintf(stderr,
					"Warning: /%u as prefix is invalid, only /128 (or none) is supported.\n",
					addr.bitlen);
				req.r.rtm_dst_len = 128;
			} else
				req.r.rtm_dst_len = addr.bitlen;
			address_found = true;
		}
		argc--; argv++;
	}

	if (!address_found) {
		fprintf(stderr, "need at least a destination address\n");
		return -1;
	}

	if (idev || odev)  {
		int idx;

		if (idev) {
			idx = ll_name_to_index(idev);
			if (!idx)
				return nodev(idev);
			addattr32(&req.n, sizeof(req), RTA_IIF, idx);
		}
		if (odev) {
			idx = ll_name_to_index(odev);
			if (!idx)
				return nodev(odev);
			addattr32(&req.n, sizeof(req), RTA_OIF, idx);
		}
	}
	if (mark)
		addattr32(&req.n, sizeof(req), RTA_MARK, mark);

	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = AF_INET;

	/* Only IPv4 supports the RTM_F_LOOKUP_TABLE flag */
	if (req.r.rtm_family == AF_INET)
		req.r.rtm_flags |= RTM_F_LOOKUP_TABLE;
	if (fib_match)
		req.r.rtm_flags |= RTM_F_FIB_MATCH;

	if (rtnl_talk(&rth, &req.n, &answer) < 0)
		return -2;

	new_json_obj(json);

	if (connected && !from_ok) {
		struct rtmsg *r = NLMSG_DATA(answer);
		int len = answer->nlmsg_len;
		struct rtattr *tb[RTA_MAX+1];

		if (print_route(answer, (void *)stdout) < 0) {
			fprintf(stderr, "An error :-)\n");
			free(answer);
			return -1;
		}

		if (answer->nlmsg_type != RTM_NEWROUTE) {
			fprintf(stderr, "Not a route?\n");
			free(answer);
			return -1;
		}
		len -= NLMSG_LENGTH(sizeof(*r));
		if (len < 0) {
			fprintf(stderr, "Wrong len %d\n", len);
			free(answer);
			return -1;
		}

		parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

		if (tb[RTA_PREFSRC]) {
			tb[RTA_PREFSRC]->rta_type = RTA_SRC;
			r->rtm_src_len = 8*RTA_PAYLOAD(tb[RTA_PREFSRC]);
		} else if (!tb[RTA_SRC]) {
			fprintf(stderr, "Failed to connect the route\n");
			free(answer);
			return -1;
		}
		if (!odev && tb[RTA_OIF])
			tb[RTA_OIF]->rta_type = 0;
		if (tb[RTA_GATEWAY])
			tb[RTA_GATEWAY]->rta_type = 0;
		if (tb[RTA_VIA])
			tb[RTA_VIA]->rta_type = 0;
		if (!idev && tb[RTA_IIF])
			tb[RTA_IIF]->rta_type = 0;
		req.n.nlmsg_flags = NLM_F_REQUEST;
		req.n.nlmsg_type = RTM_GETROUTE;

		delete_json_obj();
		free(answer);
		if (rtnl_talk(&rth, &req.n, &answer) < 0)
			return -2;
	}

	if (print_route(answer, (void *)stdout) < 0) {
		fprintf(stderr, "An error :-)\n");
		free(answer);
		return -1;
	}

	delete_json_obj();
	free(answer);
	return 0;
}

static int rtattr_cmp(const struct rtattr *rta1, const struct rtattr *rta2)
{
	if (!rta1 || !rta2 || rta1->rta_len != rta2->rta_len)
		return 1;

	return memcmp(RTA_DATA(rta1), RTA_DATA(rta2), RTA_PAYLOAD(rta1));
}

static int restore_handler(struct rtnl_ctrl_data *ctrl,
			   struct nlmsghdr *n, void *arg)
{
	struct rtmsg *r = NLMSG_DATA(n);
	struct rtattr *tb[RTA_MAX+1];
	int len = n->nlmsg_len - NLMSG_LENGTH(sizeof(*r));
	int ret, prio = *(int *)arg;

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

	/* Restore routes in correct order:
	 * 0. ones for local addresses,
	 * 1. ones for local networks,
	 * 2. others (remote networks/hosts).
	 */
	if (!prio && !tb[RTA_GATEWAY] && (!tb[RTA_PREFSRC] ||
	    !rtattr_cmp(tb[RTA_PREFSRC], tb[RTA_DST])))
		goto restore;
	else if (prio == 1 && !tb[RTA_GATEWAY] && tb[RTA_PREFSRC] &&
		 rtattr_cmp(tb[RTA_PREFSRC], tb[RTA_DST]))
		goto restore;
	else if (prio == 2 && tb[RTA_GATEWAY])
		goto restore;

	return 0;

restore:
	n->nlmsg_flags |= NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;

	ll_init_map(&rth);

	ret = rtnl_talk(&rth, n, NULL);
	if ((ret < 0) && (errno == EEXIST))
		ret = 0;

	return ret;
}

static int route_dump_check_magic(void)
{
	int ret;
	__u32 magic = 0;

	if (isatty(STDIN_FILENO)) {
		fprintf(stderr, "Can't restore route dump from a terminal\n");
		return -1;
	}

	ret = fread(&magic, sizeof(magic), 1, stdin);
	if (magic != route_dump_magic) {
		fprintf(stderr, "Magic mismatch (%d elems, %x magic)\n", ret, magic);
		return -1;
	}

	return 0;
}

static int iproute_restore(void)
{
	int pos, prio;

	if (route_dump_check_magic())
		return -1;

	pos = ftell(stdin);
	if (pos == -1) {
		perror("Failed to restore: ftell");
		return -1;
	}

	for (prio = 0; prio < 3; prio++) {
		int err;

		err = rtnl_from_file(stdin, &restore_handler, &prio);
		if (err)
			return -2;

		if (fseek(stdin, pos, SEEK_SET) == -1) {
			perror("Failed to restore: fseek");
			return -1;
		}
	}

	return 0;
}

static int show_handler(struct rtnl_ctrl_data *ctrl,
			struct nlmsghdr *n, void *arg)
{
	print_route(n, stdout);
	return 0;
}

static int iproute_showdump(void)
{
	if (route_dump_check_magic())
		return -1;

	if (rtnl_from_file(stdin, &show_handler, NULL))
		return -2;

	return 0;
}

void iproute_reset_filter(int ifindex)
{
	memset(&filter, 0, sizeof(filter));
	filter.mdst.bitlen = -1;
	filter.msrc.bitlen = -1;
	filter.oif = ifindex;
	if (filter.oif > 0)
		filter.oifmask = -1;
}

int do_iproute(int argc, char **argv)
{
	if (argc < 1)
		return iproute_list_flush_or_save(0, NULL, IPROUTE_LIST);

	if (matches(*argv, "add") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_CREATE|NLM_F_EXCL,
				      argc-1, argv+1);
	if (matches(*argv, "change") == 0 || strcmp(*argv, "chg") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_REPLACE,
				      argc-1, argv+1);
	if (matches(*argv, "replace") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_CREATE|NLM_F_REPLACE,
				      argc-1, argv+1);
	if (matches(*argv, "prepend") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_CREATE,
				      argc-1, argv+1);
	if (matches(*argv, "append") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_CREATE|NLM_F_APPEND,
				      argc-1, argv+1);
	if (matches(*argv, "test") == 0)
		return iproute_modify(RTM_NEWROUTE, NLM_F_EXCL,
				      argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return iproute_modify(RTM_DELROUTE, 0,
				      argc-1, argv+1);
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return iproute_list_flush_or_save(argc-1, argv+1, IPROUTE_LIST);
	if (matches(*argv, "get") == 0)
		return iproute_get(argc-1, argv+1);
	if (matches(*argv, "flush") == 0)
		return iproute_list_flush_or_save(argc-1, argv+1, IPROUTE_FLUSH);
	if (matches(*argv, "save") == 0)
		return iproute_list_flush_or_save(argc-1, argv+1, IPROUTE_SAVE);
	if (matches(*argv, "restore") == 0)
		return iproute_restore();
	if (matches(*argv, "showdump") == 0)
		return iproute_showdump();
	if (matches(*argv, "help") == 0)
		usage();

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip route help\".\n", *argv);
	exit(-1);
}
