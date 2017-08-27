#include <stdbool.h>
#include <errno.h>
#include <net/if.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static void print_flag(const char *name, int *open)
{
	if (!*open)
		printf(" (");
	else
		printf(", ");
	printf("%s", name);
	*open = 1;
}

static char *cipher_name(__u32 c)
{
	static char buf[20];

	switch (c) {
	case 0x000fac01:
		return "WEP40 (00-0f-ac:1)";
	case 0x000fac05:
		return "WEP104 (00-0f-ac:5)";
	case 0x000fac02:
		return "TKIP (00-0f-ac:2)";
	case 0x000fac04:
		return "CCMP (00-0f-ac:4)";
	case 0x000fac06:
		return "CMAC (00-0f-ac:6)";
	case 0x000fac08:
		return "GCMP (00-0f-ac:8)";
	case 0x00147201:
		return "WPI-SMS4 (00-14-72:1)";
	default:
		sprintf(buf, "%.2x-%.2x-%.2x:%d",
			c >> 24, (c >> 16) & 0xff,
			(c >> 8) & 0xff, c & 0xff);

		return buf;
	}
}

static int print_phy_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];

	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
		[NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_RADAR] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
	};

	struct nlattr *tb_rate[NL80211_BITRATE_ATTR_MAX + 1];
	static struct nla_policy rate_policy[NL80211_BITRATE_ATTR_MAX + 1] = {
		[NL80211_BITRATE_ATTR_RATE] = { .type = NLA_U32 },
		[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE] = { .type = NLA_FLAG },
	};

	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	struct nlattr *nl_rate;
	struct nlattr *nl_mode;
	struct nlattr *nl_cmd;
	struct nlattr *nl_if, *nl_ftype;
	int bandidx = 1;
	int rem_band, rem_freq, rem_rate, rem_mode, rem_cmd, rem_ftype, rem_if;
	int open;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb_msg[NL80211_ATTR_WIPHY_BANDS])
		return NL_SKIP;

	if (tb_msg[NL80211_ATTR_WIPHY_NAME])
		printf("Wiphy %s\n", nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]));

	nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
		printf("\tBand %d:\n", bandidx);
		bandidx++;

		nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
			  nla_len(nl_band), NULL);

#ifdef NL80211_BAND_ATTR_HT_CAPA
		if (tb_band[NL80211_BAND_ATTR_HT_CAPA]) {
			__u16 cap = nla_get_u16(tb_band[NL80211_BAND_ATTR_HT_CAPA]);
			print_ht_capability(cap);
		}
		if (tb_band[NL80211_BAND_ATTR_HT_AMPDU_FACTOR]) {
			__u8 exponent = nla_get_u8(tb_band[NL80211_BAND_ATTR_HT_AMPDU_FACTOR]);
			print_ampdu_length(exponent);
		}
		if (tb_band[NL80211_BAND_ATTR_HT_AMPDU_DENSITY]) {
			__u8 spacing = nla_get_u8(tb_band[NL80211_BAND_ATTR_HT_AMPDU_DENSITY]);
			print_ampdu_spacing(spacing);
		}
		if (tb_band[NL80211_BAND_ATTR_HT_MCS_SET] &&
		    nla_len(tb_band[NL80211_BAND_ATTR_HT_MCS_SET]) == 16)
			print_ht_mcs(nla_data(tb_band[NL80211_BAND_ATTR_HT_MCS_SET]));
#endif

		printf("\t\tFrequencies:\n");

		nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
			uint32_t freq;
			nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq),
				  nla_len(nl_freq), freq_policy);
			if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
				continue;
			freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);
			printf("\t\t\t* %d MHz [%d]", freq, ieee80211_frequency_to_channel(freq));

			if (tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] &&
			    !tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
				printf(" (%.1f dBm)", 0.01 * nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]));

			open = 0;
			if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED]) {
				print_flag("disabled", &open);
				goto next;
			}
			if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN])
				print_flag("passive scanning", &open);
			if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IBSS])
				print_flag("no IBSS", &open);
			if (tb_freq[NL80211_FREQUENCY_ATTR_RADAR])
				print_flag("radar detection", &open);
 next:
			if (open)
				printf(")");
			printf("\n");
		}

		printf("\t\tBitrates (non-HT):\n");

		nla_for_each_nested(nl_rate, tb_band[NL80211_BAND_ATTR_RATES], rem_rate) {
			nla_parse(tb_rate, NL80211_BITRATE_ATTR_MAX, nla_data(nl_rate),
				  nla_len(nl_rate), rate_policy);
			if (!tb_rate[NL80211_BITRATE_ATTR_RATE])
				continue;
			printf("\t\t\t* %2.1f Mbps", 0.1 * nla_get_u32(tb_rate[NL80211_BITRATE_ATTR_RATE]));
			open = 0;
			if (tb_rate[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE])
				print_flag("short preamble supported", &open);
			if (open)
				printf(")");
			printf("\n");
		}
	}

	if (tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS])
		printf("\tmax # scan SSIDs: %d\n",
		       nla_get_u8(tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]));
	if (tb_msg[NL80211_ATTR_MAX_SCAN_IE_LEN])
		printf("\tmax scan IEs length: %d bytes\n",
		       nla_get_u16(tb_msg[NL80211_ATTR_MAX_SCAN_IE_LEN]));

	if (tb_msg[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]) {
		unsigned int frag;

		frag = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]);
		if (frag != (unsigned int)-1)
			printf("\tFragmentation threshold: %d\n", frag);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_RTS_THRESHOLD]) {
		unsigned int rts;

		rts = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_RTS_THRESHOLD]);
		if (rts != (unsigned int)-1)
			printf("\tRTS threshold: %d\n", rts);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_COVERAGE_CLASS]) {
		unsigned char coverage;

		coverage = nla_get_u8(tb_msg[NL80211_ATTR_WIPHY_COVERAGE_CLASS]);
		/* See handle_distance() for an explanation where the '450' comes from */
		printf("\tCoverage class: %d (up to %dm)\n", coverage, 450 * coverage);
	}

	if (tb_msg[NL80211_ATTR_CIPHER_SUITES]) {
		int num = nla_len(tb_msg[NL80211_ATTR_CIPHER_SUITES]) / sizeof(__u32);
		int i;
		__u32 *ciphers = nla_data(tb_msg[NL80211_ATTR_CIPHER_SUITES]);
		if (num > 0) {
			printf("\tSupported Ciphers:\n");
			for (i = 0; i < num; i++)
				printf("\t\t* %s\n",
					cipher_name(ciphers[i]));
		}
	}

	if (tb_msg[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX] &&
	    tb_msg[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX])
		printf("\tAvailable Antennas: TX %#x RX %#x\n",
		       nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]),
		       nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]));

	if (tb_msg[NL80211_ATTR_WIPHY_ANTENNA_TX] &&
	    tb_msg[NL80211_ATTR_WIPHY_ANTENNA_RX])
		printf("\tConfigured Antennas: TX %#x RX %#x\n",
		       nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_ANTENNA_TX]),
		       nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_ANTENNA_RX]));

	if (tb_msg[NL80211_ATTR_SUPPORTED_IFTYPES]) {
		printf("\tSupported interface modes:\n");
		nla_for_each_nested(nl_mode, tb_msg[NL80211_ATTR_SUPPORTED_IFTYPES], rem_mode)
			printf("\t\t * %s\n", iftype_name(nla_type(nl_mode)));
	}

	if (tb_msg[NL80211_ATTR_SOFTWARE_IFTYPES]) {
		printf("\tsoftware interface modes (can always be added):\n");
		nla_for_each_nested(nl_mode, tb_msg[NL80211_ATTR_SOFTWARE_IFTYPES], rem_mode)
			printf("\t\t * %s\n", iftype_name(nla_type(nl_mode)));
	}

	if (tb_msg[NL80211_ATTR_INTERFACE_COMBINATIONS]) {
		struct nlattr *nl_combi;
		int rem_combi;
		bool have_combinations = false;

		nla_for_each_nested(nl_combi, tb_msg[NL80211_ATTR_INTERFACE_COMBINATIONS], rem_combi) {
			static struct nla_policy iface_combination_policy[NUM_NL80211_IFACE_COMB] = {
				[NL80211_IFACE_COMB_LIMITS] = { .type = NLA_NESTED },
				[NL80211_IFACE_COMB_MAXNUM] = { .type = NLA_U32 },
				[NL80211_IFACE_COMB_STA_AP_BI_MATCH] = { .type = NLA_FLAG },
				[NL80211_IFACE_COMB_NUM_CHANNELS] = { .type = NLA_U32 },
			};
			struct nlattr *tb_comb[NUM_NL80211_IFACE_COMB];
			static struct nla_policy iface_limit_policy[NUM_NL80211_IFACE_LIMIT] = {
				[NL80211_IFACE_LIMIT_TYPES] = { .type = NLA_NESTED },
				[NL80211_IFACE_LIMIT_MAX] = { .type = NLA_U32 },
			};
			struct nlattr *tb_limit[NUM_NL80211_IFACE_LIMIT];
			struct nlattr *nl_limit;
			int err, rem_limit;
			bool comma = false;

			if (!have_combinations) {
				printf("\tvalid interface combinations:\n");
				have_combinations = true;
			}

			printf("\t\t * ");

			err = nla_parse_nested(tb_comb, MAX_NL80211_IFACE_COMB,
					       nl_combi, iface_combination_policy);
			if (err || !tb_comb[NL80211_IFACE_COMB_LIMITS] ||
			    !tb_comb[NL80211_IFACE_COMB_MAXNUM] ||
			    !tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]) {
				printf(" <failed to parse>\n");
				goto broken_combination;
			}

			nla_for_each_nested(nl_limit, tb_comb[NL80211_IFACE_COMB_LIMITS], rem_limit) {
				bool ift_comma = false;

				err = nla_parse_nested(tb_limit, MAX_NL80211_IFACE_LIMIT,
						       nl_limit, iface_limit_policy);
				if (err || !tb_limit[NL80211_IFACE_LIMIT_TYPES]) {
					printf("<failed to parse>\n");
					goto broken_combination;
				}

				if (comma)
					printf(", ");
				comma = true;
				printf("#{");

				nla_for_each_nested(nl_mode, tb_limit[NL80211_IFACE_LIMIT_TYPES], rem_mode) {
					printf("%s %s", ift_comma ? "," : "",
						iftype_name(nla_type(nl_mode)));
					ift_comma = true;
				}
				printf(" } <= %u", nla_get_u32(tb_limit[NL80211_IFACE_LIMIT_MAX]));
			}
			printf(",\n\t\t   ");

			printf("total <= %d, #channels <= %d%s\n",
				nla_get_u32(tb_comb[NL80211_IFACE_COMB_MAXNUM]),
				nla_get_u32(tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]),
				tb_comb[NL80211_IFACE_COMB_STA_AP_BI_MATCH] ?
					", STA/AP BI must match" : "");
broken_combination:
			;
		}

		if (!have_combinations)
			printf("\tinterface combinations are not supported\n");
	}

	if (tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS]) {
		printf("\tSupported commands:\n");
		nla_for_each_nested(nl_cmd, tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS], rem_cmd)
			printf("\t\t * %s\n", command_name(nla_get_u32(nl_cmd)));
	}

	if (tb_msg[NL80211_ATTR_TX_FRAME_TYPES]) {
		printf("\tSupported TX frame types:\n");
		nla_for_each_nested(nl_if, tb_msg[NL80211_ATTR_TX_FRAME_TYPES], rem_if) {
			bool printed = false;
			nla_for_each_nested(nl_ftype, nl_if, rem_ftype) {
				if (!printed)
					printf("\t\t * %s:", iftype_name(nla_type(nl_if)));
				printed = true;
				printf(" 0x%.2x", nla_get_u16(nl_ftype));
			}
			if (printed)
				printf("\n");
		}
	}

	if (tb_msg[NL80211_ATTR_RX_FRAME_TYPES]) {
		printf("\tSupported RX frame types:\n");
		nla_for_each_nested(nl_if, tb_msg[NL80211_ATTR_RX_FRAME_TYPES], rem_if) {
			bool printed = false;
			nla_for_each_nested(nl_ftype, nl_if, rem_ftype) {
				if (!printed)
					printf("\t\t * %s:", iftype_name(nla_type(nl_if)));
				printed = true;
				printf(" 0x%.2x", nla_get_u16(nl_ftype));
			}
			if (printed)
				printf("\n");
		}
	}

	if (tb_msg[NL80211_ATTR_SUPPORT_IBSS_RSN])
		printf("\tDevice supports RSN-IBSS.\n");

	if (tb_msg[NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED]) {
		struct nlattr *tb_wowlan[NUM_NL80211_WOWLAN_TRIG];
		static struct nla_policy wowlan_policy[NUM_NL80211_WOWLAN_TRIG] = {
			[NL80211_WOWLAN_TRIG_ANY] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_DISCONNECT] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_MAGIC_PKT] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_PKT_PATTERN] = {
				.minlen = sizeof(struct nl80211_wowlan_pattern_support),
			},
			[NL80211_WOWLAN_TRIG_GTK_REKEY_SUPPORTED] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_RFKILL_RELEASE] = { .type = NLA_FLAG },
		};
		struct nl80211_wowlan_pattern_support *pat;
		int err;

		err = nla_parse_nested(tb_wowlan, MAX_NL80211_WOWLAN_TRIG,
				       tb_msg[NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED],
				       wowlan_policy);
		printf("\tWoWLAN support:");
		if (err) {
			printf(" <failed to parse>\n");
		} else {
			printf("\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_ANY])
				printf("\t\t * wake up on anything (device continues operating normally)\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_DISCONNECT])
				printf("\t\t * wake up on disconnect\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_MAGIC_PKT])
				printf("\t\t * wake up on magic packet\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_PKT_PATTERN]) {
				pat = nla_data(tb_wowlan[NL80211_WOWLAN_TRIG_PKT_PATTERN]);
				printf("\t\t * wake up on pattern match, up to %u patterns of %u-%u bytes\n",
					pat->max_patterns, pat->min_pattern_len, pat->max_pattern_len);
			}
			if (tb_wowlan[NL80211_WOWLAN_TRIG_GTK_REKEY_SUPPORTED])
				printf("\t\t * can do GTK rekeying\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE])
				printf("\t\t * wake up on GTK rekey failure\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST])
				printf("\t\t * wake up on EAP identity request\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE])
				printf("\t\t * wake up on 4-way handshake\n");
			if (tb_wowlan[NL80211_WOWLAN_TRIG_RFKILL_RELEASE])
				printf("\t\t * wake up on rfkill release\n");
		}
	}

	if (tb_msg[NL80211_ATTR_ROAM_SUPPORT])
		printf("\tDevice supports roaming.\n");

	if (tb_msg[NL80211_ATTR_SUPPORT_AP_UAPSD])
		printf("\tDevice supports AP-side u-APSD.\n");

	if (tb_msg[NL80211_ATTR_HT_CAPABILITY_MASK]) {
		struct ieee80211_ht_cap *cm;
		printf("\tHT Capability overrides:\n");
		if (nla_len(tb_msg[NL80211_ATTR_HT_CAPABILITY_MASK]) >= sizeof(*cm)) {
			cm = nla_data(tb_msg[NL80211_ATTR_HT_CAPABILITY_MASK]);
			printf("\t\t * MCS: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx"
			       " %02hhx %02hhx %02hhx %02hhx\n",
			       cm->mcs.rx_mask[0], cm->mcs.rx_mask[1],
			       cm->mcs.rx_mask[2], cm->mcs.rx_mask[3],
			       cm->mcs.rx_mask[4], cm->mcs.rx_mask[5],
			       cm->mcs.rx_mask[6], cm->mcs.rx_mask[7],
			       cm->mcs.rx_mask[8], cm->mcs.rx_mask[9]);
			if (cm->cap_info & htole16(IEEE80211_HT_CAP_MAX_AMSDU))
				printf("\t\t * maximum A-MSDU length\n");
			if (cm->cap_info & htole16(IEEE80211_HT_CAP_SUP_WIDTH_20_40))
				printf("\t\t * supported channel width\n");
			if (cm->cap_info & htole16(IEEE80211_HT_CAP_SGI_40))
				printf("\t\t * short GI for 40 MHz\n");
			if (cm->ampdu_params_info & IEEE80211_HT_AMPDU_PARM_FACTOR)
				printf("\t\t * max A-MPDU length exponent\n");
			if (cm->ampdu_params_info & IEEE80211_HT_AMPDU_PARM_DENSITY)
				printf("\t\t * min MPDU start spacing\n");
		} else {
			printf("\tERROR: capabilities mask is too short, expected: %d, received: %d\n",
			       (int)(sizeof(*cm)),
			       (int)(nla_len(tb_msg[NL80211_ATTR_HT_CAPABILITY_MASK])));
		}
	}

	if (tb_msg[NL80211_ATTR_FEATURE_FLAGS]) {
		if (nla_get_u32(tb_msg[NL80211_ATTR_FEATURE_FLAGS]) &
				NL80211_FEATURE_SK_TX_STATUS)
			printf("\tDevice supports TX status socket option.\n");
		if (nla_get_u32(tb_msg[NL80211_ATTR_FEATURE_FLAGS]) &
				NL80211_FEATURE_HT_IBSS)
			printf("\tDevice supports HT-IBSS.\n");
	}

	return NL_SKIP;
}

static int handle_info(struct nl80211_state *state,
		       struct nl_cb *cb,
		       struct nl_msg *msg,
		       int argc, char **argv,
		       enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_phy_handler, NULL);

	return 0;
}
__COMMAND(NULL, info, "info", NULL, NL80211_CMD_GET_WIPHY, 0, 0, CIB_PHY, handle_info,
	 "Show capabilities for the specified wireless device.", NULL);
TOPLEVEL(list, NULL, NL80211_CMD_GET_WIPHY, NLM_F_DUMP, CIB_NONE, handle_info,
	 "List all wireless devices and their capabilities.");
TOPLEVEL(phy, NULL, NL80211_CMD_GET_WIPHY, NLM_F_DUMP, CIB_NONE, handle_info, NULL);
