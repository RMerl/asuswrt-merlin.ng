#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

SECTION(ibss);

static int join_ibss(struct nl80211_state *state,
		     struct nl_cb *cb,
		     struct nl_msg *msg,
		     int argc, char **argv,
		     enum id_input id)
{
	char *end;
	unsigned char abssid[6];
	unsigned char rates[NL80211_MAX_SUPP_RATES];
	int n_rates = 0;
	char *value = NULL, *sptr = NULL;
	float rate;
	int bintval;
	int i;
	unsigned long freq;
	static const struct {
		const char *name;
		unsigned int width;
		int freq1_diff;
		int chantype; /* for older kernel */
	} *chanmode_selected = NULL, chanmode[] = {
		{ .name = "HT20",
		  .width = NL80211_CHAN_WIDTH_20,
		  .freq1_diff = 0,
		  .chantype = NL80211_CHAN_HT20 },
		{ .name = "HT40+",
		  .width = NL80211_CHAN_WIDTH_40,
		  .freq1_diff = 10,
		  .chantype = NL80211_CHAN_HT40PLUS },
		{ .name = "HT40-",
		  .width = NL80211_CHAN_WIDTH_40,
		  .freq1_diff = -10,
		  .chantype = NL80211_CHAN_HT40MINUS },
		{ .name = "NOHT",
		  .width = NL80211_CHAN_WIDTH_20_NOHT,
		  .freq1_diff = 0,
		  .chantype = NL80211_CHAN_NO_HT },
		{ .name = "5MHZ",
		  .width = NL80211_CHAN_WIDTH_5,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "10MHZ",
		  .width = NL80211_CHAN_WIDTH_10,
		  .freq1_diff = 0,
		  .chantype = -1 },
	};

	if (argc < 2)
		return 1;

	/* SSID */
	NLA_PUT(msg, NL80211_ATTR_SSID, strlen(argv[0]), argv[0]);
	argv++;
	argc--;

	/* freq */
	freq = strtoul(argv[0], &end, 10);
	if (*end != '\0')
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	argv++;
	argc--;

	if (argc) {
		for (i = 0; i < ARRAY_SIZE(chanmode); i++) {
			if (strcasecmp(chanmode[i].name, argv[0]) == 0) {
				chanmode_selected = &chanmode[i];
				break;
			}
		}
		if (chanmode_selected) {
			NLA_PUT_U32(msg, NL80211_ATTR_CHANNEL_WIDTH,
				    chanmode_selected->width);
			NLA_PUT_U32(msg, NL80211_ATTR_CENTER_FREQ1,
				    freq + chanmode_selected->freq1_diff);
			if (chanmode_selected->chantype != -1)
				NLA_PUT_U32(msg,
					    NL80211_ATTR_WIPHY_CHANNEL_TYPE,
					    chanmode_selected->chantype);

			argv++;
			argc--;
		}

	}

	if (argc && strcmp(argv[0], "fixed-freq") == 0) {
		NLA_PUT_FLAG(msg, NL80211_ATTR_FREQ_FIXED);
		argv++;
		argc--;
	}

	if (argc) {
		if (mac_addr_a2n(abssid, argv[0]) == 0) {
			NLA_PUT(msg, NL80211_ATTR_MAC, 6, abssid);
			argv++;
			argc--;
		}
	}

	if (argc > 1 && strcmp(argv[0], "beacon-interval") == 0) {
		argv++;
		argc--;
		bintval = strtoul(argv[0], &end, 10);
		if (*end != '\0')
			return 1;
		NLA_PUT_U32(msg, NL80211_ATTR_BEACON_INTERVAL, bintval);
		argv++;
		argc--;
	}

	/* basic rates */
	if (argc > 1 && strcmp(argv[0], "basic-rates") == 0) {
		argv++;
		argc--;

		value = strtok_r(argv[0], ",", &sptr);

		while (value && n_rates < NL80211_MAX_SUPP_RATES) {
			rate = strtod(value, &end);
			rates[n_rates] = rate * 2;

			/* filter out suspicious values  */
			if (*end != '\0' || !rates[n_rates] ||
			    rate*2 != rates[n_rates])
				return 1;

			n_rates++;
			value = strtok_r(NULL, ",", &sptr);
		}

		NLA_PUT(msg, NL80211_ATTR_BSS_BASIC_RATES, n_rates, rates);

		argv++;
		argc--;
	}

	/* multicast rate */
	if (argc > 1 && strcmp(argv[0], "mcast-rate") == 0) {
		argv++;
		argc--;

		rate = strtod(argv[0], &end);
		if (*end != '\0')
			return 1;

		NLA_PUT_U32(msg, NL80211_ATTR_MCAST_RATE, (int)(rate * 10));
		argv++;
		argc--;
	}

	if (!argc)
		return 0;

	if (strcmp(*argv, "key") != 0 && strcmp(*argv, "keys") != 0)
		return 1;

	argv++;
	argc--;

	return parse_keys(msg, argv, argc);
 nla_put_failure:
	return -ENOSPC;
}

static int leave_ibss(struct nl80211_state *state,
		      struct nl_cb *cb,
		      struct nl_msg *msg,
		      int argc, char **argv,
		      enum id_input id)
{
	return 0;
}
COMMAND(ibss, leave, NULL,
	NL80211_CMD_LEAVE_IBSS, 0, CIB_NETDEV, leave_ibss,
	"Leave the current IBSS cell.");
COMMAND(ibss, join,
	"<SSID> <freq in MHz> [HT20|HT40+|HT40-|NOHT|5MHZ|10MHZ] [fixed-freq] [<fixed bssid>] [beacon-interval <TU>]"
	" [basic-rates <rate in Mbps,rate2,...>] [mcast-rate <rate in Mbps>] "
	"[key d:0:abcde]",
	NL80211_CMD_JOIN_IBSS, 0, CIB_NETDEV, join_ibss,
	"Join the IBSS cell with the given SSID, if it doesn't exist create\n"
	"it on the given frequency. When fixed frequency is requested, don't\n"
	"join/create a cell on a different frequency. When a fixed BSSID is\n"
	"requested use that BSSID and do not adopt another cell's BSSID even\n"
	"if it has higher TSF and the same SSID. If an IBSS is created, create\n"
	"it with the specified basic-rates, multicast-rate and beacon-interval.");
