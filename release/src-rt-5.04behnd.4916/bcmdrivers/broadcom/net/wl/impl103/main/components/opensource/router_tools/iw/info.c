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

static char *dfs_state_name(enum nl80211_dfs_state state)
{
	switch (state) {
	case NL80211_DFS_USABLE:
		return "usable";
	case NL80211_DFS_AVAILABLE:
		return "available";
	case NL80211_DFS_UNAVAILABLE:
		return "unavailable";
	default:
		return "unknown";
	}
}

static int ext_feature_isset(const unsigned char *ext_features, int ext_features_len,
			     enum nl80211_ext_feature_index ftidx)
{
	unsigned char ft_byte;

	if ((int) ftidx / 8 >= ext_features_len)
		return 0;

	ft_byte = ext_features[ftidx / 8];
	return (ft_byte & BIT(ftidx % 8)) != 0;
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
		[NL80211_FREQUENCY_ATTR_NO_IR] = { .type = NLA_FLAG },
		[__NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
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
	int rem_band, rem_freq, rem_rate, rem_mode, rem_cmd, rem_ftype, rem_if;
	int open;
	/*
	 * static variables only work here, other applications need to use the
	 * callback pointer and store them there so they can be multithreaded
	 * and/or have multiple netlink sockets, etc.
	 */
	static int64_t phy_id = -1;
	static int last_band = -1;
	static bool band_had_freq = false;
	bool print_name = true;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (tb_msg[NL80211_ATTR_WIPHY]) {
		if (nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]) == phy_id)
			print_name = false;
		else
			last_band = -1;
		phy_id = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
	}
	if (print_name && tb_msg[NL80211_ATTR_WIPHY_NAME])
		printf("Wiphy %s\n", nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]));

	/* needed for split dump */
	if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
		nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
			if (last_band != nl_band->nla_type) {
				printf("\tBand %d:\n", nl_band->nla_type + 1);
				band_had_freq = false;
			}
			last_band = nl_band->nla_type;

			nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
				  nla_len(nl_band), NULL);

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
			if (tb_band[NL80211_BAND_ATTR_VHT_CAPA] &&
			    tb_band[NL80211_BAND_ATTR_VHT_MCS_SET])
				print_vht_info(nla_get_u32(tb_band[NL80211_BAND_ATTR_VHT_CAPA]),
					       nla_data(tb_band[NL80211_BAND_ATTR_VHT_MCS_SET]));

			if (tb_band[NL80211_BAND_ATTR_FREQS]) {
				if (!band_had_freq) {
					printf("\t\tFrequencies:\n");
					band_had_freq = true;
				}
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

					/* If both flags are set assume an new kernel */
					if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IR] && tb_freq[__NL80211_FREQUENCY_ATTR_NO_IBSS]) {
						print_flag("no IR", &open);
					} else if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN]) {
						print_flag("passive scan", &open);
					} else if (tb_freq[__NL80211_FREQUENCY_ATTR_NO_IBSS]){
						print_flag("no ibss", &open);
					}

					if (tb_freq[NL80211_FREQUENCY_ATTR_RADAR])
						print_flag("radar detection", &open);
next:
					if (open)
						printf(")");
					printf("\n");

					if (!tb_freq[NL80211_FREQUENCY_ATTR_DISABLED] && tb_freq[NL80211_FREQUENCY_ATTR_DFS_STATE]) {
						enum nl80211_dfs_state state = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_DFS_STATE]);
						unsigned long time;

						printf("\t\t\t  DFS state: %s", dfs_state_name(state));
						if (tb_freq[NL80211_FREQUENCY_ATTR_DFS_TIME]) {
							time = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_DFS_TIME]);
							printf(" (for %lu sec)", time/1000);
						}
						printf("\n");
						if (tb_freq[NL80211_FREQUENCY_ATTR_DFS_CAC_TIME])
							printf("\t\t\t  DFS CAC time: %u ms\n",
							       nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_DFS_CAC_TIME]));
					}

				}
			}

			if (tb_band[NL80211_BAND_ATTR_RATES]) {
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
		}
	}

	if (tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS])
		printf("\tmax # scan SSIDs: %d\n",
		       nla_get_u8(tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]));
	if (tb_msg[NL80211_ATTR_MAX_SCAN_IE_LEN])
		printf("\tmax scan IEs length: %d bytes\n",
		       nla_get_u16(tb_msg[NL80211_ATTR_MAX_SCAN_IE_LEN]));
	if (tb_msg[NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS])
		printf("\tmax # sched scan SSIDs: %d\n",
		       nla_get_u8(tb_msg[NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS]));
	if (tb_msg[NL80211_ATTR_MAX_MATCH_SETS])
		printf("\tmax # match sets: %d\n",
		       nla_get_u8(tb_msg[NL80211_ATTR_MAX_MATCH_SETS]));

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

	if (tb_msg[NL80211_ATTR_WIPHY_RETRY_SHORT] ||
	    tb_msg[NL80211_ATTR_WIPHY_RETRY_LONG]) {
		unsigned char retry_short = 0, retry_long = 0;

		if (tb_msg[NL80211_ATTR_WIPHY_RETRY_SHORT])
			retry_short = nla_get_u8(tb_msg[NL80211_ATTR_WIPHY_RETRY_SHORT]);
		if (tb_msg[NL80211_ATTR_WIPHY_RETRY_LONG])
			retry_long = nla_get_u8(tb_msg[NL80211_ATTR_WIPHY_RETRY_LONG]);
		if (retry_short == retry_long) {
			printf("\tRetry short long limit: %d\n", retry_short);
		} else {
			printf("\tRetry short limit: %d\n", retry_short);
			printf("\tRetry long limit: %d\n", retry_long);
		}
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
				[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS] = { .type = NLA_U32 },
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

			printf("total <= %d, #channels <= %d%s",
				nla_get_u32(tb_comb[NL80211_IFACE_COMB_MAXNUM]),
				nla_get_u32(tb_comb[NL80211_IFACE_COMB_NUM_CHANNELS]),
				tb_comb[NL80211_IFACE_COMB_STA_AP_BI_MATCH] ?
					", STA/AP BI must match" : "");
			if (tb_comb[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]) {
				unsigned long widths = nla_get_u32(tb_comb[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]);

				if (widths) {
					int width;
					bool first = true;

					printf(", radar detect widths: {");
					for (width = 0; width < 32; width++)
						if (widths & (1 << width)) {
							printf("%s %s",
							       first ? "":",",
							       channel_width_name(width));
							first = false;
						}
					printf(" }\n");
				}
			}
			printf("\n");
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
			[NL80211_WOWLAN_TRIG_PKT_PATTERN] = { .minlen = 12 },
			[NL80211_WOWLAN_TRIG_GTK_REKEY_SUPPORTED] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_RFKILL_RELEASE] = { .type = NLA_FLAG },
			[NL80211_WOWLAN_TRIG_NET_DETECT] = { .type = NLA_U32 },
			[NL80211_WOWLAN_TRIG_TCP_CONNECTION] = { .type = NLA_NESTED },
		};
		struct nl80211_pattern_support *pat;
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
				printf("\t\t * wake up on pattern match, up to %u patterns of %u-%u bytes,\n"
					"\t\t   maximum packet offset %u bytes\n",
					pat->max_patterns, pat->min_pattern_len, pat->max_pattern_len,
					(nla_len(tb_wowlan[NL80211_WOWLAN_TRIG_PKT_PATTERN]) <
					sizeof(*pat)) ? 0 : pat->max_pkt_offset);
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
			if (tb_wowlan[NL80211_WOWLAN_TRIG_NET_DETECT])
				printf("\t\t * wake up on network detection, up to %d match sets\n",
				       nla_get_u32(tb_wowlan[NL80211_WOWLAN_TRIG_NET_DETECT]));
			if (tb_wowlan[NL80211_WOWLAN_TRIG_TCP_CONNECTION])
				printf("\t\t * wake up on TCP connection\n");
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
		unsigned int features = nla_get_u32(tb_msg[NL80211_ATTR_FEATURE_FLAGS]);

		if (features & NL80211_FEATURE_SK_TX_STATUS)
			printf("\tDevice supports TX status socket option.\n");
		if (features & NL80211_FEATURE_HT_IBSS)
			printf("\tDevice supports HT-IBSS.\n");
		if (features & NL80211_FEATURE_INACTIVITY_TIMER)
			printf("\tDevice has client inactivity timer.\n");
		if (features & NL80211_FEATURE_CELL_BASE_REG_HINTS)
			printf("\tDevice accepts cell base station regulatory hints.\n");
		if (features & NL80211_FEATURE_P2P_DEVICE_NEEDS_CHANNEL)
			printf("\tP2P Device uses a channel (of the concurrent ones)\n");
		if (features & NL80211_FEATURE_SAE)
			printf("\tDevice supports SAE with AUTHENTICATE command\n");
		if (features & NL80211_FEATURE_LOW_PRIORITY_SCAN)
			printf("\tDevice supports low priority scan.\n");
		if (features & NL80211_FEATURE_SCAN_FLUSH)
			printf("\tDevice supports scan flush.\n");
		if (features & NL80211_FEATURE_AP_SCAN)
			printf("\tDevice supports AP scan.\n");
		if (features & NL80211_FEATURE_VIF_TXPOWER)
			printf("\tDevice supports per-vif TX power setting\n");
		if (features & NL80211_FEATURE_NEED_OBSS_SCAN)
			printf("\tUserspace should do OBSS scan and generate 20/40 coex reports\n");
		if (features & NL80211_FEATURE_P2P_GO_CTWIN)
			printf("\tP2P GO supports CT window setting\n");
		if (features & NL80211_FEATURE_P2P_GO_OPPPS)
			printf("\tP2P GO supports opportunistic powersave setting\n");
		if (features & NL80211_FEATURE_FULL_AP_CLIENT_STATE)
			printf("\tDriver supports full state transitions for AP/GO clients\n");
		if (features & NL80211_FEATURE_USERSPACE_MPM)
			printf("\tDriver supports a userspace MPM\n");
		if (features & NL80211_FEATURE_ACTIVE_MONITOR)
			printf("\tDevice supports active monitor (which will ACK incoming frames)\n");
		if (features & NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE)
			printf("\tDriver/device bandwidth changes during BSS lifetime (AP/GO mode)\n");
		if (features & NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES)
			printf("\tDevice adds DS IE to probe requests\n");
		if (features & NL80211_FEATURE_WFA_TPC_IE_IN_PROBES)
			printf("\tDevice adds WFA TPC Report IE to probe requests\n");
		if (features & NL80211_FEATURE_QUIET)
			printf("\tDevice supports quiet requests from AP\n");
		if (features & NL80211_FEATURE_TX_POWER_INSERTION)
			printf("\tDevice can update TPC Report IE\n");
		if (features & NL80211_FEATURE_ACKTO_ESTIMATION)
			printf("\tDevice supports ACK timeout estimation.\n");
		if (features & NL80211_FEATURE_STATIC_SMPS)
			printf("\tDevice supports static SMPS\n");
		if (features & NL80211_FEATURE_DYNAMIC_SMPS)
			printf("\tDevice supports dynamic SMPS\n");
		if (features & NL80211_FEATURE_SUPPORTS_WMM_ADMISSION)
			printf("\tDevice supports WMM-AC admission (TSPECs)\n");
		if (features & NL80211_FEATURE_MAC_ON_CREATE)
			printf("\tDevice supports configuring vdev MAC-addr on create.\n");
		if (features & NL80211_FEATURE_TDLS_CHANNEL_SWITCH)
			printf("\tDevice supports TDLS channel switching\n");
	}

	if (tb_msg[NL80211_ATTR_EXT_FEATURES]) {
		struct nlattr *tb = tb_msg[NL80211_ATTR_EXT_FEATURES];

		if (ext_feature_isset(nla_data(tb), nla_len(tb),
				      NL80211_EXT_FEATURE_VHT_IBSS))
			printf("\tDevice supports VHT-IBSS.\n");
	}

	if (tb_msg[NL80211_ATTR_TDLS_SUPPORT])
		printf("\tDevice supports T-DLS.\n");

	if (tb_msg[NL80211_ATTR_COALESCE_RULE]) {
		struct nl80211_coalesce_rule_support *rule;
		struct nl80211_pattern_support *pat;

		printf("\tCoalesce support:\n");
		rule = nla_data(tb_msg[NL80211_ATTR_COALESCE_RULE]);
		pat = &rule->pat;
		printf("\t\t * Maximum %u coalesce rules supported\n"
		       "\t\t * Each rule contains upto %u patterns of %u-%u bytes,\n"
		       "\t\t   maximum packet offset %u bytes\n"
		       "\t\t * Maximum supported coalescing delay %u msecs\n",
			rule->max_rules, pat->max_patterns, pat->min_pattern_len,
			pat->max_pattern_len, pat->max_pkt_offset, rule->max_delay);
	}

	return NL_SKIP;
}

static bool nl80211_has_split_wiphy = false;

static int handle_info(struct nl80211_state *state,
		       struct nl_cb *cb,
		       struct nl_msg *msg,
		       int argc, char **argv,
		       enum id_input id)
{
	char *feat_args[] = { "features", "-q" };
	int err;

	err = handle_cmd(state, CIB_NONE, 2, feat_args);
	if (!err && nl80211_has_split_wiphy) {
		nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
		nlmsg_hdr(msg)->nlmsg_flags |= NLM_F_DUMP;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_phy_handler, NULL);

	return 0;
}
__COMMAND(NULL, info, "info", NULL, NL80211_CMD_GET_WIPHY, 0, 0, CIB_PHY, handle_info,
	 "Show capabilities for the specified wireless device.", NULL);
TOPLEVEL(list, NULL, NL80211_CMD_GET_WIPHY, NLM_F_DUMP, CIB_NONE, handle_info,
	 "List all wireless devices and their capabilities.");
TOPLEVEL(phy, NULL, NL80211_CMD_GET_WIPHY, NLM_F_DUMP, CIB_NONE, handle_info, NULL);

static int handle_commands(struct nl80211_state *state,
			   struct nl_cb *cb, struct nl_msg *msg,
			   int argc, char **argv, enum id_input id)
{
	int i;
	for (i = 1; i < NL80211_CMD_MAX; i++)
		printf("%d (0x%x): %s\n", i, i, command_name(i));
	/* don't send netlink messages */
	return 2;
}
TOPLEVEL(commands, NULL, NL80211_CMD_GET_WIPHY, 0, CIB_NONE, handle_commands,
	 "list all known commands and their decimal & hex value");

static int print_feature_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	bool print = (unsigned long)arg;
#define maybe_printf(...) do { if (print) printf(__VA_ARGS__); } while (0)

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (tb_msg[NL80211_ATTR_PROTOCOL_FEATURES]) {
		uint32_t feat = nla_get_u32(tb_msg[NL80211_ATTR_PROTOCOL_FEATURES]);

		maybe_printf("nl80211 features: 0x%x\n", feat);
		if (feat & NL80211_PROTOCOL_FEATURE_SPLIT_WIPHY_DUMP) {
			maybe_printf("\t* split wiphy dump\n");
			nl80211_has_split_wiphy = true;
		}
	}

	return NL_SKIP;
}

static int handle_features(struct nl80211_state *state,
			   struct nl_cb *cb, struct nl_msg *msg,
			   int argc, char **argv, enum id_input id)
{
	unsigned long print = argc == 0 || strcmp(argv[0], "-q");
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_feature_handler, (void *)print);
	return 0;
}

TOPLEVEL(features, "", NL80211_CMD_GET_PROTOCOL_FEATURES, 0, CIB_NONE,
	 handle_features, "");
