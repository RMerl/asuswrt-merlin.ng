/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <string.h>
#include <stdbool.h>

#include "json_print.h"
#include "libnetlink.h"
#include "utils.h"
#include "br_common.h"

static unsigned int filter_index;

static const char *port_states[] = {
	[BR_STATE_DISABLED] = "disabled",
	[BR_STATE_LISTENING] = "listening",
	[BR_STATE_LEARNING] = "learning",
	[BR_STATE_FORWARDING] = "forwarding",
	[BR_STATE_BLOCKING] = "blocking",
};

static const char *hw_mode[] = {
	"VEB", "VEPA"
};

static void print_link_flags(FILE *fp, unsigned int flags, unsigned int mdown)
{
	open_json_array(PRINT_ANY, is_json_context() ? "flags" : "<");
	if (flags & IFF_UP && !(flags & IFF_RUNNING))
		print_string(PRINT_ANY, NULL,
			     flags ? "%s," : "%s", "NO-CARRIER");
	flags &= ~IFF_RUNNING;

#define _PF(f) if (flags&IFF_##f) {					\
		flags &= ~IFF_##f ;					\
		print_string(PRINT_ANY, NULL, flags ? "%s," : "%s", #f); }
	_PF(LOOPBACK);
	_PF(BROADCAST);
	_PF(POINTOPOINT);
	_PF(MULTICAST);
	_PF(NOARP);
	_PF(ALLMULTI);
	_PF(PROMISC);
	_PF(MASTER);
	_PF(SLAVE);
	_PF(DEBUG);
	_PF(DYNAMIC);
	_PF(AUTOMEDIA);
	_PF(PORTSEL);
	_PF(NOTRAILERS);
	_PF(UP);
	_PF(LOWER_UP);
	_PF(DORMANT);
	_PF(ECHO);
#undef _PF
	if (flags)
		print_hex(PRINT_ANY, NULL, "%x", flags);
	if (mdown)
		print_string(PRINT_ANY, NULL, ",%s", "M-DOWN");
	close_json_array(PRINT_ANY, "> ");
}

static void print_portstate(__u8 state)
{
	if (state <= BR_STATE_BLOCKING)
		print_string(PRINT_ANY, "state",
			     "state %s ", port_states[state]);
	else
		print_uint(PRINT_ANY, "state",
			     "state (%d) ", state);
}

static void print_hwmode(__u16 mode)
{
	if (mode >= ARRAY_SIZE(hw_mode))
		print_0xhex(PRINT_ANY, "hwmode",
			    "hwmode %#llx ", mode);
	else
		print_string(PRINT_ANY, "hwmode",
			     "hwmode %s ", hw_mode[mode]);
}

static void print_protinfo(FILE *fp, struct rtattr *attr)
{
	if (attr->rta_type & NLA_F_NESTED) {
		struct rtattr *prtb[IFLA_BRPORT_MAX + 1];

		parse_rtattr_nested(prtb, IFLA_BRPORT_MAX, attr);

		if (prtb[IFLA_BRPORT_STATE])
			print_portstate(rta_getattr_u8(prtb[IFLA_BRPORT_STATE]));

		if (prtb[IFLA_BRPORT_PRIORITY])
			print_uint(PRINT_ANY, "priority",
				   "priority %u ",
				   rta_getattr_u16(prtb[IFLA_BRPORT_PRIORITY]));

		if (prtb[IFLA_BRPORT_COST])
			print_uint(PRINT_ANY, "cost",
				   "cost %u ",
				   rta_getattr_u32(prtb[IFLA_BRPORT_COST]));

		if (!show_details)
			return;

		if (!is_json_context())
			fprintf(fp, "%s    ", _SL_);

		if (prtb[IFLA_BRPORT_MODE])
			print_on_off(PRINT_ANY, "hairpin", "hairpin %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_MODE]));
		if (prtb[IFLA_BRPORT_GUARD])
			print_on_off(PRINT_ANY, "guard", "guard %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_GUARD]));
		if (prtb[IFLA_BRPORT_PROTECT])
			print_on_off(PRINT_ANY, "root_block", "root_block %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_PROTECT]));
		if (prtb[IFLA_BRPORT_FAST_LEAVE])
			print_on_off(PRINT_ANY, "fastleave", "fastleave %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_FAST_LEAVE]));
		if (prtb[IFLA_BRPORT_LEARNING])
			print_on_off(PRINT_ANY, "learning", "learning %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_LEARNING]));
		if (prtb[IFLA_BRPORT_LEARNING_SYNC])
			print_on_off(PRINT_ANY, "learning_sync", "learning_sync %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_LEARNING_SYNC]));
		if (prtb[IFLA_BRPORT_UNICAST_FLOOD])
			print_on_off(PRINT_ANY, "flood", "flood %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_UNICAST_FLOOD]));
		if (prtb[IFLA_BRPORT_MCAST_FLOOD])
			print_on_off(PRINT_ANY, "mcast_flood", "mcast_flood %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_MCAST_FLOOD]));
		if (prtb[IFLA_BRPORT_MCAST_TO_UCAST])
			print_on_off(PRINT_ANY, "mcast_to_unicast", "mcast_to_unicast %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_MCAST_TO_UCAST]));
		if (prtb[IFLA_BRPORT_NEIGH_SUPPRESS])
			print_on_off(PRINT_ANY, "neigh_suppress", "neigh_suppress %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_NEIGH_SUPPRESS]));
		if (prtb[IFLA_BRPORT_VLAN_TUNNEL])
			print_on_off(PRINT_ANY, "vlan_tunnel", "vlan_tunnel %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_VLAN_TUNNEL]));

		if (prtb[IFLA_BRPORT_BACKUP_PORT]) {
			int ifidx;

			ifidx = rta_getattr_u32(prtb[IFLA_BRPORT_BACKUP_PORT]);
			print_string(PRINT_ANY,
				     "backup_port", "backup_port %s ",
				     ll_index_to_name(ifidx));
		}

		if (prtb[IFLA_BRPORT_ISOLATED])
			print_on_off(PRINT_ANY, "isolated", "isolated %s ",
				     rta_getattr_u8(prtb[IFLA_BRPORT_ISOLATED]));
	} else
		print_portstate(rta_getattr_u8(attr));
}


/*
 * This is reported by HW devices that have some bridging
 * capabilities.
 */
static void print_af_spec(struct rtattr *attr, int ifindex)
{
	struct rtattr *aftb[IFLA_BRIDGE_MAX+1];

	parse_rtattr_nested(aftb, IFLA_BRIDGE_MAX, attr);

	if (aftb[IFLA_BRIDGE_MODE])
		print_hwmode(rta_getattr_u16(aftb[IFLA_BRIDGE_MODE]));

	if (!show_details)
		return;

	if (aftb[IFLA_BRIDGE_VLAN_INFO])
		print_vlan_info(aftb[IFLA_BRIDGE_VLAN_INFO], ifindex);
}

int print_linkinfo(struct nlmsghdr *n, void *arg)
{
	FILE *fp = arg;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX+1];
	unsigned int m_flag = 0;
	int len = n->nlmsg_len;
	const char *name;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "Message too short!\n");
		return -1;
	}

	if (!(ifi->ifi_family == AF_BRIDGE || ifi->ifi_family == AF_UNSPEC))
		return 0;

	if (filter_index && filter_index != ifi->ifi_index)
		return 0;

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);

	name = get_ifname_rta(ifi->ifi_index, tb[IFLA_IFNAME]);
	if (!name)
		return -1;

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELLINK)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	print_int(PRINT_ANY, "ifindex", "%d: ", ifi->ifi_index);
	m_flag = print_name_and_link("%s: ", name, tb);
	print_link_flags(fp, ifi->ifi_flags, m_flag);

	if (tb[IFLA_MTU])
		print_int(PRINT_ANY,
			  "mtu", "mtu %u ",
			  rta_getattr_u32(tb[IFLA_MTU]));

	if (tb[IFLA_MASTER]) {
		int master = rta_getattr_u32(tb[IFLA_MASTER]);

		print_string(PRINT_ANY, "master", "master %s ",
			     ll_index_to_name(master));
	}

	if (tb[IFLA_PROTINFO])
		print_protinfo(fp, tb[IFLA_PROTINFO]);

	if (tb[IFLA_AF_SPEC])
		print_af_spec(tb[IFLA_AF_SPEC], ifi->ifi_index);

	print_string(PRINT_FP, NULL, "%s", "\n");
	close_json_object();
	fflush(fp);
	return 0;
}

static void usage(void)
{
	fprintf(stderr,
		"Usage: bridge link set dev DEV [ cost COST ] [ priority PRIO ] [ state STATE ]\n"
		"                               [ guard {on | off} ]\n"
		"                               [ hairpin {on | off} ]\n"
		"                               [ fastleave {on | off} ]\n"
		"                               [ root_block {on | off} ]\n"
		"                               [ learning {on | off} ]\n"
		"                               [ learning_sync {on | off} ]\n"
		"                               [ flood {on | off} ]\n"
		"                               [ mcast_flood {on | off} ]\n"
		"                               [ mcast_to_unicast {on | off} ]\n"
		"                               [ neigh_suppress {on | off} ]\n"
		"                               [ vlan_tunnel {on | off} ]\n"
		"                               [ isolated {on | off} ]\n"
		"                               [ hwmode {vepa | veb} ]\n"
		"                               [ backup_port DEVICE ] [ nobackup_port ]\n"
		"                               [ self ] [ master ]\n"
		"       bridge link show [dev DEV]\n");
	exit(-1);
}

static int brlink_modify(int argc, char **argv)
{
	struct {
		struct nlmsghdr  n;
		struct ifinfomsg ifm;
		char             buf[512];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_SETLINK,
		.ifm.ifi_family = PF_BRIDGE,
	};
	char *d = NULL;
	int backup_port_idx = -1;
	__s8 neigh_suppress = -1;
	__s8 learning = -1;
	__s8 learning_sync = -1;
	__s8 flood = -1;
	__s8 vlan_tunnel = -1;
	__s8 mcast_flood = -1;
	__s8 mcast_to_unicast = -1;
	__s8 isolated = -1;
	__s8 hairpin = -1;
	__s8 bpdu_guard = -1;
	__s8 fast_leave = -1;
	__s8 root_block = -1;
	__u32 cost = 0;
	__s16 priority = -1;
	__s8 state = -1;
	__s16 mode = -1;
	__u16 flags = 0;
	struct rtattr *nest;
	int ret;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (strcmp(*argv, "guard") == 0) {
			NEXT_ARG();
			bpdu_guard = parse_on_off("guard", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "hairpin") == 0) {
			NEXT_ARG();
			hairpin = parse_on_off("hairpin", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "fastleave") == 0) {
			NEXT_ARG();
			fast_leave = parse_on_off("fastleave", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "root_block") == 0) {
			NEXT_ARG();
			root_block = parse_on_off("root_block", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "learning") == 0) {
			NEXT_ARG();
			learning = parse_on_off("learning", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "learning_sync") == 0) {
			NEXT_ARG();
			learning_sync = parse_on_off("learning_sync", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "flood") == 0) {
			NEXT_ARG();
			flood = parse_on_off("flood", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "mcast_flood") == 0) {
			NEXT_ARG();
			mcast_flood = parse_on_off("mcast_flood", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "mcast_to_unicast") == 0) {
			NEXT_ARG();
			mcast_to_unicast = parse_on_off("mcast_to_unicast", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "cost") == 0) {
			NEXT_ARG();
			cost = atoi(*argv);
		} else if (strcmp(*argv, "priority") == 0) {
			NEXT_ARG();
			priority = atoi(*argv);
		} else if (strcmp(*argv, "state") == 0) {
			NEXT_ARG();
			char *endptr;
			size_t nstates = ARRAY_SIZE(port_states);

			state = strtol(*argv, &endptr, 10);
			if (!(**argv != '\0' && *endptr == '\0')) {
				for (state = 0; state < nstates; state++)
					if (strcasecmp(port_states[state], *argv) == 0)
						break;
				if (state == nstates) {
					fprintf(stderr,
						"Error: invalid STP port state\n");
					return -1;
				}
			}
		} else if (strcmp(*argv, "hwmode") == 0) {
			NEXT_ARG();
			flags = BRIDGE_FLAGS_SELF;
			if (strcmp(*argv, "vepa") == 0)
				mode = BRIDGE_MODE_VEPA;
			else if (strcmp(*argv, "veb") == 0)
				mode = BRIDGE_MODE_VEB;
			else {
				fprintf(stderr,
					"Mode argument must be \"vepa\" or \"veb\".\n");
				return -1;
			}
		} else if (strcmp(*argv, "self") == 0) {
			flags |= BRIDGE_FLAGS_SELF;
		} else if (strcmp(*argv, "master") == 0) {
			flags |= BRIDGE_FLAGS_MASTER;
		} else if (strcmp(*argv, "neigh_suppress") == 0) {
			NEXT_ARG();
			neigh_suppress = parse_on_off("neigh_suppress", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "vlan_tunnel") == 0) {
			NEXT_ARG();
			vlan_tunnel = parse_on_off("vlan_tunnel", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "isolated") == 0) {
			NEXT_ARG();
			isolated = parse_on_off("isolated", *argv, &ret);
			if (ret)
				return ret;
		} else if (strcmp(*argv, "backup_port") == 0) {
			NEXT_ARG();
			backup_port_idx = ll_name_to_index(*argv);
			if (!backup_port_idx) {
				fprintf(stderr, "Error: device %s does not exist\n",
					*argv);
				return -1;
			}
		} else if (strcmp(*argv, "nobackup_port") == 0) {
			backup_port_idx = 0;
		} else {
			usage();
		}
		argc--; argv++;
	}
	if (d == NULL) {
		fprintf(stderr, "Device is a required argument.\n");
		return -1;
	}


	req.ifm.ifi_index = ll_name_to_index(d);
	if (req.ifm.ifi_index == 0) {
		fprintf(stderr, "Cannot find bridge device \"%s\"\n", d);
		return -1;
	}

	/* Nested PROTINFO attribute.  Contains: port flags, cost, priority and
	 * state.
	 */
	nest = addattr_nest(&req.n, sizeof(req),
			    IFLA_PROTINFO | NLA_F_NESTED);
	/* Flags first */
	if (bpdu_guard >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_GUARD, bpdu_guard);
	if (hairpin >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_MODE, hairpin);
	if (fast_leave >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_FAST_LEAVE,
			 fast_leave);
	if (root_block >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_PROTECT, root_block);
	if (flood >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_UNICAST_FLOOD, flood);
	if (mcast_flood >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_MCAST_FLOOD,
			 mcast_flood);
	if (mcast_to_unicast >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_MCAST_TO_UCAST,
			 mcast_to_unicast);
	if (learning >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_LEARNING, learning);
	if (learning_sync >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_LEARNING_SYNC,
			 learning_sync);

	if (cost > 0)
		addattr32(&req.n, sizeof(req), IFLA_BRPORT_COST, cost);

	if (priority >= 0)
		addattr16(&req.n, sizeof(req), IFLA_BRPORT_PRIORITY, priority);

	if (state >= 0)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_STATE, state);

	if (neigh_suppress != -1)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_NEIGH_SUPPRESS,
			 neigh_suppress);
	if (vlan_tunnel != -1)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_VLAN_TUNNEL,
			 vlan_tunnel);
	if (isolated != -1)
		addattr8(&req.n, sizeof(req), IFLA_BRPORT_ISOLATED, isolated);

	if (backup_port_idx != -1)
		addattr32(&req.n, sizeof(req), IFLA_BRPORT_BACKUP_PORT,
			  backup_port_idx);

	addattr_nest_end(&req.n, nest);

	/* IFLA_AF_SPEC nested attribute. Contains IFLA_BRIDGE_FLAGS that
	 * designates master or self operation and IFLA_BRIDGE_MODE
	 * for hw 'vepa' or 'veb' operation modes. The hwmodes are
	 * only valid in 'self' mode on some devices so far.
	 */
	if (mode >= 0 || flags > 0) {
		nest = addattr_nest(&req.n, sizeof(req), IFLA_AF_SPEC);

		if (flags > 0)
			addattr16(&req.n, sizeof(req), IFLA_BRIDGE_FLAGS, flags);

		if (mode >= 0)
			addattr16(&req.n, sizeof(req), IFLA_BRIDGE_MODE, mode);

		addattr_nest_end(&req.n, nest);
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	return 0;
}

static int brlink_show(int argc, char **argv)
{
	char *filter_dev = NULL;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (filter_dev)
				duparg("dev", *argv);
			filter_dev = *argv;
		}
		argc--; argv++;
	}

	if (filter_dev) {
		filter_index = ll_name_to_index(filter_dev);
		if (!filter_index)
			return nodev(filter_dev);
	}

	if (show_details) {
		if (rtnl_linkdump_req_filter(&rth, PF_BRIDGE,
					     (compress_vlans ?
					      RTEXT_FILTER_BRVLAN_COMPRESSED :
					      RTEXT_FILTER_BRVLAN)) < 0) {
			perror("Cannon send dump request");
			exit(1);
		}
	} else {
		if (rtnl_linkdump_req(&rth, PF_BRIDGE) < 0) {
			perror("Cannon send dump request");
			exit(1);
		}
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_linkinfo, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	delete_json_obj();
	fflush(stdout);
	return 0;
}

int do_link(int argc, char **argv)
{
	ll_init_map(&rth);
	if (argc > 0) {
		if (matches(*argv, "set") == 0 ||
		    matches(*argv, "change") == 0)
			return brlink_modify(argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return brlink_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return brlink_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"bridge link help\".\n", *argv);
	exit(-1);
}
