/*
 * lib/route/link/inet6.c	AF_INET6 link operations
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link/api.h>

struct inet6_data
{
	uint32_t		i6_flags;
	struct ifla_cacheinfo	i6_cacheinfo;
	uint32_t		i6_conf[DEVCONF_MAX];
};

static void *inet6_alloc(struct rtnl_link *link)
{
	return calloc(1, sizeof(struct inet6_data));
}

static void *inet6_clone(struct rtnl_link *link, void *data)
{
	struct inet6_data *i6;

	if ((i6 = inet6_alloc(link)))
		memcpy(i6, data, sizeof(*i6));

	return i6;
}

static void inet6_free(struct rtnl_link *link, void *data)
{
	free(data);
}

static struct nla_policy inet6_policy[IFLA_INET6_MAX+1] = {
	[IFLA_INET6_FLAGS]	= { .type = NLA_U32 },
	[IFLA_INET6_CACHEINFO]	= { .minlen = sizeof(struct ifla_cacheinfo) },
	[IFLA_INET6_CONF]	= { .minlen = DEVCONF_MAX * 4 },
	[IFLA_INET6_STATS]	= { .minlen = __IPSTATS_MIB_MAX * 8 },
	[IFLA_INET6_ICMP6STATS]	= { .minlen = __ICMP6_MIB_MAX * 8 },
};

static int inet6_parse_protinfo(struct rtnl_link *link, struct nlattr *attr,
				void *data)
{
	struct inet6_data *i6 = data;
	struct nlattr *tb[IFLA_INET6_MAX+1];
	int err;

	err = nla_parse_nested(tb, IFLA_INET6_MAX, attr, inet6_policy);
	if (err < 0)
		return err;

	if (tb[IFLA_INET6_FLAGS])
		i6->i6_flags = nla_get_u32(tb[IFLA_INET6_FLAGS]);

	if (tb[IFLA_INET6_CACHEINFO])
		nla_memcpy(&i6->i6_cacheinfo, tb[IFLA_INET6_CACHEINFO],
			   sizeof(i6->i6_cacheinfo));

	if (tb[IFLA_INET6_CONF])
		nla_memcpy(&i6->i6_conf, tb[IFLA_INET6_CONF],
			   sizeof(i6->i6_conf));
 
	/*
	 * Due to 32bit data alignment, these addresses must be copied to an
	 * aligned location prior to access.
	 */
	if (tb[IFLA_INET6_STATS]) {
		unsigned char *cnt = nla_data(tb[IFLA_INET6_STATS]);
		uint64_t stat;
		int i;

		for (i = 1; i < __IPSTATS_MIB_MAX; i++) {
			memcpy(&stat, &cnt[i * sizeof(stat)], sizeof(stat));
			rtnl_link_set_stat(link, RTNL_LINK_IP6_INPKTS + i - 1,
					   stat);
		}
	}

	if (tb[IFLA_INET6_ICMP6STATS]) {
		unsigned char *cnt = nla_data(tb[IFLA_INET6_ICMP6STATS]);
		uint64_t stat;
		int i;

		for (i = 1; i < __ICMP6_MIB_MAX; i++) {
			memcpy(&stat, &cnt[i * sizeof(stat)], sizeof(stat));
			rtnl_link_set_stat(link, RTNL_LINK_ICMP6_INMSGS + i - 1,
					   stat);
		}
	}

	return 0;
}

/* These live in include/net/if_inet6.h and should be moved to include/linux */
#define IF_RA_OTHERCONF	0x80
#define IF_RA_MANAGED	0x40
#define IF_RA_RCVD	0x20
#define IF_RS_SENT	0x10
#define IF_READY	0x80000000

static const struct trans_tbl inet6_flags[] = {
	__ADD(IF_RA_OTHERCONF, ra_otherconf)
	__ADD(IF_RA_MANAGED, ra_managed)
	__ADD(IF_RA_RCVD, ra_rcvd)
	__ADD(IF_RS_SENT, rs_sent)
	__ADD(IF_READY, ready)
};

static char *inet6_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, inet6_flags,
			   ARRAY_SIZE(inet6_flags));
}

static const struct trans_tbl inet6_devconf[] = {
	__ADD(DEVCONF_FORWARDING, forwarding)
	__ADD(DEVCONF_HOPLIMIT, hoplimit)
	__ADD(DEVCONF_MTU6, mtu6)
	__ADD(DEVCONF_ACCEPT_RA, accept_ra)
	__ADD(DEVCONF_ACCEPT_REDIRECTS, accept_redirects)
	__ADD(DEVCONF_AUTOCONF, autoconf)
	__ADD(DEVCONF_DAD_TRANSMITS, dad_transmits)
	__ADD(DEVCONF_RTR_SOLICITS, rtr_solicits)
	__ADD(DEVCONF_RTR_SOLICIT_INTERVAL, rtr_solicit_interval)
	__ADD(DEVCONF_RTR_SOLICIT_DELAY, rtr_solicit_delay)
	__ADD(DEVCONF_USE_TEMPADDR, use_tempaddr)
	__ADD(DEVCONF_TEMP_VALID_LFT, temp_valid_lft)
	__ADD(DEVCONF_TEMP_PREFERED_LFT, temp_prefered_lft)
	__ADD(DEVCONF_REGEN_MAX_RETRY, regen_max_retry)
	__ADD(DEVCONF_MAX_DESYNC_FACTOR, max_desync_factor)
	__ADD(DEVCONF_MAX_ADDRESSES, max_addresses)
	__ADD(DEVCONF_FORCE_MLD_VERSION, force_mld_version)
	__ADD(DEVCONF_ACCEPT_RA_DEFRTR, accept_ra_defrtr)
	__ADD(DEVCONF_ACCEPT_RA_PINFO, accept_ra_pinfo)
	__ADD(DEVCONF_ACCEPT_RA_RTR_PREF, accept_ra_rtr_pref)
	__ADD(DEVCONF_RTR_PROBE_INTERVAL, rtr_probe_interval)
	__ADD(DEVCONF_ACCEPT_RA_RT_INFO_MAX_PLEN, accept_ra_rt_info)
	__ADD(DEVCONF_PROXY_NDP, proxy_ndp)
	__ADD(DEVCONF_OPTIMISTIC_DAD, optimistic_dad)
	__ADD(DEVCONF_ACCEPT_SOURCE_ROUTE, accept_source_route)
	__ADD(DEVCONF_MC_FORWARDING, mc_forwarding)
	__ADD(DEVCONF_DISABLE_IPV6, disable_ipv6)
	__ADD(DEVCONF_ACCEPT_DAD, accept_dad)
	__ADD(DEVCONF_FORCE_TLLAO, force_tllao)
};

static char *inet6_devconf2str(int type, char *buf, size_t len)
{
	return __type2str(type, buf, len, inet6_devconf,
			  ARRAY_SIZE(inet6_devconf));
}


static void inet6_dump_details(struct rtnl_link *link,
				struct nl_dump_params *p, void *data)
{
	struct inet6_data *i6 = data;
	char buf[64], buf2[64];
	int i, n = 0;

	nl_dump_line(p, "    ipv6 max-reasm-len %s",
		nl_size2str(i6->i6_cacheinfo.max_reasm_len, buf, sizeof(buf)));

	nl_dump(p, " <%s>\n",
		inet6_flags2str(i6->i6_flags, buf, sizeof(buf)));


	nl_dump_line(p, "      create-stamp %.2fs reachable-time %s",
		(double) i6->i6_cacheinfo.tstamp / 100.,
		nl_msec2str(i6->i6_cacheinfo.reachable_time, buf, sizeof(buf)));

	nl_dump(p, " retrans-time %s\n",
		nl_msec2str(i6->i6_cacheinfo.retrans_time, buf, sizeof(buf)));

	nl_dump_line(p, "      devconf:\n");
	nl_dump_line(p, "      ");

	for (i = 0; i < DEVCONF_MAX; i++) {
		uint32_t value = i6->i6_conf[i];
		int x, offset;

		switch (i) {
		case DEVCONF_TEMP_VALID_LFT:
		case DEVCONF_TEMP_PREFERED_LFT:
			nl_msec2str((uint64_t) value * 1000., buf2, sizeof(buf2));
			break;

		case DEVCONF_RTR_PROBE_INTERVAL:
		case DEVCONF_RTR_SOLICIT_INTERVAL:
		case DEVCONF_RTR_SOLICIT_DELAY:
			nl_msec2str(value, buf2, sizeof(buf2));
			break;

		default:
			snprintf(buf2, sizeof(buf2), "%u", value);
			break;
			
		}

		inet6_devconf2str(i, buf, sizeof(buf));

		offset = 23 - strlen(buf2);
		if (offset < 0)
			offset = 0;

		for (x = strlen(buf); x < offset; x++)
			buf[x] = ' ';

		strncpy(&buf[offset], buf2, strlen(buf2));

		nl_dump_line(p, "%s", buf);

		if (++n == 3) {
			nl_dump(p, "\n");
			nl_dump_line(p, "      ");
			n = 0;
		} else
			nl_dump(p, "  ");
	}

	if (n != 0)
		nl_dump(p, "\n");
}

static void inet6_dump_stats(struct rtnl_link *link,
			     struct nl_dump_params *p, void *data)
{
	double octets;
	char *octetsUnit;

	nl_dump(p, "    IPv6:       InPkts           InOctets     "
		   "    InDiscards         InDelivers\n");
	nl_dump(p, "    %18llu ", link->l_stats[RTNL_LINK_IP6_INPKTS]);

	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_INOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s ", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B ", 0);
	
	nl_dump(p, "%18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_INDISCARDS],
		link->l_stats[RTNL_LINK_IP6_INDELIVERS]);

	nl_dump(p, "               OutPkts          OutOctets     "
		   "   OutDiscards        OutForwards\n");

	nl_dump(p, "    %18llu ", link->l_stats[RTNL_LINK_IP6_OUTPKTS]);

	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_OUTOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s ", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B ", 0);

	nl_dump(p, "%18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_OUTDISCARDS],
		link->l_stats[RTNL_LINK_IP6_OUTFORWDATAGRAMS]);

	nl_dump(p, "           InMcastPkts      InMcastOctets     "
		   "   InBcastPkts     InBcastOctests\n");

	nl_dump(p, "    %18llu ", link->l_stats[RTNL_LINK_IP6_INMCASTPKTS]);

	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_INMCASTOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s ", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B ", 0);

	nl_dump(p, "%18llu ", link->l_stats[RTNL_LINK_IP6_INBCASTPKTS]);
	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_INBCASTOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s\n", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B\n", 0);

	nl_dump(p, "          OutMcastPkts     OutMcastOctets     "
		   "  OutBcastPkts    OutBcastOctests\n");

	nl_dump(p, "    %18llu ", link->l_stats[RTNL_LINK_IP6_OUTMCASTPKTS]);

	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_OUTMCASTOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s ", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B ", 0);

	nl_dump(p, "%18llu ", link->l_stats[RTNL_LINK_IP6_OUTBCASTPKTS]);
	octets = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_IP6_OUTBCASTOCTETS],
				      &octetsUnit);
	if (octets)
		nl_dump(p, "%14.2f %3s\n", octets, octetsUnit);
	else
		nl_dump(p, "%16llu B\n", 0);

	nl_dump(p, "              ReasmOKs         ReasmFails     "
		   "    ReasmReqds       ReasmTimeout\n");
	nl_dump(p, "    %18llu %18llu %18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_REASMOKS],
		link->l_stats[RTNL_LINK_IP6_REASMFAILS],
		link->l_stats[RTNL_LINK_IP6_REASMREQDS],
		link->l_stats[RTNL_LINK_IP6_REASMTIMEOUT]);

	nl_dump(p, "               FragOKs          FragFails    "
		   "    FragCreates\n");
	nl_dump(p, "    %18llu %18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_FRAGOKS],
		link->l_stats[RTNL_LINK_IP6_FRAGFAILS],
		link->l_stats[RTNL_LINK_IP6_FRAGCREATES]);

	nl_dump(p, "           InHdrErrors      InTooBigErrors   "
		   "     InNoRoutes       InAddrErrors\n");
	nl_dump(p, "    %18llu %18llu %18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_INHDRERRORS],
		link->l_stats[RTNL_LINK_IP6_INTOOBIGERRORS],
		link->l_stats[RTNL_LINK_IP6_INNOROUTES],
		link->l_stats[RTNL_LINK_IP6_INADDRERRORS]);

	nl_dump(p, "       InUnknownProtos     InTruncatedPkts   "
		   "    OutNoRoutes\n");
	nl_dump(p, "    %18llu %18llu %18llu\n",
		link->l_stats[RTNL_LINK_IP6_INUNKNOWNPROTOS],
		link->l_stats[RTNL_LINK_IP6_INTRUNCATEDPKTS],
		link->l_stats[RTNL_LINK_IP6_OUTNOROUTES]);

	nl_dump(p, "    ICMPv6:     InMsgs           InErrors        "
		   "    OutMsgs          OutErrors\n");
	nl_dump(p, "    %18llu %18llu %18llu %18llu\n",
		link->l_stats[RTNL_LINK_ICMP6_INMSGS],
		link->l_stats[RTNL_LINK_ICMP6_INERRORS],
		link->l_stats[RTNL_LINK_ICMP6_OUTMSGS],
		link->l_stats[RTNL_LINK_ICMP6_OUTERRORS]);
}

static const struct nla_policy protinfo_policy = {
	.type			= NLA_NESTED,
};

static struct rtnl_link_af_ops inet6_ops = {
	.ao_family			= AF_INET6,
	.ao_alloc			= &inet6_alloc,
	.ao_clone			= &inet6_clone,
	.ao_free			= &inet6_free,
	.ao_parse_protinfo		= &inet6_parse_protinfo,
	.ao_parse_af			= &inet6_parse_protinfo,
	.ao_dump[NL_DUMP_DETAILS]	= &inet6_dump_details,
	.ao_dump[NL_DUMP_STATS]		= &inet6_dump_stats,
	.ao_protinfo_policy		= &protinfo_policy,
};

static void __init inet6_init(void)
{
	rtnl_link_af_register(&inet6_ops);
}

static void __exit inet6_exit(void)
{
	rtnl_link_af_unregister(&inet6_ops);
}
