#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

SECTION(station);

enum plink_state {
	LISTEN,
	OPN_SNT,
	OPN_RCVD,
	CNF_RCVD,
	ESTAB,
	HOLDING,
	BLOCKED
};

enum plink_actions {
	PLINK_ACTION_UNDEFINED,
	PLINK_ACTION_OPEN,
	PLINK_ACTION_BLOCK,
};

static void print_sta_bitrate(struct nlattr *nla, const char *name)
{
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];

	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

	if (!nla)
		return;

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX, nla, rate_policy)) {
		fprintf(stderr, "failed to parse nested rate attributes!\n");
	} else {
		int rate = 0;
		printf("\n\t%s:\t", name);
		if (rinfo[NL80211_RATE_INFO_BITRATE32])
			rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
		else if (rinfo[NL80211_RATE_INFO_BITRATE])
			rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
		if (rate > 0)
			printf("%d.%d MBit/s", rate / 10, rate % 10);

		if (rinfo[NL80211_RATE_INFO_MCS])
			printf(" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
		if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
			printf(" 40Mhz");
		if (rinfo[NL80211_RATE_INFO_SHORT_GI])
			printf(" short GI");
	}
}

static char *get_chain_signal(struct nlattr *attr_list)
{
	struct nlattr *attr;
	static char buf[64];
	char *cur = buf;
	int i = 0, rem;
	const char *prefix;

	if (!attr_list)
		return "";

	nla_for_each_nested(attr, attr_list, rem) {
		if (i++ > 0)
			prefix = ", ";
		else
			prefix = "[";

		cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
				(int8_t) nla_get_u8(attr));
	}

	if (i)
		snprintf(cur, sizeof(buf) - (cur - buf), "] ");

	return buf;
}

static int print_sta_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	char mac_addr[20], state_name[10], dev[20];
	struct nl80211_sta_flag_update *sta_flags;
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
		[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_STA_FLAGS] =
			{ .minlen = sizeof(struct nl80211_sta_flag_update) },
		[NL80211_STA_INFO_CHAIN_SIGNAL] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type = NLA_NESTED },
	};
	char *chain;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Station %s (on %s)", mac_addr, dev);

	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
		printf("\n\tinactive time:\t%u ms",
			nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	if (sinfo[NL80211_STA_INFO_RX_BYTES])
		printf("\n\trx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		printf("\n\trx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_BYTES])
		printf("\n\ttx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	if (sinfo[NL80211_STA_INFO_TX_PACKETS])
		printf("\n\ttx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_RETRIES])
		printf("\n\ttx retries:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
	if (sinfo[NL80211_STA_INFO_TX_FAILED])
		printf("\n\ttx failed:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL])
		printf("\n\tsignal:  \t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]),
			chain);

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]);
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
		printf("\n\tsignal avg:\t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]),
			chain);

	if (sinfo[NL80211_STA_INFO_T_OFFSET])
		printf("\n\tToffset:\t%lld us",
			(unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_T_OFFSET]));

	print_sta_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], "tx bitrate");
	print_sta_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], "rx bitrate");

	if (sinfo[NL80211_STA_INFO_LLID])
		printf("\n\tmesh llid:\t%d",
			nla_get_u16(sinfo[NL80211_STA_INFO_LLID]));
	if (sinfo[NL80211_STA_INFO_PLID])
		printf("\n\tmesh plid:\t%d",
			nla_get_u16(sinfo[NL80211_STA_INFO_PLID]));
	if (sinfo[NL80211_STA_INFO_PLINK_STATE]) {
		switch (nla_get_u8(sinfo[NL80211_STA_INFO_PLINK_STATE])) {
		case LISTEN:
			strcpy(state_name, "LISTEN");
			break;
		case OPN_SNT:
			strcpy(state_name, "OPN_SNT");
			break;
		case OPN_RCVD:
			strcpy(state_name, "OPN_RCVD");
			break;
		case CNF_RCVD:
			strcpy(state_name, "CNF_RCVD");
			break;
		case ESTAB:
			strcpy(state_name, "ESTAB");
			break;
		case HOLDING:
			strcpy(state_name, "HOLDING");
			break;
		case BLOCKED:
			strcpy(state_name, "BLOCKED");
			break;
		default:
			strcpy(state_name, "UNKNOWN");
			break;
		}
		printf("\n\tmesh plink:\t%s", state_name);
	}

	if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
		sta_flags = (struct nl80211_sta_flag_update *)
			    nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			printf("\n\tauthorized:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHORIZED))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
			printf("\n\tauthenticated:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHENTICATED))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
			printf("\n\tpreamble:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE))
				printf("short");
			else
				printf("long");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_WME)) {
			printf("\n\tWMM/WME:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_WME))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_MFP)) {
			printf("\n\tMFP:\t\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_MFP))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			printf("\n\tTDLS peer:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_TDLS_PEER))
				printf("yes");
			else
				printf("no");
		}
	}
	if (sinfo[NL80211_STA_INFO_CONNECTED_TIME])
		printf("\n\tconnected time:\t%u seconds",
		nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]));

	printf("\n");
	return NL_SKIP;
}

static int handle_station_get(struct nl80211_state *state,
			      struct nl_cb *cb,
			      struct nl_msg *msg,
			      int argc, char **argv,
			      enum id_input id)
{
	unsigned char mac_addr[ETH_ALEN];

	if (argc < 1)
		return 1;

	if (mac_addr_a2n(mac_addr, argv[0])) {
		fprintf(stderr, "invalid mac address\n");
		return 2;
	}

	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_sta_handler, NULL);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(station, get, "<MAC address>",
	NL80211_CMD_GET_STATION, 0, CIB_NETDEV, handle_station_get,
	"Get information for a specific station.");
COMMAND(station, del, "<MAC address>",
	NL80211_CMD_DEL_STATION, 0, CIB_NETDEV, handle_station_get,
	"Remove the given station entry (use with caution!)");

static const struct cmd *station_set_plink;
static const struct cmd *station_set_vlan;

static const struct cmd *select_station_cmd(int argc, char **argv)
{
	if (argc < 2)
		return NULL;
	if (strcmp(argv[1], "plink_action") == 0)
		return station_set_plink;
	if (strcmp(argv[1], "vlan") == 0)
		return station_set_vlan;
	return NULL;
}

static int handle_station_set_plink(struct nl80211_state *state,
			      struct nl_cb *cb,
			      struct nl_msg *msg,
			      int argc, char **argv,
			      enum id_input id)
{
	unsigned char plink_action;
	unsigned char mac_addr[ETH_ALEN];

	if (argc < 3)
		return 1;

	if (mac_addr_a2n(mac_addr, argv[0])) {
		fprintf(stderr, "invalid mac address\n");
		return 2;
	}
	argc--;
	argv++;

	if (strcmp("plink_action", argv[0]) != 0)
		return 1;
	argc--;
	argv++;

	if (strcmp("open", argv[0]) == 0)
		plink_action = PLINK_ACTION_OPEN;
	else if (strcmp("block", argv[0]) == 0)
		plink_action = PLINK_ACTION_BLOCK;
	else {
		fprintf(stderr, "plink action not supported\n");
		return 2;
	}
	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);
	NLA_PUT_U8(msg, NL80211_ATTR_STA_PLINK_ACTION, plink_action);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND_ALIAS(station, set, "<MAC address> plink_action <open|block>",
	NL80211_CMD_SET_STATION, 0, CIB_NETDEV, handle_station_set_plink,
	"Set mesh peer link action for this station (peer).",
	select_station_cmd, station_set_plink);

static int handle_station_set_vlan(struct nl80211_state *state,
				   struct nl_cb *cb,
				   struct nl_msg *msg,
				   int argc, char **argv,
				   enum id_input id)
{
	unsigned char mac_addr[ETH_ALEN];
	unsigned long sta_vlan = 0;
	char *err = NULL;

	if (argc < 3)
		return 1;

	if (mac_addr_a2n(mac_addr, argv[0])) {
		fprintf(stderr, "invalid mac address\n");
		return 2;
	}
	argc--;
	argv++;

	if (strcmp("vlan", argv[0]) != 0)
		return 1;
	argc--;
	argv++;

	sta_vlan = strtoul(argv[0], &err, 0);
	if (err && *err) {
		fprintf(stderr, "invalid vlan id\n");
		return 2;
	}
	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);
	NLA_PUT_U32(msg, NL80211_ATTR_STA_VLAN, sta_vlan);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND_ALIAS(station, set, "<MAC address> vlan <ifindex>",
	NL80211_CMD_SET_STATION, 0, CIB_NETDEV, handle_station_set_vlan,
	"Set an AP VLAN for this station.",
	select_station_cmd, station_set_vlan);


static int handle_station_dump(struct nl80211_state *state,
			       struct nl_cb *cb,
			       struct nl_msg *msg,
			       int argc, char **argv,
			       enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_sta_handler, NULL);
	return 0;
}
COMMAND(station, dump, NULL,
	NL80211_CMD_GET_STATION, NLM_F_DUMP, CIB_NETDEV, handle_station_dump,
	"List all stations known, e.g. the AP on managed interfaces");
