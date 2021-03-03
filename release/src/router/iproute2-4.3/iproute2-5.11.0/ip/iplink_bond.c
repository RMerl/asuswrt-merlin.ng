/*
 * iplink_bond.c	Bonding device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 *              Scott Feldman <sfeldma@cumulusnetworks.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_bonding.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

#define BOND_MAX_ARP_TARGETS    16

static unsigned int xstats_print_attr;
static int filter_index;

static const char *mode_tbl[] = {
	"balance-rr",
	"active-backup",
	"balance-xor",
	"broadcast",
	"802.3ad",
	"balance-tlb",
	"balance-alb",
	NULL,
};

static const char *arp_validate_tbl[] = {
	"none",
	"active",
	"backup",
	"all",
	NULL,
};

static const char *arp_all_targets_tbl[] = {
	"any",
	"all",
	NULL,
};

static const char *primary_reselect_tbl[] = {
	"always",
	"better",
	"failure",
	NULL,
};

static const char *fail_over_mac_tbl[] = {
	"none",
	"active",
	"follow",
	NULL,
};

static const char *xmit_hash_policy_tbl[] = {
	"layer2",
	"layer3+4",
	"layer2+3",
	"encap2+3",
	"encap3+4",
	"vlan+srcmac",
	NULL,
};

static const char *lacp_rate_tbl[] = {
	"slow",
	"fast",
	NULL,
};

static const char *ad_select_tbl[] = {
	"stable",
	"bandwidth",
	"count",
	NULL,
};

static const char *get_name(const char **tbl, int index)
{
	int i;

	for (i = 0; tbl[i]; i++)
		if (i == index)
			return tbl[i];

	return "UNKNOWN";
}

static int get_index(const char **tbl, char *name)
{
	int i, index;

	/* check for integer index passed in instead of name */
	if (get_integer(&index, name, 10) == 0)
		for (i = 0; tbl[i]; i++)
			if (i == index)
				return i;

	for (i = 0; tbl[i]; i++)
		if (strcmp(tbl[i], name) == 0)
			return i;

	return -1;
}

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... bond [ mode BONDMODE ] [ active_slave SLAVE_DEV ]\n"
		"                [ clear_active_slave ] [ miimon MIIMON ]\n"
		"                [ updelay UPDELAY ] [ downdelay DOWNDELAY ]\n"
		"                [ peer_notify_delay DELAY ]\n"
		"                [ use_carrier USE_CARRIER ]\n"
		"                [ arp_interval ARP_INTERVAL ]\n"
		"                [ arp_validate ARP_VALIDATE ]\n"
		"                [ arp_all_targets ARP_ALL_TARGETS ]\n"
		"                [ arp_ip_target [ ARP_IP_TARGET, ... ] ]\n"
		"                [ primary SLAVE_DEV ]\n"
		"                [ primary_reselect PRIMARY_RESELECT ]\n"
		"                [ fail_over_mac FAIL_OVER_MAC ]\n"
		"                [ xmit_hash_policy XMIT_HASH_POLICY ]\n"
		"                [ resend_igmp RESEND_IGMP ]\n"
		"                [ num_grat_arp|num_unsol_na NUM_GRAT_ARP|NUM_UNSOL_NA ]\n"
		"                [ all_slaves_active ALL_SLAVES_ACTIVE ]\n"
		"                [ min_links MIN_LINKS ]\n"
		"                [ lp_interval LP_INTERVAL ]\n"
		"                [ packets_per_slave PACKETS_PER_SLAVE ]\n"
		"                [ tlb_dynamic_lb TLB_DYNAMIC_LB ]\n"
		"                [ lacp_rate LACP_RATE ]\n"
		"                [ ad_select AD_SELECT ]\n"
		"                [ ad_user_port_key PORTKEY ]\n"
		"                [ ad_actor_sys_prio SYSPRIO ]\n"
		"                [ ad_actor_system LLADDR ]\n"
		"\n"
		"BONDMODE := balance-rr|active-backup|balance-xor|broadcast|802.3ad|balance-tlb|balance-alb\n"
		"ARP_VALIDATE := none|active|backup|all\n"
		"ARP_ALL_TARGETS := any|all\n"
		"PRIMARY_RESELECT := always|better|failure\n"
		"FAIL_OVER_MAC := none|active|follow\n"
		"XMIT_HASH_POLICY := layer2|layer2+3|layer3+4|encap2+3|encap3+4|vlan+srcmac\n"
		"LACP_RATE := slow|fast\n"
		"AD_SELECT := stable|bandwidth|count\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

static int bond_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	__u8 mode, use_carrier, primary_reselect, fail_over_mac;
	__u8 xmit_hash_policy, num_peer_notif, all_slaves_active;
	__u8 lacp_rate, ad_select, tlb_dynamic_lb;
	__u16 ad_user_port_key, ad_actor_sys_prio;
	__u32 miimon, updelay, downdelay, peer_notify_delay, arp_interval, arp_validate;
	__u32 arp_all_targets, resend_igmp, min_links, lp_interval;
	__u32 packets_per_slave;
	unsigned int ifindex;

	while (argc > 0) {
		if (matches(*argv, "mode") == 0) {
			NEXT_ARG();
			if (get_index(mode_tbl, *argv) < 0)
				invarg("invalid mode", *argv);
			mode = get_index(mode_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_MODE, mode);
		} else if (matches(*argv, "active_slave") == 0) {
			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				return nodev(*argv);
			addattr32(n, 1024, IFLA_BOND_ACTIVE_SLAVE, ifindex);
		} else if (matches(*argv, "clear_active_slave") == 0) {
			addattr32(n, 1024, IFLA_BOND_ACTIVE_SLAVE, 0);
		} else if (matches(*argv, "miimon") == 0) {
			NEXT_ARG();
			if (get_u32(&miimon, *argv, 0))
				invarg("invalid miimon", *argv);
			addattr32(n, 1024, IFLA_BOND_MIIMON, miimon);
		} else if (matches(*argv, "updelay") == 0) {
			NEXT_ARG();
			if (get_u32(&updelay, *argv, 0))
				invarg("invalid updelay", *argv);
			addattr32(n, 1024, IFLA_BOND_UPDELAY, updelay);
		} else if (matches(*argv, "downdelay") == 0) {
			NEXT_ARG();
			if (get_u32(&downdelay, *argv, 0))
				invarg("invalid downdelay", *argv);
			addattr32(n, 1024, IFLA_BOND_DOWNDELAY, downdelay);
		} else if (matches(*argv, "peer_notify_delay") == 0) {
			NEXT_ARG();
			if (get_u32(&peer_notify_delay, *argv, 0))
				invarg("invalid peer_notify_delay", *argv);
			addattr32(n, 1024, IFLA_BOND_PEER_NOTIF_DELAY, peer_notify_delay);
		} else if (matches(*argv, "use_carrier") == 0) {
			NEXT_ARG();
			if (get_u8(&use_carrier, *argv, 0))
				invarg("invalid use_carrier", *argv);
			addattr8(n, 1024, IFLA_BOND_USE_CARRIER, use_carrier);
		} else if (matches(*argv, "arp_interval") == 0) {
			NEXT_ARG();
			if (get_u32(&arp_interval, *argv, 0))
				invarg("invalid arp_interval", *argv);
			addattr32(n, 1024, IFLA_BOND_ARP_INTERVAL, arp_interval);
		} else if (matches(*argv, "arp_ip_target") == 0) {
			struct rtattr *nest = addattr_nest(n, 1024,
				IFLA_BOND_ARP_IP_TARGET);
			if (NEXT_ARG_OK()) {
				NEXT_ARG();
				char *targets = strdupa(*argv);
				char *target = strtok(targets, ",");
				int i;

				for (i = 0; target && i < BOND_MAX_ARP_TARGETS; i++) {
					__u32 addr = get_addr32(target);

					addattr32(n, 1024, i, addr);
					target = strtok(NULL, ",");
				}
				addattr_nest_end(n, nest);
			}
			addattr_nest_end(n, nest);
		} else if (matches(*argv, "arp_validate") == 0) {
			NEXT_ARG();
			if (get_index(arp_validate_tbl, *argv) < 0)
				invarg("invalid arp_validate", *argv);
			arp_validate = get_index(arp_validate_tbl, *argv);
			addattr32(n, 1024, IFLA_BOND_ARP_VALIDATE, arp_validate);
		} else if (matches(*argv, "arp_all_targets") == 0) {
			NEXT_ARG();
			if (get_index(arp_all_targets_tbl, *argv) < 0)
				invarg("invalid arp_all_targets", *argv);
			arp_all_targets = get_index(arp_all_targets_tbl, *argv);
			addattr32(n, 1024, IFLA_BOND_ARP_ALL_TARGETS, arp_all_targets);
		} else if (matches(*argv, "primary") == 0) {
			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				return nodev(*argv);
			addattr32(n, 1024, IFLA_BOND_PRIMARY, ifindex);
		} else if (matches(*argv, "primary_reselect") == 0) {
			NEXT_ARG();
			if (get_index(primary_reselect_tbl, *argv) < 0)
				invarg("invalid primary_reselect", *argv);
			primary_reselect = get_index(primary_reselect_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_PRIMARY_RESELECT,
				 primary_reselect);
		} else if (matches(*argv, "fail_over_mac") == 0) {
			NEXT_ARG();
			if (get_index(fail_over_mac_tbl, *argv) < 0)
				invarg("invalid fail_over_mac", *argv);
			fail_over_mac = get_index(fail_over_mac_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_FAIL_OVER_MAC,
				 fail_over_mac);
		} else if (matches(*argv, "xmit_hash_policy") == 0) {
			NEXT_ARG();
			if (get_index(xmit_hash_policy_tbl, *argv) < 0)
				invarg("invalid xmit_hash_policy", *argv);

			xmit_hash_policy = get_index(xmit_hash_policy_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_XMIT_HASH_POLICY,
				 xmit_hash_policy);
		} else if (matches(*argv, "resend_igmp") == 0) {
			NEXT_ARG();
			if (get_u32(&resend_igmp, *argv, 0))
				invarg("invalid resend_igmp", *argv);

			addattr32(n, 1024, IFLA_BOND_RESEND_IGMP, resend_igmp);
		} else if (matches(*argv, "num_grat_arp") == 0 ||
			   matches(*argv, "num_unsol_na") == 0) {
			NEXT_ARG();
			if (get_u8(&num_peer_notif, *argv, 0))
				invarg("invalid num_grat_arp|num_unsol_na",
				       *argv);

			addattr8(n, 1024, IFLA_BOND_NUM_PEER_NOTIF,
				 num_peer_notif);
		} else if (matches(*argv, "all_slaves_active") == 0) {
			NEXT_ARG();
			if (get_u8(&all_slaves_active, *argv, 0))
				invarg("invalid all_slaves_active", *argv);

			addattr8(n, 1024, IFLA_BOND_ALL_SLAVES_ACTIVE,
				 all_slaves_active);
		} else if (matches(*argv, "min_links") == 0) {
			NEXT_ARG();
			if (get_u32(&min_links, *argv, 0))
				invarg("invalid min_links", *argv);

			addattr32(n, 1024, IFLA_BOND_MIN_LINKS, min_links);
		} else if (matches(*argv, "lp_interval") == 0) {
			NEXT_ARG();
			if (get_u32(&lp_interval, *argv, 0))
				invarg("invalid lp_interval", *argv);

			addattr32(n, 1024, IFLA_BOND_LP_INTERVAL, lp_interval);
		} else if (matches(*argv, "packets_per_slave") == 0) {
			NEXT_ARG();
			if (get_u32(&packets_per_slave, *argv, 0))
				invarg("invalid packets_per_slave", *argv);

			addattr32(n, 1024, IFLA_BOND_PACKETS_PER_SLAVE,
				  packets_per_slave);
		} else if (matches(*argv, "lacp_rate") == 0) {
			NEXT_ARG();
			if (get_index(lacp_rate_tbl, *argv) < 0)
				invarg("invalid lacp_rate", *argv);

			lacp_rate = get_index(lacp_rate_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_AD_LACP_RATE, lacp_rate);
		} else if (matches(*argv, "ad_select") == 0) {
			NEXT_ARG();
			if (get_index(ad_select_tbl, *argv) < 0)
				invarg("invalid ad_select", *argv);

			ad_select = get_index(ad_select_tbl, *argv);
			addattr8(n, 1024, IFLA_BOND_AD_SELECT, ad_select);
		} else if (matches(*argv, "ad_user_port_key") == 0) {
			NEXT_ARG();
			if (get_u16(&ad_user_port_key, *argv, 0))
				invarg("invalid ad_user_port_key", *argv);

			addattr16(n, 1024, IFLA_BOND_AD_USER_PORT_KEY,
				  ad_user_port_key);
		} else if (matches(*argv, "ad_actor_sys_prio") == 0) {
			NEXT_ARG();
			if (get_u16(&ad_actor_sys_prio, *argv, 0))
				invarg("invalid ad_actor_sys_prio", *argv);

			addattr16(n, 1024, IFLA_BOND_AD_ACTOR_SYS_PRIO,
				  ad_actor_sys_prio);
		} else if (matches(*argv, "ad_actor_system") == 0) {
			int len;
			char abuf[32];

			NEXT_ARG();
			len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			if (len < 0)
				return -1;
			addattr_l(n, 1024, IFLA_BOND_AD_ACTOR_SYSTEM,
				  abuf, len);
		} else if (matches(*argv, "tlb_dynamic_lb") == 0) {
			NEXT_ARG();
			if (get_u8(&tlb_dynamic_lb, *argv, 0)) {
				invarg("invalid tlb_dynamic_lb", *argv);
				return -1;
			}
			addattr8(n, 1024, IFLA_BOND_TLB_DYNAMIC_LB,
				 tlb_dynamic_lb);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "bond: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void bond_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_BOND_MODE]) {
		const char *mode = get_name(mode_tbl,
					    rta_getattr_u8(tb[IFLA_BOND_MODE]));
		print_string(PRINT_ANY, "mode", "mode %s ", mode);
	}

	if (tb[IFLA_BOND_ACTIVE_SLAVE]) {
		unsigned int ifindex =
			rta_getattr_u32(tb[IFLA_BOND_ACTIVE_SLAVE]);

		if (ifindex) {
			print_string(PRINT_ANY,
				     "active_slave",
				     "active_slave %s ",
				     ll_index_to_name(ifindex));
		}
	}

	if (tb[IFLA_BOND_MIIMON])
		print_uint(PRINT_ANY,
			   "miimon",
			   "miimon %u ",
			   rta_getattr_u32(tb[IFLA_BOND_MIIMON]));

	if (tb[IFLA_BOND_UPDELAY])
		print_uint(PRINT_ANY,
			   "updelay",
			   "updelay %u ",
			   rta_getattr_u32(tb[IFLA_BOND_UPDELAY]));

	if (tb[IFLA_BOND_DOWNDELAY])
		print_uint(PRINT_ANY,
			   "downdelay",
			   "downdelay %u ",
			   rta_getattr_u32(tb[IFLA_BOND_DOWNDELAY]));

	if (tb[IFLA_BOND_PEER_NOTIF_DELAY])
		print_uint(PRINT_ANY,
			   "peer_notify_delay",
			   "peer_notify_delay %u ",
			   rta_getattr_u32(tb[IFLA_BOND_PEER_NOTIF_DELAY]));

	if (tb[IFLA_BOND_USE_CARRIER])
		print_uint(PRINT_ANY,
			   "use_carrier",
			   "use_carrier %u ",
			   rta_getattr_u8(tb[IFLA_BOND_USE_CARRIER]));

	if (tb[IFLA_BOND_ARP_INTERVAL])
		print_uint(PRINT_ANY,
			   "arp_interval",
			   "arp_interval %u ",
			   rta_getattr_u32(tb[IFLA_BOND_ARP_INTERVAL]));

	if (tb[IFLA_BOND_ARP_IP_TARGET]) {
		struct rtattr *iptb[BOND_MAX_ARP_TARGETS + 1];
		int i;

		parse_rtattr_nested(iptb, BOND_MAX_ARP_TARGETS,
				    tb[IFLA_BOND_ARP_IP_TARGET]);

		if (iptb[0]) {
			open_json_array(PRINT_JSON, "arp_ip_target");
			print_string(PRINT_FP, NULL, "arp_ip_target ", NULL);
		}

		for (i = 0; i < BOND_MAX_ARP_TARGETS; i++) {
			if (iptb[i])
				print_string(PRINT_ANY,
					     NULL,
					     "%s",
					     rt_addr_n2a_rta(AF_INET, iptb[i]));
			if (!is_json_context()
			    && i < BOND_MAX_ARP_TARGETS-1
			    && iptb[i+1])
				fprintf(f, ",");
		}

		if (iptb[0]) {
			print_string(PRINT_FP, NULL, " ", NULL);
			close_json_array(PRINT_JSON, NULL);
		}
	}

	if (tb[IFLA_BOND_ARP_VALIDATE]) {
		__u32 arp_v = rta_getattr_u32(tb[IFLA_BOND_ARP_VALIDATE]);
		const char *arp_validate = get_name(arp_validate_tbl, arp_v);

		if (!arp_v && is_json_context())
			print_null(PRINT_JSON, "arp_validate", NULL, NULL);
		else
			print_string(PRINT_ANY,
				     "arp_validate",
				     "arp_validate %s ",
				     arp_validate);
	}

	if (tb[IFLA_BOND_ARP_ALL_TARGETS]) {
		const char *arp_all_targets = get_name(arp_all_targets_tbl,
						       rta_getattr_u32(tb[IFLA_BOND_ARP_ALL_TARGETS]));
		print_string(PRINT_ANY,
			     "arp_all_targets",
			     "arp_all_targets %s ",
			     arp_all_targets);
	}

	if (tb[IFLA_BOND_PRIMARY]) {
		unsigned int ifindex = rta_getattr_u32(tb[IFLA_BOND_PRIMARY]);

		if (ifindex) {
			print_string(PRINT_ANY,
				     "primary",
				     "primary %s ",
				     ll_index_to_name(ifindex));
		}
	}

	if (tb[IFLA_BOND_PRIMARY_RESELECT]) {
		const char *primary_reselect = get_name(primary_reselect_tbl,
							rta_getattr_u8(tb[IFLA_BOND_PRIMARY_RESELECT]));
		print_string(PRINT_ANY,
			     "primary_reselect",
			     "primary_reselect %s ",
			     primary_reselect);
	}

	if (tb[IFLA_BOND_FAIL_OVER_MAC]) {
		const char *fail_over_mac = get_name(fail_over_mac_tbl,
						     rta_getattr_u8(tb[IFLA_BOND_FAIL_OVER_MAC]));
		print_string(PRINT_ANY,
			     "fail_over_mac",
			     "fail_over_mac %s ",
			     fail_over_mac);
	}

	if (tb[IFLA_BOND_XMIT_HASH_POLICY]) {
		const char *xmit_hash_policy = get_name(xmit_hash_policy_tbl,
							rta_getattr_u8(tb[IFLA_BOND_XMIT_HASH_POLICY]));
		print_string(PRINT_ANY,
			     "xmit_hash_policy",
			     "xmit_hash_policy %s ",
			     xmit_hash_policy);
	}

	if (tb[IFLA_BOND_RESEND_IGMP])
		print_uint(PRINT_ANY,
			   "resend_igmp",
			   "resend_igmp %u ",
			   rta_getattr_u32(tb[IFLA_BOND_RESEND_IGMP]));

	if (tb[IFLA_BOND_NUM_PEER_NOTIF])
		print_uint(PRINT_ANY,
			   "num_peer_notif",
			   "num_grat_arp %u ",
			   rta_getattr_u8(tb[IFLA_BOND_NUM_PEER_NOTIF]));

	if (tb[IFLA_BOND_ALL_SLAVES_ACTIVE])
		print_uint(PRINT_ANY,
			   "all_slaves_active",
			   "all_slaves_active %u ",
			   rta_getattr_u8(tb[IFLA_BOND_ALL_SLAVES_ACTIVE]));

	if (tb[IFLA_BOND_MIN_LINKS])
		print_uint(PRINT_ANY,
			   "min_links",
			   "min_links %u ",
			   rta_getattr_u32(tb[IFLA_BOND_MIN_LINKS]));

	if (tb[IFLA_BOND_LP_INTERVAL])
		print_uint(PRINT_ANY,
			   "lp_interval",
			   "lp_interval %u ",
			   rta_getattr_u32(tb[IFLA_BOND_LP_INTERVAL]));

	if (tb[IFLA_BOND_PACKETS_PER_SLAVE])
		print_uint(PRINT_ANY,
			   "packets_per_slave",
			   "packets_per_slave %u ",
			   rta_getattr_u32(tb[IFLA_BOND_PACKETS_PER_SLAVE]));

	if (tb[IFLA_BOND_AD_LACP_RATE]) {
		const char *lacp_rate = get_name(lacp_rate_tbl,
						 rta_getattr_u8(tb[IFLA_BOND_AD_LACP_RATE]));
		print_string(PRINT_ANY,
			     "ad_lacp_rate",
			     "lacp_rate %s ",
			     lacp_rate);
	}

	if (tb[IFLA_BOND_AD_SELECT]) {
		const char *ad_select = get_name(ad_select_tbl,
						 rta_getattr_u8(tb[IFLA_BOND_AD_SELECT]));
		print_string(PRINT_ANY,
			     "ad_select",
			     "ad_select %s ",
			     ad_select);
	}

	if (tb[IFLA_BOND_AD_INFO]) {
		struct rtattr *adtb[IFLA_BOND_AD_INFO_MAX + 1];

		parse_rtattr_nested(adtb, IFLA_BOND_AD_INFO_MAX,
				    tb[IFLA_BOND_AD_INFO]);

		open_json_object("ad_info");

		if (adtb[IFLA_BOND_AD_INFO_AGGREGATOR])
			print_int(PRINT_ANY,
				  "aggregator",
				  "ad_aggregator %d ",
				  rta_getattr_u16(adtb[IFLA_BOND_AD_INFO_AGGREGATOR]));

		if (adtb[IFLA_BOND_AD_INFO_NUM_PORTS])
			print_int(PRINT_ANY,
				  "num_ports",
				  "ad_num_ports %d ",
				  rta_getattr_u16(adtb[IFLA_BOND_AD_INFO_NUM_PORTS]));

		if (adtb[IFLA_BOND_AD_INFO_ACTOR_KEY])
			print_int(PRINT_ANY,
				  "actor_key",
				  "ad_actor_key %d ",
				  rta_getattr_u16(adtb[IFLA_BOND_AD_INFO_ACTOR_KEY]));

		if (adtb[IFLA_BOND_AD_INFO_PARTNER_KEY])
			print_int(PRINT_ANY,
				  "partner_key",
				  "ad_partner_key %d ",
				  rta_getattr_u16(adtb[IFLA_BOND_AD_INFO_PARTNER_KEY]));

		if (adtb[IFLA_BOND_AD_INFO_PARTNER_MAC]) {
			unsigned char *p =
				RTA_DATA(adtb[IFLA_BOND_AD_INFO_PARTNER_MAC]);
			SPRINT_BUF(b);
			print_string(PRINT_ANY,
				     "partner_mac",
				     "ad_partner_mac %s ",
				     ll_addr_n2a(p, ETH_ALEN, 0, b, sizeof(b)));
		}

		close_json_object();
	}

	if (tb[IFLA_BOND_AD_ACTOR_SYS_PRIO]) {
		print_uint(PRINT_ANY,
			   "ad_actor_sys_prio",
			   "ad_actor_sys_prio %u ",
			   rta_getattr_u16(tb[IFLA_BOND_AD_ACTOR_SYS_PRIO]));
	}

	if (tb[IFLA_BOND_AD_USER_PORT_KEY]) {
		print_uint(PRINT_ANY,
			   "ad_user_port_key",
			   "ad_user_port_key %u ",
			   rta_getattr_u16(tb[IFLA_BOND_AD_USER_PORT_KEY]));
	}

	if (tb[IFLA_BOND_AD_ACTOR_SYSTEM]) {
		/* We assume the l2 address is an Ethernet MAC address */
		SPRINT_BUF(b1);

		print_string(PRINT_ANY,
			     "ad_actor_system",
			     "ad_actor_system %s ",
			     ll_addr_n2a(RTA_DATA(tb[IFLA_BOND_AD_ACTOR_SYSTEM]),
					 RTA_PAYLOAD(tb[IFLA_BOND_AD_ACTOR_SYSTEM]),
					 1 /*ARPHDR_ETHER*/, b1, sizeof(b1)));
	}

	if (tb[IFLA_BOND_TLB_DYNAMIC_LB]) {
		print_uint(PRINT_ANY,
			   "tlb_dynamic_lb",
			   "tlb_dynamic_lb %u ",
			   rta_getattr_u8(tb[IFLA_BOND_TLB_DYNAMIC_LB]));
	}
}

static void bond_print_help(struct link_util *lu, int argc, char **argv,
			    FILE *f)
{
	print_explain(f);
}

static void bond_print_xstats_help(struct link_util *lu, FILE *f)
{
	fprintf(f, "Usage: ... %s [ 802.3ad ] [ dev DEVICE ]\n", lu->id);
}

static void bond_print_3ad_stats(struct rtattr *lacpattr)
{
	struct rtattr *lacptb[BOND_3AD_STAT_MAX+1];
	__u64 val;

	parse_rtattr(lacptb, BOND_3AD_STAT_MAX, RTA_DATA(lacpattr),
		     RTA_PAYLOAD(lacpattr));
	open_json_object("802.3ad");
	if (lacptb[BOND_3AD_STAT_LACPDU_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		print_u64(PRINT_ANY, "lacpdu_rx", "LACPDU Rx %llu\n",
			  rta_getattr_u64(lacptb[BOND_3AD_STAT_LACPDU_RX]));
	}
	if (lacptb[BOND_3AD_STAT_LACPDU_TX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		print_u64(PRINT_ANY, "lacpdu_tx", "LACPDU Tx %llu\n",
			  rta_getattr_u64(lacptb[BOND_3AD_STAT_LACPDU_TX]));
	}
	if (lacptb[BOND_3AD_STAT_LACPDU_UNKNOWN_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		val = rta_getattr_u64(lacptb[BOND_3AD_STAT_LACPDU_UNKNOWN_RX]);
		print_u64(PRINT_ANY,
			  "lacpdu_unknown_rx",
			  "LACPDU Unknown type Rx %llu\n",
			  val);
	}
	if (lacptb[BOND_3AD_STAT_LACPDU_ILLEGAL_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		val = rta_getattr_u64(lacptb[BOND_3AD_STAT_LACPDU_ILLEGAL_RX]);
		print_u64(PRINT_ANY,
			  "lacpdu_illegal_rx",
			  "LACPDU Illegal Rx %llu\n",
			  val);
	}
	if (lacptb[BOND_3AD_STAT_MARKER_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		print_u64(PRINT_ANY, "marker_rx", "Marker Rx %llu\n",
			  rta_getattr_u64(lacptb[BOND_3AD_STAT_MARKER_RX]));
	}
	if (lacptb[BOND_3AD_STAT_MARKER_TX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		print_u64(PRINT_ANY, "marker_tx", "Marker Tx %llu\n",
			  rta_getattr_u64(lacptb[BOND_3AD_STAT_MARKER_TX]));
	}
	if (lacptb[BOND_3AD_STAT_MARKER_RESP_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		val = rta_getattr_u64(lacptb[BOND_3AD_STAT_MARKER_RESP_RX]);
		print_u64(PRINT_ANY,
			  "marker_response_rx",
			  "Marker response Rx %llu\n",
			  val);
	}
	if (lacptb[BOND_3AD_STAT_MARKER_RESP_TX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		val = rta_getattr_u64(lacptb[BOND_3AD_STAT_MARKER_RESP_TX]);
		print_u64(PRINT_ANY,
			  "marker_response_tx",
			  "Marker response Tx %llu\n",
			  val);
	}
	if (lacptb[BOND_3AD_STAT_MARKER_UNKNOWN_RX]) {
		print_string(PRINT_FP, NULL, "%-16s    ", "");
		val = rta_getattr_u64(lacptb[BOND_3AD_STAT_MARKER_UNKNOWN_RX]);
		print_u64(PRINT_ANY,
			  "marker_unknown_rx",
			  "Marker unknown type Rx %llu\n",
			  val);
	}
	close_json_object();
}

static void bond_print_stats_attr(struct rtattr *attr, int ifindex)
{
	struct rtattr *bondtb[LINK_XSTATS_TYPE_MAX+1];
	struct rtattr *i, *list;
	const char *ifname = "";
	int rem;

	parse_rtattr(bondtb, LINK_XSTATS_TYPE_MAX+1, RTA_DATA(attr),
	RTA_PAYLOAD(attr));
	if (!bondtb[LINK_XSTATS_TYPE_BOND])
		return;

	list = bondtb[LINK_XSTATS_TYPE_BOND];
	rem = RTA_PAYLOAD(list);
	open_json_object(NULL);
	ifname = ll_index_to_name(ifindex);
	print_string(PRINT_ANY, "ifname", "%-16s\n", ifname);
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (xstats_print_attr && i->rta_type != xstats_print_attr)
			continue;

		switch (i->rta_type) {
		case BOND_XSTATS_3AD:
			bond_print_3ad_stats(i);
			break;
		}
		break;
	}
	close_json_object();
}

int bond_print_xstats(struct nlmsghdr *n, void *arg)
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
		bond_print_stats_attr(tb[IFLA_STATS_LINK_XSTATS],
				      ifsm->ifindex);

	if (tb[IFLA_STATS_LINK_XSTATS_SLAVE])
		bond_print_stats_attr(tb[IFLA_STATS_LINK_XSTATS_SLAVE],
				      ifsm->ifindex);

	return 0;
}

int bond_parse_xstats(struct link_util *lu, int argc, char **argv)
{
	while (argc > 0) {
		if (strcmp(*argv, "lacp") == 0 ||
		    strcmp(*argv, "802.3ad") == 0) {
			xstats_print_attr = BOND_XSTATS_3AD;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			filter_index = ll_name_to_index(*argv);
			if (!filter_index)
				return nodev(*argv);
		} else if (strcmp(*argv, "help") == 0) {
			bond_print_xstats_help(lu, stdout);
			exit(0);
		} else {
			invarg("unknown attribute", *argv);
		}
		argc--; argv++;
	}

	return 0;
}


struct link_util bond_link_util = {
	.id		= "bond",
	.maxattr	= IFLA_BOND_MAX,
	.parse_opt	= bond_parse_opt,
	.print_opt	= bond_print_opt,
	.print_help	= bond_print_help,
	.parse_ifla_xstats = bond_parse_xstats,
	.print_ifla_xstats = bond_print_xstats,
};
