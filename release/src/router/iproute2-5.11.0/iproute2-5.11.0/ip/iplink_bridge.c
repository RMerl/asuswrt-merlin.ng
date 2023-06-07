/*
 * iplink_bridge.c	Bridge device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <linux/if_link.h>
#include <linux/if_bridge.h>
#include <net/if.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static unsigned int xstats_print_attr;
static int filter_index;

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... bridge [ fdb_flush ]\n"
		"		  [ forward_delay FORWARD_DELAY ]\n"
		"		  [ hello_time HELLO_TIME ]\n"
		"		  [ max_age MAX_AGE ]\n"
		"		  [ ageing_time AGEING_TIME ]\n"
		"		  [ stp_state STP_STATE ]\n"
		"		  [ priority PRIORITY ]\n"
		"		  [ group_fwd_mask MASK ]\n"
		"		  [ group_address ADDRESS ]\n"
		"		  [ vlan_filtering VLAN_FILTERING ]\n"
		"		  [ vlan_protocol VLAN_PROTOCOL ]\n"
		"		  [ vlan_default_pvid VLAN_DEFAULT_PVID ]\n"
		"		  [ vlan_stats_enabled VLAN_STATS_ENABLED ]\n"
		"		  [ vlan_stats_per_port VLAN_STATS_PER_PORT ]\n"
		"		  [ mcast_snooping MULTICAST_SNOOPING ]\n"
		"		  [ mcast_router MULTICAST_ROUTER ]\n"
		"		  [ mcast_query_use_ifaddr MCAST_QUERY_USE_IFADDR ]\n"
		"		  [ mcast_querier MULTICAST_QUERIER ]\n"
		"		  [ mcast_hash_elasticity HASH_ELASTICITY ]\n"
		"		  [ mcast_hash_max HASH_MAX ]\n"
		"		  [ mcast_last_member_count LAST_MEMBER_COUNT ]\n"
		"		  [ mcast_startup_query_count STARTUP_QUERY_COUNT ]\n"
		"		  [ mcast_last_member_interval LAST_MEMBER_INTERVAL ]\n"
		"		  [ mcast_membership_interval MEMBERSHIP_INTERVAL ]\n"
		"		  [ mcast_querier_interval QUERIER_INTERVAL ]\n"
		"		  [ mcast_query_interval QUERY_INTERVAL ]\n"
		"		  [ mcast_query_response_interval QUERY_RESPONSE_INTERVAL ]\n"
		"		  [ mcast_startup_query_interval STARTUP_QUERY_INTERVAL ]\n"
		"		  [ mcast_stats_enabled MCAST_STATS_ENABLED ]\n"
		"		  [ mcast_igmp_version IGMP_VERSION ]\n"
		"		  [ mcast_mld_version MLD_VERSION ]\n"
		"		  [ nf_call_iptables NF_CALL_IPTABLES ]\n"
		"		  [ nf_call_ip6tables NF_CALL_IP6TABLES ]\n"
		"		  [ nf_call_arptables NF_CALL_ARPTABLES ]\n"
		"\n"
		"Where: VLAN_PROTOCOL := { 802.1Q | 802.1ad }\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

void br_dump_bridge_id(const struct ifla_bridge_id *id, char *buf, size_t len)
{
	char eaddr[18];

	ether_ntoa_r((const struct ether_addr *)id->addr, eaddr);
	snprintf(buf, len, "%.2x%.2x.%s", id->prio[0], id->prio[1], eaddr);
}

static int bridge_parse_opt(struct link_util *lu, int argc, char **argv,
			    struct nlmsghdr *n)
{
	__u32 val;

	while (argc > 0) {
		if (matches(*argv, "forward_delay") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid forward_delay", *argv);

			addattr32(n, 1024, IFLA_BR_FORWARD_DELAY, val);
		} else if (matches(*argv, "hello_time") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid hello_time", *argv);

			addattr32(n, 1024, IFLA_BR_HELLO_TIME, val);
		} else if (matches(*argv, "max_age") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid max_age", *argv);

			addattr32(n, 1024, IFLA_BR_MAX_AGE, val);
		} else if (matches(*argv, "ageing_time") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid ageing_time", *argv);

			addattr32(n, 1024, IFLA_BR_AGEING_TIME, val);
		} else if (matches(*argv, "stp_state") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid stp_state", *argv);

			addattr32(n, 1024, IFLA_BR_STP_STATE, val);
		} else if (matches(*argv, "priority") == 0) {
			__u16 prio;

			NEXT_ARG();
			if (get_u16(&prio, *argv, 0))
				invarg("invalid priority", *argv);

			addattr16(n, 1024, IFLA_BR_PRIORITY, prio);
		} else if (matches(*argv, "vlan_filtering") == 0) {
			__u8 vlan_filter;

			NEXT_ARG();
			if (get_u8(&vlan_filter, *argv, 0))
				invarg("invalid vlan_filtering", *argv);

			addattr8(n, 1024, IFLA_BR_VLAN_FILTERING, vlan_filter);
		} else if (matches(*argv, "vlan_protocol") == 0) {
			__u16 vlan_proto;

			NEXT_ARG();
			if (ll_proto_a2n(&vlan_proto, *argv))
				invarg("invalid vlan_protocol", *argv);

			addattr16(n, 1024, IFLA_BR_VLAN_PROTOCOL, vlan_proto);
		} else if (matches(*argv, "group_fwd_mask") == 0) {
			__u16 fwd_mask;

			NEXT_ARG();
			if (get_u16(&fwd_mask, *argv, 0))
				invarg("invalid group_fwd_mask", *argv);

			addattr16(n, 1024, IFLA_BR_GROUP_FWD_MASK, fwd_mask);
		} else if (matches(*argv, "group_address") == 0) {
			char llabuf[32];
			int len;

			NEXT_ARG();
			len = ll_addr_a2n(llabuf, sizeof(llabuf), *argv);
			if (len < 0)
				return -1;
			addattr_l(n, 1024, IFLA_BR_GROUP_ADDR, llabuf, len);
		} else if (matches(*argv, "fdb_flush") == 0) {
			addattr(n, 1024, IFLA_BR_FDB_FLUSH);
		} else if (matches(*argv, "vlan_default_pvid") == 0) {
			__u16 default_pvid;

			NEXT_ARG();
			if (get_u16(&default_pvid, *argv, 0))
				invarg("invalid vlan_default_pvid", *argv);

			addattr16(n, 1024, IFLA_BR_VLAN_DEFAULT_PVID,
				  default_pvid);
		} else if (matches(*argv, "vlan_stats_enabled") == 0) {
			__u8 vlan_stats_enabled;

			NEXT_ARG();
			if (get_u8(&vlan_stats_enabled, *argv, 0))
				invarg("invalid vlan_stats_enabled", *argv);
			addattr8(n, 1024, IFLA_BR_VLAN_STATS_ENABLED,
				  vlan_stats_enabled);
		} else if (matches(*argv, "vlan_stats_per_port") == 0) {
			__u8 vlan_stats_per_port;

			NEXT_ARG();
			if (get_u8(&vlan_stats_per_port, *argv, 0))
				invarg("invalid vlan_stats_per_port", *argv);
			addattr8(n, 1024, IFLA_BR_VLAN_STATS_PER_PORT,
				 vlan_stats_per_port);
		} else if (matches(*argv, "mcast_router") == 0) {
			__u8 mcast_router;

			NEXT_ARG();
			if (get_u8(&mcast_router, *argv, 0))
				invarg("invalid mcast_router", *argv);

			addattr8(n, 1024, IFLA_BR_MCAST_ROUTER, mcast_router);
		} else if (matches(*argv, "mcast_snooping") == 0) {
			__u8 mcast_snoop;

			NEXT_ARG();
			if (get_u8(&mcast_snoop, *argv, 0))
				invarg("invalid mcast_snooping", *argv);

			addattr8(n, 1024, IFLA_BR_MCAST_SNOOPING, mcast_snoop);
		} else if (matches(*argv, "mcast_query_use_ifaddr") == 0) {
			__u8 mcast_qui;

			NEXT_ARG();
			if (get_u8(&mcast_qui, *argv, 0))
				invarg("invalid mcast_query_use_ifaddr",
				       *argv);

			addattr8(n, 1024, IFLA_BR_MCAST_QUERY_USE_IFADDR,
				 mcast_qui);
		} else if (matches(*argv, "mcast_querier") == 0) {
			__u8 mcast_querier;

			NEXT_ARG();
			if (get_u8(&mcast_querier, *argv, 0))
				invarg("invalid mcast_querier", *argv);

			addattr8(n, 1024, IFLA_BR_MCAST_QUERIER, mcast_querier);
		} else if (matches(*argv, "mcast_hash_elasticity") == 0) {
			__u32 mcast_hash_el;

			NEXT_ARG();
			if (get_u32(&mcast_hash_el, *argv, 0))
				invarg("invalid mcast_hash_elasticity",
				       *argv);

			addattr32(n, 1024, IFLA_BR_MCAST_HASH_ELASTICITY,
				  mcast_hash_el);
		} else if (matches(*argv, "mcast_hash_max") == 0) {
			__u32 mcast_hash_max;

			NEXT_ARG();
			if (get_u32(&mcast_hash_max, *argv, 0))
				invarg("invalid mcast_hash_max", *argv);

			addattr32(n, 1024, IFLA_BR_MCAST_HASH_MAX,
				  mcast_hash_max);
		} else if (matches(*argv, "mcast_last_member_count") == 0) {
			__u32 mcast_lmc;

			NEXT_ARG();
			if (get_u32(&mcast_lmc, *argv, 0))
				invarg("invalid mcast_last_member_count",
				       *argv);

			addattr32(n, 1024, IFLA_BR_MCAST_LAST_MEMBER_CNT,
				  mcast_lmc);
		} else if (matches(*argv, "mcast_startup_query_count") == 0) {
			__u32 mcast_sqc;

			NEXT_ARG();
			if (get_u32(&mcast_sqc, *argv, 0))
				invarg("invalid mcast_startup_query_count",
				       *argv);

			addattr32(n, 1024, IFLA_BR_MCAST_STARTUP_QUERY_CNT,
				  mcast_sqc);
		} else if (matches(*argv, "mcast_last_member_interval") == 0) {
			__u64 mcast_last_member_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_last_member_intvl, *argv, 0))
				invarg("invalid mcast_last_member_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_LAST_MEMBER_INTVL,
				  mcast_last_member_intvl);
		} else if (matches(*argv, "mcast_membership_interval") == 0) {
			__u64 mcast_membership_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_membership_intvl, *argv, 0))
				invarg("invalid mcast_membership_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_MEMBERSHIP_INTVL,
				  mcast_membership_intvl);
		} else if (matches(*argv, "mcast_querier_interval") == 0) {
			__u64 mcast_querier_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_querier_intvl, *argv, 0))
				invarg("invalid mcast_querier_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_QUERIER_INTVL,
				  mcast_querier_intvl);
		} else if (matches(*argv, "mcast_query_interval") == 0) {
			__u64 mcast_query_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_query_intvl, *argv, 0))
				invarg("invalid mcast_query_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_QUERY_INTVL,
				  mcast_query_intvl);
		} else if (!matches(*argv, "mcast_query_response_interval")) {
			__u64 mcast_query_resp_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_query_resp_intvl, *argv, 0))
				invarg("invalid mcast_query_response_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_QUERY_RESPONSE_INTVL,
				  mcast_query_resp_intvl);
		} else if (!matches(*argv, "mcast_startup_query_interval")) {
			__u64 mcast_startup_query_intvl;

			NEXT_ARG();
			if (get_u64(&mcast_startup_query_intvl, *argv, 0))
				invarg("invalid mcast_startup_query_interval",
				       *argv);

			addattr64(n, 1024, IFLA_BR_MCAST_STARTUP_QUERY_INTVL,
				  mcast_startup_query_intvl);
		} else if (matches(*argv, "mcast_stats_enabled") == 0) {
			__u8 mcast_stats_enabled;

			NEXT_ARG();
			if (get_u8(&mcast_stats_enabled, *argv, 0))
				invarg("invalid mcast_stats_enabled", *argv);
			addattr8(n, 1024, IFLA_BR_MCAST_STATS_ENABLED,
				  mcast_stats_enabled);
		} else if (matches(*argv, "mcast_igmp_version") == 0) {
			__u8 igmp_version;

			NEXT_ARG();
			if (get_u8(&igmp_version, *argv, 0))
				invarg("invalid mcast_igmp_version", *argv);
			addattr8(n, 1024, IFLA_BR_MCAST_IGMP_VERSION,
				  igmp_version);
		} else if (matches(*argv, "mcast_mld_version") == 0) {
			__u8 mld_version;

			NEXT_ARG();
			if (get_u8(&mld_version, *argv, 0))
				invarg("invalid mcast_mld_version", *argv);
			addattr8(n, 1024, IFLA_BR_MCAST_MLD_VERSION,
				  mld_version);
		} else if (matches(*argv, "nf_call_iptables") == 0) {
			__u8 nf_call_ipt;

			NEXT_ARG();
			if (get_u8(&nf_call_ipt, *argv, 0))
				invarg("invalid nf_call_iptables", *argv);

			addattr8(n, 1024, IFLA_BR_NF_CALL_IPTABLES,
				 nf_call_ipt);
		} else if (matches(*argv, "nf_call_ip6tables") == 0) {
			__u8 nf_call_ip6t;

			NEXT_ARG();
			if (get_u8(&nf_call_ip6t, *argv, 0))
				invarg("invalid nf_call_ip6tables", *argv);

			addattr8(n, 1024, IFLA_BR_NF_CALL_IP6TABLES,
				 nf_call_ip6t);
		} else if (matches(*argv, "nf_call_arptables") == 0) {
			__u8 nf_call_arpt;

			NEXT_ARG();
			if (get_u8(&nf_call_arpt, *argv, 0))
				invarg("invalid nf_call_arptables", *argv);

			addattr8(n, 1024, IFLA_BR_NF_CALL_ARPTABLES,
				 nf_call_arpt);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "bridge: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void _bridge_print_timer(FILE *f,
				const char *attr,
				struct rtattr *timer)
{
	struct timeval tv;

	__jiffies_to_tv(&tv, rta_getattr_u64(timer));
	if (is_json_context()) {
		json_writer_t *jw = get_json_writer();

		jsonw_name(jw, attr);
		jsonw_printf(jw, "%i.%.2i",
			     (int)tv.tv_sec,
			     (int)tv.tv_usec / 10000);
	} else {
		fprintf(f, "%s %4i.%.2i ", attr, (int)tv.tv_sec,
			(int)tv.tv_usec / 10000);
	}
}

static void bridge_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_BR_FORWARD_DELAY])
		print_uint(PRINT_ANY,
			   "forward_delay",
			   "forward_delay %u ",
			   rta_getattr_u32(tb[IFLA_BR_FORWARD_DELAY]));

	if (tb[IFLA_BR_HELLO_TIME])
		print_uint(PRINT_ANY,
			   "hello_time",
			   "hello_time %u ",
			   rta_getattr_u32(tb[IFLA_BR_HELLO_TIME]));

	if (tb[IFLA_BR_MAX_AGE])
		print_uint(PRINT_ANY,
			   "max_age",
			   "max_age %u ",
			   rta_getattr_u32(tb[IFLA_BR_MAX_AGE]));

	if (tb[IFLA_BR_AGEING_TIME])
		print_uint(PRINT_ANY,
			   "ageing_time",
			   "ageing_time %u ",
			   rta_getattr_u32(tb[IFLA_BR_AGEING_TIME]));

	if (tb[IFLA_BR_STP_STATE])
		print_uint(PRINT_ANY,
			   "stp_state",
			   "stp_state %u ",
			   rta_getattr_u32(tb[IFLA_BR_STP_STATE]));

	if (tb[IFLA_BR_PRIORITY])
		print_uint(PRINT_ANY,
			   "priority",
			   "priority %u ",
			   rta_getattr_u16(tb[IFLA_BR_PRIORITY]));

	if (tb[IFLA_BR_VLAN_FILTERING])
		print_uint(PRINT_ANY,
			   "vlan_filtering",
			   "vlan_filtering %u ",
			   rta_getattr_u8(tb[IFLA_BR_VLAN_FILTERING]));

	if (tb[IFLA_BR_VLAN_PROTOCOL]) {
		SPRINT_BUF(b1);

		print_string(PRINT_ANY,
			     "vlan_protocol",
			     "vlan_protocol %s ",
			     ll_proto_n2a(rta_getattr_u16(tb[IFLA_BR_VLAN_PROTOCOL]),
					  b1, sizeof(b1)));
	}

	if (tb[IFLA_BR_BRIDGE_ID]) {
		char bridge_id[32];

		br_dump_bridge_id(RTA_DATA(tb[IFLA_BR_BRIDGE_ID]), bridge_id,
				  sizeof(bridge_id));
		print_string(PRINT_ANY,
			     "bridge_id",
			     "bridge_id %s ",
			     bridge_id);
	}

	if (tb[IFLA_BR_ROOT_ID]) {
		char root_id[32];

		br_dump_bridge_id(RTA_DATA(tb[IFLA_BR_BRIDGE_ID]), root_id,
				  sizeof(root_id));
		print_string(PRINT_ANY,
			     "root_id",
			     "designated_root %s ",
			     root_id);
	}

	if (tb[IFLA_BR_ROOT_PORT])
		print_uint(PRINT_ANY,
			   "root_port",
			   "root_port %u ",
			   rta_getattr_u16(tb[IFLA_BR_ROOT_PORT]));

	if (tb[IFLA_BR_ROOT_PATH_COST])
		print_uint(PRINT_ANY,
			   "root_path_cost",
			   "root_path_cost %u ",
			   rta_getattr_u32(tb[IFLA_BR_ROOT_PATH_COST]));

	if (tb[IFLA_BR_TOPOLOGY_CHANGE])
		print_uint(PRINT_ANY,
			   "topology_change",
			   "topology_change %u ",
			   rta_getattr_u8(tb[IFLA_BR_TOPOLOGY_CHANGE]));

	if (tb[IFLA_BR_TOPOLOGY_CHANGE_DETECTED])
		print_uint(PRINT_ANY,
			   "topology_change_detected",
			   "topology_change_detected %u ",
			   rta_getattr_u8(tb[IFLA_BR_TOPOLOGY_CHANGE_DETECTED]));

	if (tb[IFLA_BR_HELLO_TIMER])
		_bridge_print_timer(f, "hello_timer", tb[IFLA_BR_HELLO_TIMER]);

	if (tb[IFLA_BR_TCN_TIMER])
		_bridge_print_timer(f, "tcn_timer", tb[IFLA_BR_TCN_TIMER]);

	if (tb[IFLA_BR_TOPOLOGY_CHANGE_TIMER])
		_bridge_print_timer(f, "topology_change_timer",
				    tb[IFLA_BR_TOPOLOGY_CHANGE_TIMER]);

	if (tb[IFLA_BR_GC_TIMER])
		_bridge_print_timer(f, "gc_timer", tb[IFLA_BR_GC_TIMER]);

	if (tb[IFLA_BR_VLAN_DEFAULT_PVID])
		print_uint(PRINT_ANY,
			   "vlan_default_pvid",
			   "vlan_default_pvid %u ",
			   rta_getattr_u16(tb[IFLA_BR_VLAN_DEFAULT_PVID]));

	if (tb[IFLA_BR_VLAN_STATS_ENABLED])
		print_uint(PRINT_ANY,
			   "vlan_stats_enabled",
			   "vlan_stats_enabled %u ",
			   rta_getattr_u8(tb[IFLA_BR_VLAN_STATS_ENABLED]));

	if (tb[IFLA_BR_VLAN_STATS_PER_PORT])
		print_uint(PRINT_ANY,
			   "vlan_stats_per_port",
			   "vlan_stats_per_port %u ",
			   rta_getattr_u8(tb[IFLA_BR_VLAN_STATS_PER_PORT]));

	if (tb[IFLA_BR_GROUP_FWD_MASK])
		print_0xhex(PRINT_ANY,
			    "group_fwd_mask",
			    "group_fwd_mask %#llx ",
			    rta_getattr_u16(tb[IFLA_BR_GROUP_FWD_MASK]));

	if (tb[IFLA_BR_GROUP_ADDR]) {
		SPRINT_BUF(mac);

		print_string(PRINT_ANY,
			     "group_addr",
			     "group_address %s ",
			     ll_addr_n2a(RTA_DATA(tb[IFLA_BR_GROUP_ADDR]),
					 RTA_PAYLOAD(tb[IFLA_BR_GROUP_ADDR]),
					 1 /*ARPHDR_ETHER*/, mac, sizeof(mac)));
	}

	if (tb[IFLA_BR_MCAST_SNOOPING])
		print_uint(PRINT_ANY,
			   "mcast_snooping",
			   "mcast_snooping %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_SNOOPING]));

	if (tb[IFLA_BR_MCAST_ROUTER])
		print_uint(PRINT_ANY,
			   "mcast_router",
			   "mcast_router %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_ROUTER]));

	if (tb[IFLA_BR_MCAST_QUERY_USE_IFADDR])
		print_uint(PRINT_ANY,
			   "mcast_query_use_ifaddr",
			   "mcast_query_use_ifaddr %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_QUERY_USE_IFADDR]));

	if (tb[IFLA_BR_MCAST_QUERIER])
		print_uint(PRINT_ANY,
			   "mcast_querier",
			   "mcast_querier %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_QUERIER]));

	if (tb[IFLA_BR_MCAST_HASH_ELASTICITY])
		print_uint(PRINT_ANY,
			   "mcast_hash_elasticity",
			   "mcast_hash_elasticity %u ",
			   rta_getattr_u32(tb[IFLA_BR_MCAST_HASH_ELASTICITY]));

	if (tb[IFLA_BR_MCAST_HASH_MAX])
		print_uint(PRINT_ANY,
			   "mcast_hash_max",
			   "mcast_hash_max %u ",
			   rta_getattr_u32(tb[IFLA_BR_MCAST_HASH_MAX]));

	if (tb[IFLA_BR_MCAST_LAST_MEMBER_CNT])
		print_uint(PRINT_ANY,
			   "mcast_last_member_cnt",
			   "mcast_last_member_count %u ",
			   rta_getattr_u32(tb[IFLA_BR_MCAST_LAST_MEMBER_CNT]));

	if (tb[IFLA_BR_MCAST_STARTUP_QUERY_CNT])
		print_uint(PRINT_ANY,
			   "mcast_startup_query_cnt",
			   "mcast_startup_query_count %u ",
			   rta_getattr_u32(tb[IFLA_BR_MCAST_STARTUP_QUERY_CNT]));

	if (tb[IFLA_BR_MCAST_LAST_MEMBER_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_last_member_intvl",
			     "mcast_last_member_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_LAST_MEMBER_INTVL]));

	if (tb[IFLA_BR_MCAST_MEMBERSHIP_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_membership_intvl",
			     "mcast_membership_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_MEMBERSHIP_INTVL]));

	if (tb[IFLA_BR_MCAST_QUERIER_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_querier_intvl",
			     "mcast_querier_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_QUERIER_INTVL]));

	if (tb[IFLA_BR_MCAST_QUERY_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_query_intvl",
			     "mcast_query_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_QUERY_INTVL]));

	if (tb[IFLA_BR_MCAST_QUERY_RESPONSE_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_query_response_intvl",
			     "mcast_query_response_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_QUERY_RESPONSE_INTVL]));

	if (tb[IFLA_BR_MCAST_STARTUP_QUERY_INTVL])
		print_lluint(PRINT_ANY,
			     "mcast_startup_query_intvl",
			     "mcast_startup_query_interval %llu ",
			     rta_getattr_u64(tb[IFLA_BR_MCAST_STARTUP_QUERY_INTVL]));

	if (tb[IFLA_BR_MCAST_STATS_ENABLED])
		print_uint(PRINT_ANY,
			   "mcast_stats_enabled",
			   "mcast_stats_enabled %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_STATS_ENABLED]));

	if (tb[IFLA_BR_MCAST_IGMP_VERSION])
		print_uint(PRINT_ANY,
			   "mcast_igmp_version",
			   "mcast_igmp_version %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_IGMP_VERSION]));

	if (tb[IFLA_BR_MCAST_MLD_VERSION])
		print_uint(PRINT_ANY,
			   "mcast_mld_version",
			   "mcast_mld_version %u ",
			   rta_getattr_u8(tb[IFLA_BR_MCAST_MLD_VERSION]));

	if (tb[IFLA_BR_NF_CALL_IPTABLES])
		print_uint(PRINT_ANY,
			   "nf_call_iptables",
			   "nf_call_iptables %u ",
			   rta_getattr_u8(tb[IFLA_BR_NF_CALL_IPTABLES]));

	if (tb[IFLA_BR_NF_CALL_IP6TABLES])
		print_uint(PRINT_ANY,
			   "nf_call_ip6tables",
			   "nf_call_ip6tables %u ",
			   rta_getattr_u8(tb[IFLA_BR_NF_CALL_IP6TABLES]));

	if (tb[IFLA_BR_NF_CALL_ARPTABLES])
		print_uint(PRINT_ANY,
			   "nf_call_arptables",
			   "nf_call_arptables %u ",
			   rta_getattr_u8(tb[IFLA_BR_NF_CALL_ARPTABLES]));
}

static void bridge_print_help(struct link_util *lu, int argc, char **argv,
			      FILE *f)
{
	print_explain(f);
}

static void bridge_print_xstats_help(struct link_util *lu, FILE *f)
{
	fprintf(f, "Usage: ... %s [ igmp ] [ dev DEVICE ]\n", lu->id);
}

static void bridge_print_stats_attr(struct rtattr *attr, int ifindex)
{
	struct rtattr *brtb[LINK_XSTATS_TYPE_MAX+1];
	struct bridge_stp_xstats *sstats;
	struct br_mcast_stats *mstats;
	struct rtattr *i, *list;
	const char *ifname = "";
	int rem;

	parse_rtattr(brtb, LINK_XSTATS_TYPE_MAX, RTA_DATA(attr),
	RTA_PAYLOAD(attr));
	if (!brtb[LINK_XSTATS_TYPE_BRIDGE])
		return;

	list = brtb[LINK_XSTATS_TYPE_BRIDGE];
	rem = RTA_PAYLOAD(list);
	open_json_object(NULL);
	ifname = ll_index_to_name(ifindex);
	print_string(PRINT_ANY, "ifname", "%-16s\n", ifname);
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (xstats_print_attr && i->rta_type != xstats_print_attr)
			continue;
		switch (i->rta_type) {
		case BRIDGE_XSTATS_MCAST:
			mstats = RTA_DATA(i);
			open_json_object("multicast");
			open_json_object("igmp_queries");
			print_string(PRINT_FP, NULL,
				     "%-16s    IGMP queries:\n", "");
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "rx_v1", "RX: v1 %llu ",
				  mstats->igmp_v1queries[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v2", "v2 %llu ",
				  mstats->igmp_v2queries[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v3", "v3 %llu\n",
				  mstats->igmp_v3queries[BR_MCAST_DIR_RX]);
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "tx_v1", "TX: v1 %llu ",
				  mstats->igmp_v1queries[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v2", "v2 %llu ",
				  mstats->igmp_v2queries[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v3", "v3 %llu\n",
				  mstats->igmp_v3queries[BR_MCAST_DIR_TX]);
			close_json_object();

			open_json_object("igmp_reports");
			print_string(PRINT_FP, NULL,
				     "%-16s    IGMP reports:\n", "");
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "rx_v1", "RX: v1 %llu ",
				  mstats->igmp_v1reports[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v2", "v2 %llu ",
				  mstats->igmp_v2reports[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v3", "v3 %llu\n",
				  mstats->igmp_v3reports[BR_MCAST_DIR_RX]);
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "tx_v1", "TX: v1 %llu ",
				  mstats->igmp_v1reports[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v2", "v2 %llu ",
				  mstats->igmp_v2reports[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v3", "v3 %llu\n",
				  mstats->igmp_v3reports[BR_MCAST_DIR_TX]);
			close_json_object();

			open_json_object("igmp_leaves");
			print_string(PRINT_FP, NULL,
				     "%-16s    IGMP leaves: ", "");
			print_u64(PRINT_ANY, "rx", "RX: %llu ",
				  mstats->igmp_leaves[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "tx", "TX: %llu\n",
				  mstats->igmp_leaves[BR_MCAST_DIR_TX]);
			close_json_object();

			print_string(PRINT_FP, NULL,
				     "%-16s    IGMP parse errors: ", "");
			print_u64(PRINT_ANY, "igmp_parse_errors", "%llu\n",
				  mstats->igmp_parse_errors);

			open_json_object("mld_queries");
			print_string(PRINT_FP, NULL,
				     "%-16s    MLD queries:\n", "");
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "rx_v1", "RX: v1 %llu ",
				  mstats->mld_v1queries[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v2", "v2 %llu\n",
				  mstats->mld_v2queries[BR_MCAST_DIR_RX]);
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "tx_v1", "TX: v1 %llu ",
				  mstats->mld_v1queries[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v2", "v2 %llu\n",
				  mstats->mld_v2queries[BR_MCAST_DIR_TX]);
			close_json_object();

			open_json_object("mld_reports");
			print_string(PRINT_FP, NULL,
				     "%-16s    MLD reports:\n", "");
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "rx_v1", "RX: v1 %llu ",
				  mstats->mld_v1reports[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "rx_v2", "v2 %llu\n",
				  mstats->mld_v2reports[BR_MCAST_DIR_RX]);
			print_string(PRINT_FP, NULL, "%-16s      ", "");
			print_u64(PRINT_ANY, "tx_v1", "TX: v1 %llu ",
				  mstats->mld_v1reports[BR_MCAST_DIR_TX]);
			print_u64(PRINT_ANY, "tx_v2", "v2 %llu\n",
				  mstats->mld_v2reports[BR_MCAST_DIR_TX]);
			close_json_object();

			open_json_object("mld_leaves");
			print_string(PRINT_FP, NULL,
				     "%-16s    MLD leaves: ", "");
			print_u64(PRINT_ANY, "rx", "RX: %llu ",
				  mstats->mld_leaves[BR_MCAST_DIR_RX]);
			print_u64(PRINT_ANY, "tx", "TX: %llu\n",
				  mstats->mld_leaves[BR_MCAST_DIR_TX]);
			close_json_object();

			print_string(PRINT_FP, NULL,
				     "%-16s    MLD parse errors: ", "");
			print_u64(PRINT_ANY, "mld_parse_errors", "%llu\n",
				  mstats->mld_parse_errors);
			close_json_object();
			break;
		case BRIDGE_XSTATS_STP:
			sstats = RTA_DATA(i);
			open_json_object("stp");
			print_string(PRINT_FP, NULL,
				     "%-16s    STP BPDU:  ", "");
			print_u64(PRINT_ANY, "rx_bpdu", "RX: %llu ",
				  sstats->rx_bpdu);
			print_u64(PRINT_ANY, "tx_bpdu", "TX: %llu\n",
				  sstats->tx_bpdu);
			print_string(PRINT_FP, NULL,
				     "%-16s    STP TCN:   ", "");
			print_u64(PRINT_ANY, "rx_tcn", "RX: %llu ",
				  sstats->rx_tcn);
			print_u64(PRINT_ANY, "tx_tcn", "TX: %llu\n",
				  sstats->tx_tcn);
			print_string(PRINT_FP, NULL,
				     "%-16s    STP Transitions: ", "");
			print_u64(PRINT_ANY, "transition_blk", "Blocked: %llu ",
				  sstats->transition_blk);
			print_u64(PRINT_ANY, "transition_fwd", "Forwarding: %llu\n",
				  sstats->transition_fwd);
			close_json_object();
			break;
		}
	}
	close_json_object();
}

int bridge_print_xstats(struct nlmsghdr *n, void *arg)
{
	struct if_stats_msg *ifsm = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_STATS_MAX+1];
	int len = n->nlmsg_len;

	len -= NLMSG_LENGTH(sizeof(*ifsm));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}
	if (filter_index && filter_index != ifsm->ifindex)
		return 0;

	parse_rtattr(tb, IFLA_STATS_MAX, IFLA_STATS_RTA(ifsm), len);
	if (tb[IFLA_STATS_LINK_XSTATS])
		bridge_print_stats_attr(tb[IFLA_STATS_LINK_XSTATS],
					ifsm->ifindex);

	if (tb[IFLA_STATS_LINK_XSTATS_SLAVE])
		bridge_print_stats_attr(tb[IFLA_STATS_LINK_XSTATS_SLAVE],
					ifsm->ifindex);

	return 0;
}

int bridge_parse_xstats(struct link_util *lu, int argc, char **argv)
{
	while (argc > 0) {
		if (strcmp(*argv, "igmp") == 0 || strcmp(*argv, "mcast") == 0) {
			xstats_print_attr = BRIDGE_XSTATS_MCAST;
		} else if (strcmp(*argv, "stp") == 0) {
			xstats_print_attr = BRIDGE_XSTATS_STP;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			filter_index = ll_name_to_index(*argv);
			if (!filter_index)
				return nodev(*argv);
		} else if (strcmp(*argv, "help") == 0) {
			bridge_print_xstats_help(lu, stdout);
			exit(0);
		} else {
			invarg("unknown attribute", *argv);
		}
		argc--; argv++;
	}

	return 0;
}

struct link_util bridge_link_util = {
	.id		= "bridge",
	.maxattr	= IFLA_BR_MAX,
	.parse_opt	= bridge_parse_opt,
	.print_opt	= bridge_print_opt,
	.print_help     = bridge_print_help,
	.parse_ifla_xstats = bridge_parse_xstats,
	.print_ifla_xstats = bridge_print_xstats,
};
