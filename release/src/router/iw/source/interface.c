#include <net/if.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

#define VALID_FLAGS	"none:     no special flags\n"\
			"fcsfail:  show frames with FCS errors\n"\
			"control:  show control frames\n"\
			"otherbss: show frames from other BSSes\n"\
			"cook:     use cooked mode"

SECTION(interface);

static char *mntr_flags[NL80211_MNTR_FLAG_MAX + 1] = {
	"none",
	"fcsfail",
	"plcpfail",
	"control",
	"otherbss",
	"cook",
};

static int parse_mntr_flags(int *_argc, char ***_argv,
			    struct nl_msg *msg)
{
	struct nl_msg *flags;
	int err = -ENOBUFS;
	enum nl80211_mntr_flags flag;
	int argc = *_argc;
	char **argv = *_argv;

	flags = nlmsg_alloc();
	if (!flags)
		return -ENOMEM;

	while (argc) {
		int ok = 0;
		for (flag = __NL80211_MNTR_FLAG_INVALID;
		     flag <= NL80211_MNTR_FLAG_MAX; flag++) {
			if (strcmp(*argv, mntr_flags[flag]) == 0) {
				ok = 1;
				/*
				 * This shouldn't be adding "flag" if that is
				 * zero, but due to a problem in the kernel's
				 * nl80211 code (using NLA_NESTED policy) it
				 * will reject an empty nested attribute but
				 * not one that contains an invalid attribute
				 */
				NLA_PUT_FLAG(flags, flag);
				break;
			}
		}
		if (!ok) {
			err = -EINVAL;
			goto out;
		}
		argc--;
		argv++;
	}

	nla_put_nested(msg, NL80211_ATTR_MNTR_FLAGS, flags);
	err = 0;
 nla_put_failure:
 out:
	nlmsg_free(flags);

	*_argc = argc;
	*_argv = argv;

	return err;
}

/* for help */
#define IFACE_TYPES "Valid interface types are: managed, ibss, monitor, mesh, wds."

/* return 0 if ok, internal error otherwise */
static int get_if_type(int *argc, char ***argv, enum nl80211_iftype *type,
		       bool need_type)
{
	char *tpstr;

	if (*argc < 1 + !!need_type)
		return 1;

	if (need_type && strcmp((*argv)[0], "type"))
		return 1;

	tpstr = (*argv)[!!need_type];
	*argc -= 1 + !!need_type;
	*argv += 1 + !!need_type;

	if (strcmp(tpstr, "adhoc") == 0 ||
	    strcmp(tpstr, "ibss") == 0) {
		*type = NL80211_IFTYPE_ADHOC;
		return 0;
	} else if (strcmp(tpstr, "monitor") == 0) {
		*type = NL80211_IFTYPE_MONITOR;
		return 0;
	} else if (strcmp(tpstr, "master") == 0 ||
		   strcmp(tpstr, "ap") == 0) {
		*type = NL80211_IFTYPE_UNSPECIFIED;
		fprintf(stderr, "You need to run a management daemon, e.g. hostapd,\n");
		fprintf(stderr, "see http://wireless.kernel.org/en/users/Documentation/hostapd\n");
		fprintf(stderr, "for more information on how to do that.\n");
		return 2;
	} else if (strcmp(tpstr, "__ap") == 0) {
		*type = NL80211_IFTYPE_AP;
		return 0;
	} else if (strcmp(tpstr, "__ap_vlan") == 0) {
		*type = NL80211_IFTYPE_AP_VLAN;
		return 0;
	} else if (strcmp(tpstr, "wds") == 0) {
		*type = NL80211_IFTYPE_WDS;
		return 0;
	} else if (strcmp(tpstr, "managed") == 0 ||
		   strcmp(tpstr, "mgd") == 0 ||
		   strcmp(tpstr, "station") == 0) {
		*type = NL80211_IFTYPE_STATION;
		return 0;
	} else if (strcmp(tpstr, "mp") == 0 ||
		   strcmp(tpstr, "mesh") == 0) {
		*type = NL80211_IFTYPE_MESH_POINT;
		return 0;
	} else if (strcmp(tpstr, "__p2pcl") == 0) {
		*type = NL80211_IFTYPE_P2P_CLIENT;
		return 0;
	} else if (strcmp(tpstr, "__p2pgo") == 0) {
		*type = NL80211_IFTYPE_P2P_GO;
		return 0;
	}

	fprintf(stderr, "invalid interface type %s\n", tpstr);
	return 2;
}

static int parse_4addr_flag(const char *value, struct nl_msg *msg)
{
	if (strcmp(value, "on") == 0)
		NLA_PUT_U8(msg, NL80211_ATTR_4ADDR, 1);
	else if (strcmp(value, "off") == 0)
		NLA_PUT_U8(msg, NL80211_ATTR_4ADDR, 0);
	else
		return 1;
	return 0;

nla_put_failure:
	return 1;
}

static int handle_interface_add(struct nl80211_state *state,
				struct nl_cb *cb,
				struct nl_msg *msg,
				int argc, char **argv,
				enum id_input id)
{
	char *name;
	char *mesh_id = NULL;
	enum nl80211_iftype type;
	int tpset;

	if (argc < 1)
		return 1;

	name = argv[0];
	argc--;
	argv++;

	tpset = get_if_type(&argc, &argv, &type, true);
	if (tpset)
		return tpset;

	if (argc) {
		if (strcmp(argv[0], "mesh_id") == 0) {
			argc--;
			argv++;

			if (!argc)
				return 1;
			mesh_id = argv[0];
			argc--;
			argv++;
		} else if (strcmp(argv[0], "4addr") == 0) {
			argc--;
			argv++;
			if (parse_4addr_flag(argv[0], msg)) {
				fprintf(stderr, "4addr error\n");
				return 2;
			}
			argc--;
			argv++;
		} else if (strcmp(argv[0], "flags") == 0) {
			argc--;
			argv++;
			if (parse_mntr_flags(&argc, &argv, msg)) {
				fprintf(stderr, "flags error\n");
				return 2;
			}
		} else {
			return 1;
		}
	}

	if (argc)
		return 1;

	NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, name);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, type);
	if (mesh_id)
		NLA_PUT(msg, NL80211_ATTR_MESH_ID, strlen(mesh_id), mesh_id);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(interface, add, "<name> type <type> [mesh_id <meshid>] [4addr on|off] [flags <flag>*]",
	NL80211_CMD_NEW_INTERFACE, 0, CIB_PHY, handle_interface_add,
	"Add a new virtual interface with the given configuration.\n"
	IFACE_TYPES "\n\n"
	"The flags are only used for monitor interfaces, valid flags are:\n"
	VALID_FLAGS "\n\n"
	"The mesh_id is used only for mesh mode.");
COMMAND(interface, add, "<name> type <type> [mesh_id <meshid>] [4addr on|off] [flags <flag>*]",
	NL80211_CMD_NEW_INTERFACE, 0, CIB_NETDEV, handle_interface_add, NULL);

static int handle_interface_del(struct nl80211_state *state,
				struct nl_cb *cb,
				struct nl_msg *msg,
				int argc, char **argv,
				enum id_input id)
{
	return 0;
}
TOPLEVEL(del, NULL, NL80211_CMD_DEL_INTERFACE, 0, CIB_NETDEV, handle_interface_del,
	 "Remove this virtual interface");
HIDDEN(interface, del, NULL, NL80211_CMD_DEL_INTERFACE, 0, CIB_NETDEV, handle_interface_del);

static char *channel_type_name(enum nl80211_channel_type channel_type)
{
	switch (channel_type) {
	case NL80211_CHAN_NO_HT:
		return "NO HT";
	case NL80211_CHAN_HT20:
		return "HT20";
	case NL80211_CHAN_HT40MINUS:
		return "HT40-";
	case NL80211_CHAN_HT40PLUS:
		return "HT40+";
	default:
		return "unknown";
	}
}

static int print_iface_handler(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	unsigned int *wiphy = arg;
	const char *indent = "";

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (wiphy && tb_msg[NL80211_ATTR_WIPHY]) {
		unsigned int thiswiphy = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
		indent = "\t";
		if (*wiphy != thiswiphy)
			printf("phy#%d\n", thiswiphy);
		*wiphy = thiswiphy;
	}

	if (tb_msg[NL80211_ATTR_IFNAME])
		printf("%sInterface %s\n", indent, nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	else
		printf("%sUnnamed/non-netdev interface\n", indent);
	if (tb_msg[NL80211_ATTR_IFINDEX])
		printf("%s\tifindex %d\n", indent, nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	if (tb_msg[NL80211_ATTR_WDEV])
		printf("%s\twdev 0x%llx\n", indent,
		       (unsigned long long)nla_get_u64(tb_msg[NL80211_ATTR_WDEV]));
	if (tb_msg[NL80211_ATTR_MAC]) {
		char mac_addr[20];
		mac_addr_n2a(mac_addr, nla_data(tb_msg[NL80211_ATTR_MAC]));
		printf("%s\taddr %s\n", indent, mac_addr);
	}
	if (tb_msg[NL80211_ATTR_IFTYPE])
		printf("%s\ttype %s\n", indent, iftype_name(nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE])));
	if (!wiphy && tb_msg[NL80211_ATTR_WIPHY])
		printf("%s\twiphy %d\n", indent, nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]));
	if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) {
		uint32_t freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);
		enum nl80211_channel_type channel_type = NL80211_CHAN_NO_HT;

		if (tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE])
			channel_type = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]);

		printf("%s\tchannel %d (%d MHz) %s\n", indent,
		       ieee80211_frequency_to_channel(freq), freq,
		       channel_type_name(channel_type));
	}

	return NL_SKIP;
}

static int handle_interface_info(struct nl80211_state *state,
				 struct nl_cb *cb,
				 struct nl_msg *msg,
				 int argc, char **argv,
				 enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_iface_handler, NULL);
	return 0;
}
TOPLEVEL(info, NULL, NL80211_CMD_GET_INTERFACE, 0, CIB_NETDEV, handle_interface_info,
	 "Show information for this interface.");

static int handle_interface_set(struct nl80211_state *state,
				struct nl_cb *cb,
				struct nl_msg *msg,
				int argc, char **argv,
				enum id_input id)
{
	if (!argc)
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);

	switch (parse_mntr_flags(&argc, &argv, msg)) {
	case 0:
		return 0;
	case -ENOMEM:
		fprintf(stderr, "failed to allocate flags\n");
		return 2;
	case -EINVAL:
		fprintf(stderr, "unknown flag %s\n", *argv);
		return 2;
	default:
		return 2;
	}
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, monitor, "<flag>*",
	NL80211_CMD_SET_INTERFACE, 0, CIB_NETDEV, handle_interface_set,
	"Set monitor flags. Valid flags are:\n"
	VALID_FLAGS);

static int handle_interface_meshid(struct nl80211_state *state,
				   struct nl_cb *cb,
				   struct nl_msg *msg,
				   int argc, char **argv,
				   enum id_input id)
{
	char *mesh_id = NULL;

	if (argc != 1)
		return 1;

	mesh_id = argv[0];

	NLA_PUT(msg, NL80211_ATTR_MESH_ID, strlen(mesh_id), mesh_id);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, meshid, "<meshid>",
	NL80211_CMD_SET_INTERFACE, 0, CIB_NETDEV, handle_interface_meshid, NULL);

static unsigned int dev_dump_wiphy;

static int handle_dev_dump(struct nl80211_state *state,
			   struct nl_cb *cb,
			   struct nl_msg *msg,
			   int argc, char **argv,
			   enum id_input id)
{
	dev_dump_wiphy = -1;
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_iface_handler, &dev_dump_wiphy);
	return 0;
}
TOPLEVEL(dev, NULL, NL80211_CMD_GET_INTERFACE, NLM_F_DUMP, CIB_NONE, handle_dev_dump,
	 "List all network interfaces for wireless hardware.");

static int handle_interface_type(struct nl80211_state *state,
				 struct nl_cb *cb,
				 struct nl_msg *msg,
				 int argc, char **argv,
				 enum id_input id)
{
	enum nl80211_iftype type;
	int tpset;

	tpset = get_if_type(&argc, &argv, &type, false);
	if (tpset)
		return tpset;

	if (argc)
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, type);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, type, "<type>",
	NL80211_CMD_SET_INTERFACE, 0, CIB_NETDEV, handle_interface_type,
	"Set interface type/mode.\n"
	IFACE_TYPES);

static int handle_interface_4addr(struct nl80211_state *state,
				  struct nl_cb *cb,
				  struct nl_msg *msg,
				  int argc, char **argv,
				  enum id_input id)
{
	if (argc != 1)
		return 1;
	return parse_4addr_flag(argv[0], msg);
}
COMMAND(set, 4addr, "<on|off>",
	NL80211_CMD_SET_INTERFACE, 0, CIB_NETDEV, handle_interface_4addr,
	"Set interface 4addr (WDS) mode.");

static int handle_interface_noack_map(struct nl80211_state *state,
				      struct nl_cb *cb,
				      struct nl_msg *msg,
				      int argc, char **argv,
				      enum id_input id)
{
	uint16_t noack_map;
	char *end;

	if (argc != 1)
		return 1;

	noack_map = strtoul(argv[0], &end, 16);
	if (*end)
		return 1;

	NLA_PUT_U16(msg, NL80211_ATTR_NOACK_MAP, noack_map);

	return 0;
 nla_put_failure:
	return -ENOBUFS;

}
COMMAND(set, noack_map, "<map>",
	NL80211_CMD_SET_NOACK_MAP, 0, CIB_NETDEV, handle_interface_noack_map,
	"Set the NoAck map for the TIDs. (0x0009 = BE, 0x0006 = BK, 0x0030 = VI, 0x00C0 = VO)");


static int handle_interface_wds_peer(struct nl80211_state *state,
				     struct nl_cb *cb,
				     struct nl_msg *msg,
				     int argc, char **argv,
				     enum id_input id)
{
	unsigned char mac_addr[ETH_ALEN];

	if (argc < 1)
		return 1;

	if (mac_addr_a2n(mac_addr, argv[0])) {
		fprintf(stderr, "Invalid MAC address\n");
		return 2;
	}

	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, peer, "<MAC address>",
	NL80211_CMD_SET_WDS_PEER, 0, CIB_NETDEV, handle_interface_wds_peer,
	"Set interface WDS peer.");
