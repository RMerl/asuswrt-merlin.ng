#include <stdbool.h>
#include <errno.h>
#include <net/if.h>
#include <strings.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static int handle_name(struct nl80211_state *state,
		       struct nl_cb *cb,
		       struct nl_msg *msg,
		       int argc, char **argv,
		       enum id_input id)
{
	if (argc != 1)
		return 1;

	NLA_PUT_STRING(msg, NL80211_ATTR_WIPHY_NAME, *argv);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, name, "<new name>", NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_name,
	"Rename this wireless device.");

static int handle_freqchan(struct nl_msg *msg, bool chan,
			   int argc, char **argv)
{
	char *end;
	static const struct {
		const char *name;
		unsigned int val;
	} htmap[] = {
		{ .name = "HT20", .val = NL80211_CHAN_HT20, },
		{ .name = "HT40+", .val = NL80211_CHAN_HT40PLUS, },
		{ .name = "HT40-", .val = NL80211_CHAN_HT40MINUS, },
	};
	unsigned int htval = NL80211_CHAN_NO_HT;
	unsigned int freq;
	int i;

	if (!argc || argc > 2)
		return 1;

	if (argc == 2) {
		for (i = 0; i < ARRAY_SIZE(htmap); i++) {
			if (strcasecmp(htmap[i].name, argv[1]) == 0) {
				htval = htmap[i].val;
				break;
			}
		}
		if (htval == NL80211_CHAN_NO_HT)
			return 1;
	}

	if (!*argv[0])
		return 1;
	freq = strtoul(argv[0], &end, 10);
	if (*end)
		return 1;

	if (chan)
		freq = ieee80211_channel_to_frequency(freq);

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, htval);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}

static int handle_freq(struct nl80211_state *state,
		       struct nl_cb *cb, struct nl_msg *msg,
		       int argc, char **argv,
		       enum id_input id)
{
	return handle_freqchan(msg, false, argc, argv);
}
COMMAND(set, freq, "<freq> [HT20|HT40+|HT40-]",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_freq,
	"Set frequency/channel the hardware is using, including HT\n"
	"configuration.");
COMMAND(set, freq, "<freq> [HT20|HT40+|HT40-]",
	NL80211_CMD_SET_WIPHY, 0, CIB_NETDEV, handle_freq, NULL);

static int handle_chan(struct nl80211_state *state,
		       struct nl_cb *cb, struct nl_msg *msg,
		       int argc, char **argv,
		       enum id_input id)
{
	return handle_freqchan(msg, true, argc, argv);
}
COMMAND(set, channel, "<channel> [HT20|HT40+|HT40-]",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_chan, NULL);
COMMAND(set, channel, "<channel> [HT20|HT40+|HT40-]",
	NL80211_CMD_SET_WIPHY, 0, CIB_NETDEV, handle_chan, NULL);

static int handle_fragmentation(struct nl80211_state *state,
				struct nl_cb *cb, struct nl_msg *msg,
				int argc, char **argv,
				enum id_input id)
{
	unsigned int frag;

	if (argc != 1)
		return 1;

	if (strcmp("off", argv[0]) == 0)
		frag = -1;
	else {
		char *end;

		if (!*argv[0])
			return 1;
		frag = strtoul(argv[0], &end, 10);
		if (*end != '\0')
			return 1;
	}

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FRAG_THRESHOLD, frag);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, frag, "<fragmentation threshold|off>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_fragmentation,
	"Set fragmentation threshold.");

static int handle_rts(struct nl80211_state *state,
		      struct nl_cb *cb, struct nl_msg *msg,
		      int argc, char **argv,
		      enum id_input id)
{
	unsigned int rts;

	if (argc != 1)
		return 1;

	if (strcmp("off", argv[0]) == 0)
		rts = -1;
	else {
		char *end;

		if (!*argv[0])
			return 1;
		rts = strtoul(argv[0], &end, 10);
		if (*end != '\0')
			return 1;
	}

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_RTS_THRESHOLD, rts);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, rts, "<rts threshold|off>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_rts,
	"Set rts threshold.");

static int handle_netns(struct nl80211_state *state,
			struct nl_cb *cb,
			struct nl_msg *msg,
			int argc, char **argv,
			enum id_input id)
{
	char *end;

	if (argc != 1)
		return 1;

	if (!*argv[0])
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_PID,
		    strtoul(argv[0], &end, 10));

	if (*end != '\0')
		return 1;

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, netns, "<pid>",
	NL80211_CMD_SET_WIPHY_NETNS, 0, CIB_PHY, handle_netns,
	"Put this wireless device into a different network namespace");

static int handle_coverage(struct nl80211_state *state,
			struct nl_cb *cb,
			struct nl_msg *msg,
			int argc, char **argv,
			enum id_input id)
{
	char *end;
	unsigned int coverage;

	if (argc != 1)
		return 1;

	if (!*argv[0])
		return 1;
	coverage = strtoul(argv[0], &end, 10);
	if (coverage > 255)
		return 1;

	if (*end)
		return 1;

	NLA_PUT_U8(msg, NL80211_ATTR_WIPHY_COVERAGE_CLASS, coverage);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, coverage, "<coverage class>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_coverage,
	"Set coverage class (1 for every 3 usec of air propagation time).\n"
	"Valid values: 0 - 255.");

static int handle_distance(struct nl80211_state *state,
			struct nl_cb *cb,
			struct nl_msg *msg,
			int argc, char **argv,
			enum id_input id)
{
	char *end;
	unsigned int distance, coverage;

	if (argc != 1)
		return 1;

	if (!*argv[0])
		return 1;

	distance = strtoul(argv[0], &end, 10);

	if (*end)
		return 1;

	/*
	 * Divide double the distance by the speed of light in m/usec (300) to
	 * get round-trip time in microseconds and then divide the result by
	 * three to get coverage class as specified in IEEE 802.11-2007 table
	 * 7-27. Values are rounded upwards.
	 */
	coverage = (distance + 449) / 450;
	if (coverage > 255)
		return 1;

	NLA_PUT_U8(msg, NL80211_ATTR_WIPHY_COVERAGE_CLASS, coverage);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, distance, "<distance>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_distance,
	"Set appropriate coverage class for given link distance in meters.\n"
	"Valid values: 0 - 114750");

static int handle_txpower(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	enum nl80211_tx_power_setting type;
	int mbm;

	/* get the required args */
	if (argc != 1 && argc != 2)
		return 1;

	if (!strcmp(argv[0], "auto"))
		type = NL80211_TX_POWER_AUTOMATIC;
	else if (!strcmp(argv[0], "fixed"))
		type = NL80211_TX_POWER_FIXED;
	else if (!strcmp(argv[0], "limit"))
		type = NL80211_TX_POWER_LIMITED;
	else {
		printf("Invalid parameter: %s\n", argv[0]);
		return 2;
	}

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_TX_POWER_SETTING, type);

	if (type != NL80211_TX_POWER_AUTOMATIC) {
		char *endptr;
		if (argc != 2) {
			printf("Missing TX power level argument.\n");
			return 2;
		}

		mbm = strtol(argv[1], &endptr, 10);
		if (*endptr)
			return 2;
		NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_TX_POWER_LEVEL, mbm);
	} else if (argc != 1)
		return 1;

	return 0;

 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, txpower, "<auto|fixed|limit> [<tx power in mBm>]",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_txpower,
	"Specify transmit power level and setting type.");
COMMAND(set, txpower, "<auto|fixed|limit> [<tx power in mBm>]",
	NL80211_CMD_SET_WIPHY, 0, CIB_NETDEV, handle_txpower,
	"Specify transmit power level and setting type.");

static int handle_antenna(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	char *end;
	uint32_t tx_ant = 0, rx_ant = 0;

	if (argc == 1 && strcmp(argv[0], "all") == 0) {
		tx_ant = 0xffffffff;
		rx_ant = 0xffffffff;
	} else if (argc == 1) {
		tx_ant = rx_ant = strtoul(argv[0], &end, 0);
		if (*end)
			return 1;
	}
	else if (argc == 2) {
		tx_ant = strtoul(argv[0], &end, 0);
		if (*end)
			return 1;
		rx_ant = strtoul(argv[1], &end, 0);
		if (*end)
			return 1;
	} else
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_TX, tx_ant);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_RX, rx_ant);

	return 0;

 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, antenna, "<bitmap> | all | <tx bitmap> <rx bitmap>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_antenna,
	"Set a bitmap of allowed antennas to use for TX and RX.\n"
	"The driver may reject antenna configurations it cannot support.");

static int handle_antenna_gain(struct nl80211_state *state,
			       struct nl_cb *cb,
			       struct nl_msg *msg,
			       int argc, char **argv,
			       enum id_input id)
{
	char *endptr;
	int dbm;

	/* get the required args */
	if (argc != 1)
		return 1;

	dbm = strtol(argv[0], &endptr, 10);
	if (*endptr)
		return 2;

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_GAIN, dbm);

	return 0;

 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(set, antenna_gain, "<antenna gain in dBm>",
	NL80211_CMD_SET_WIPHY, 0, CIB_PHY, handle_antenna_gain,
	"Specify antenna gain.");
