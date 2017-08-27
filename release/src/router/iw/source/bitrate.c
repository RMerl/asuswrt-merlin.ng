#include <errno.h>

#include "nl80211.h"
#include "iw.h"


static int handle_bitrates(struct nl80211_state *state,
			   struct nl_cb *cb,
			   struct nl_msg *msg,
			   int argc, char **argv,
			   enum id_input id)
{
	struct nlattr *nl_rates, *nl_band;
	int i;
	bool have_legacy_24 = false, have_legacy_5 = false;
	uint8_t legacy_24[32], legacy_5[32];
	int n_legacy_24 = 0, n_legacy_5 = 0;
	uint8_t *legacy = NULL;
	int *n_legacy = NULL;
	bool have_mcs_24 = false, have_mcs_5 = false;
	uint8_t mcs_24[77], mcs_5[77];
	int n_mcs_24 = 0, n_mcs_5 = 0;
	uint8_t *mcs = NULL;
	int *n_mcs = NULL;
	enum {
		S_NONE,
		S_LEGACY,
		S_MCS,
	} parser_state = S_NONE;

	for (i = 0; i < argc; i++) {
		char *end;
		double tmpd;
		long tmpl;

		if (strcmp(argv[i], "legacy-2.4") == 0) {
			if (have_legacy_24)
				return 1;
			parser_state = S_LEGACY;
			legacy = legacy_24;
			n_legacy = &n_legacy_24;
			have_legacy_24 = true;
		} else if (strcmp(argv[i], "legacy-5") == 0) {
			if (have_legacy_5)
				return 1;
			parser_state = S_LEGACY;
			legacy = legacy_5;
			n_legacy = &n_legacy_5;
			have_legacy_5 = true;
		}
		else if (strcmp(argv[i], "mcs-2.4") == 0) {
			if (have_mcs_24)
				return 1;
			parser_state = S_MCS;
			mcs = mcs_24;
			n_mcs = &n_mcs_24;
			have_mcs_24 = true;
		} else if (strcmp(argv[i], "mcs-5") == 0) {
			if (have_mcs_5)
				return 1;
			parser_state = S_MCS;
			mcs = mcs_5;
			n_mcs = &n_mcs_5;
			have_mcs_5 = true;
		}
		else switch (parser_state) {
		case S_LEGACY:
			tmpd = strtod(argv[i], &end);
			if (*end != '\0')
				return 1;
			if (tmpd < 1 || tmpd > 255 * 2)
				return 1;
			legacy[(*n_legacy)++] = tmpd * 2;
			break;
		case S_MCS:
			tmpl = strtol(argv[i], &end, 0);
			if (*end != '\0')
				return 1;
			if (tmpl < 0 || tmpl > 255)
				return 1;
			mcs[(*n_mcs)++] = tmpl;
			break;
		default:
			return 1;
		}
	}

	nl_rates = nla_nest_start(msg, NL80211_ATTR_TX_RATES);
	if (!nl_rates)
		goto nla_put_failure;

	if (have_legacy_24 || have_mcs_24) {
		nl_band = nla_nest_start(msg, NL80211_BAND_2GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_24)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_24, legacy_24);
		if (have_mcs_24)
			nla_put(msg, NL80211_TXRATE_MCS, n_mcs_24, mcs_24);
		nla_nest_end(msg, nl_band);
	}

	if (have_legacy_5 || have_mcs_5) {
		nl_band = nla_nest_start(msg, NL80211_BAND_5GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_5)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_5, legacy_5);
		if (have_mcs_5)
			nla_put(msg, NL80211_TXRATE_MCS, n_mcs_5, mcs_5);
		nla_nest_end(msg, nl_band);
	}

	nla_nest_end(msg, nl_rates);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}

#define DESCR_LEGACY "[legacy-<2.4|5> <legacy rate in Mbps>*]"
#define DESCR DESCR_LEGACY " [mcs-<2.4|5> <MCS index>*]"

COMMAND(set, bitrates, "[legacy-<2.4|5> <legacy rate in Mbps>*] [mcs-<2.4|5> <MCS index>*]",
	NL80211_CMD_SET_TX_BITRATE_MASK, 0, CIB_NETDEV, handle_bitrates,
	"Sets up the specified rate masks.\n"
	"Not passing any arguments would clear the existing mask (if any).");
