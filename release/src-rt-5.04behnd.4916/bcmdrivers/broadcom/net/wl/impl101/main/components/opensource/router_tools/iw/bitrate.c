#include <errno.h>

#include "nl80211.h"
#include "iw.h"

static int parse_vht_chunk(const char *arg, __u8 *nss, __u16 *mcs)
{
	int count, i;
	unsigned int inss, mcs_start, mcs_end, tab[10];

	*nss = 0; *mcs = 0;

	if (strchr(arg, '-')) {
		/* Format: NSS:MCS_START-MCS_END */
		count = sscanf(arg, "%u:%u-%u", &inss, &mcs_start, &mcs_end);

		if (count != 3)
			return 0;

		if (inss < 1 || inss > NL80211_VHT_NSS_MAX)
			return 0;

		if (mcs_start > mcs_end)
			return 0;

		if (mcs_start > 9 || mcs_end > 9)
			return 0;

		*nss = inss;
		for (i = mcs_start; i <= mcs_end; i++)
			*mcs |= 1 << i;

	} else {
		/* Format: NSS:MCSx,MCSy,... */
		count = sscanf(arg, "%u:%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", &inss,
			   &tab[0], &tab[1], &tab[2], &tab[3], &tab[4], &tab[5],
			   &tab[6], &tab[7], &tab[8], &tab[9]);

		if (count < 2)
			return 0;

		if (inss < 1 || inss > NL80211_VHT_NSS_MAX)
			return 0;

		*nss = inss;
		for (i = 0; i < count - 1; i++) {
			if (tab[i] > 9)
				return 0;
			*mcs |= 1 << tab[i];
		}
	}

	return 1;
}

static int setup_vht(struct nl80211_txrate_vht *txrate_vht,
		     int argc, char **argv)
{
	__u8 nss;
	__u16 mcs;
	int i;

	memset(txrate_vht, 0, sizeof(*txrate_vht));

	for (i = 0; i < argc; i++) {
		if(!parse_vht_chunk(argv[i], &nss, &mcs))
			return 0;

		nss--;
		txrate_vht->mcs[nss] |= mcs;
	}

	return 1;
}

#define VHT_ARGC_MAX	100

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
	bool have_ht_mcs_24 = false, have_ht_mcs_5 = false;
	bool have_vht_mcs_24 = false, have_vht_mcs_5 = false;
	uint8_t ht_mcs_24[77], ht_mcs_5[77];
	int n_ht_mcs_24 = 0, n_ht_mcs_5 = 0;
	struct nl80211_txrate_vht txrate_vht_24 = {};
	struct nl80211_txrate_vht txrate_vht_5 = {};
	uint8_t *mcs = NULL;
	int *n_mcs = NULL;
	char *vht_argv_5[VHT_ARGC_MAX] = {}; char *vht_argv_24[VHT_ARGC_MAX] = {};
	char **vht_argv = NULL;
	int vht_argc_5 = 0; int vht_argc_24 = 0;
	int *vht_argc = NULL;
	int sgi_24 = 0, sgi_5 = 0, lgi_24 = 0, lgi_5 = 0;

	enum {
		S_NONE,
		S_LEGACY,
		S_HT,
		S_VHT,
		S_GI,
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
		else if (strcmp(argv[i], "ht-mcs-2.4") == 0) {
			if (have_ht_mcs_24)
				return 1;
			parser_state = S_HT;
			mcs = ht_mcs_24;
			n_mcs = &n_ht_mcs_24;
			have_ht_mcs_24 = true;
		} else if (strcmp(argv[i], "ht-mcs-5") == 0) {
			if (have_ht_mcs_5)
				return 1;
			parser_state = S_HT;
			mcs = ht_mcs_5;
			n_mcs = &n_ht_mcs_5;
			have_ht_mcs_5 = true;
		} else if (strcmp(argv[i], "vht-mcs-2.4") == 0) {
			if (have_vht_mcs_24)
				return 1;
			parser_state = S_VHT;
			vht_argv = vht_argv_24;
			vht_argc = &vht_argc_24;
			have_vht_mcs_24 = true;
		} else if (strcmp(argv[i], "vht-mcs-5") == 0) {
			if (have_vht_mcs_5)
				return 1;
			parser_state = S_VHT;
			vht_argv = vht_argv_5;
			vht_argc = &vht_argc_5;
			have_vht_mcs_5 = true;
		} else if (strcmp(argv[i], "sgi-2.4") == 0) {
			sgi_24 = 1;
			parser_state = S_GI;
		} else if (strcmp(argv[i], "sgi-5") == 0) {
			sgi_5 = 1;
			parser_state = S_GI;
		} else if (strcmp(argv[i], "lgi-2.4") == 0) {
			lgi_24 = 1;
			parser_state = S_GI;
		} else if (strcmp(argv[i], "lgi-5") == 0) {
			lgi_5 = 1;
			parser_state = S_GI;
		} else switch (parser_state) {
		case S_LEGACY:
			tmpd = strtod(argv[i], &end);
			if (*end != '\0')
				return 1;
			if (tmpd < 1 || tmpd > 255 * 2)
				return 1;
			legacy[(*n_legacy)++] = tmpd * 2;
			break;
		case S_HT:
			tmpl = strtol(argv[i], &end, 0);
			if (*end != '\0')
				return 1;
			if (tmpl < 0 || tmpl > 255)
				return 1;
			mcs[(*n_mcs)++] = tmpl;
			break;
		case S_VHT:
			if (*vht_argc >= VHT_ARGC_MAX)
				return 1;
			vht_argv[(*vht_argc)++] = argv[i];
			break;
		case S_GI:
			break;
		default:
			return 1;
		}
	}

	if (have_vht_mcs_24)
		if(!setup_vht(&txrate_vht_24, vht_argc_24, vht_argv_24))
			return -EINVAL;

	if (have_vht_mcs_5)
		if(!setup_vht(&txrate_vht_5, vht_argc_5, vht_argv_5))
			return -EINVAL;

	if (sgi_5 && lgi_5)
		return 1;

	if (sgi_24 && lgi_24)
		return 1;

	nl_rates = nla_nest_start(msg, NL80211_ATTR_TX_RATES);
	if (!nl_rates)
		goto nla_put_failure;

	if (have_legacy_24 || have_ht_mcs_24 || have_vht_mcs_24 || sgi_24 || lgi_24) {
		nl_band = nla_nest_start(msg, NL80211_BAND_2GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_24)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_24, legacy_24);
		if (have_ht_mcs_24)
			nla_put(msg, NL80211_TXRATE_HT, n_ht_mcs_24, ht_mcs_24);
		if (have_vht_mcs_24)
			nla_put(msg, NL80211_TXRATE_VHT, sizeof(txrate_vht_24), &txrate_vht_24);
		if (sgi_24)
			nla_put_u8(msg, NL80211_TXRATE_GI, NL80211_TXRATE_FORCE_SGI);
		if (lgi_24)
			nla_put_u8(msg, NL80211_TXRATE_GI, NL80211_TXRATE_FORCE_LGI);
		nla_nest_end(msg, nl_band);
	}

	if (have_legacy_5 || have_ht_mcs_5 || have_vht_mcs_5 || sgi_5 || lgi_5) {
		nl_band = nla_nest_start(msg, NL80211_BAND_5GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_5)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_5, legacy_5);
		if (have_ht_mcs_5)
			nla_put(msg, NL80211_TXRATE_HT, n_ht_mcs_5, ht_mcs_5);
		if (have_vht_mcs_5)
			nla_put(msg, NL80211_TXRATE_VHT, sizeof(txrate_vht_5), &txrate_vht_5);
		if (sgi_5)
			nla_put_u8(msg, NL80211_TXRATE_GI, NL80211_TXRATE_FORCE_SGI);
		if (lgi_5)
			nla_put_u8(msg, NL80211_TXRATE_GI, NL80211_TXRATE_FORCE_LGI);
		nla_nest_end(msg, nl_band);
	}

	nla_nest_end(msg, nl_rates);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}

#define DESCR_LEGACY "[legacy-<2.4|5> <legacy rate in Mbps>*]"
#define DESCR DESCR_LEGACY " [ht-mcs-<2.4|5> <MCS index>*] [vht-mcs-<2.4|5> <NSS:MCSx,MCSy... | NSS:MCSx-MCSy>*] [sgi-2.4|lgi-2.4] [sgi-5|lgi-5]"

COMMAND(set, bitrates, "[legacy-<2.4|5> <legacy rate in Mbps>*] [ht-mcs-<2.4|5> <MCS index>*] [vht-mcs-<2.4|5> <NSS:MCSx,MCSy... | NSS:MCSx-MCSy>*] [sgi-2.4|lgi-2.4] [sgi-5|lgi-5]",
	NL80211_CMD_SET_TX_BITRATE_MASK, 0, CIB_NETDEV, handle_bitrates,
	"Sets up the specified rate masks.\n"
	"Not passing any arguments would clear the existing mask (if any).");
