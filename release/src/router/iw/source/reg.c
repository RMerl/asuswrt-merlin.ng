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

SECTION(reg);

#define MHZ_TO_KHZ(freq) ((freq) * 1000)
#define KHZ_TO_MHZ(freq) ((freq) / 1000)
#define DBI_TO_MBI(gain) ((gain) * 100)
#define MBI_TO_DBI(gain) ((gain) / 100)
#define DBM_TO_MBM(gain) ((gain) * 100)
#define MBM_TO_DBM(gain) ((gain) / 100)

static bool isalpha_upper(char letter)
{
	if (letter >= 65 && letter <= 90)
		return true;
	return false;
}

static bool is_alpha2(char *alpha2)
{
	if (isalpha_upper(alpha2[0]) && isalpha_upper(alpha2[1]))
		return true;
	return false;
}

static bool is_world_regdom(char *alpha2)
{
	/* ASCII 0 */
	if (alpha2[0] == 48 && alpha2[1] == 48)
		return true;
	return false;
}

char *reg_initiator_to_string(__u8 initiator)
{
	switch (initiator) {
	case NL80211_REGDOM_SET_BY_CORE:
		return "the wireless core upon initialization";
	case NL80211_REGDOM_SET_BY_USER:
		return "a user";
	case NL80211_REGDOM_SET_BY_DRIVER:
		return "a driver";
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		return "a country IE";
	default:
		return "BUG";
	}
}

static int handle_reg_set(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	char alpha2[3];

	if (argc < 1)
		return 1;

	if (!is_alpha2(argv[0]) && !is_world_regdom(argv[0])) {
		fprintf(stderr, "not a valid ISO/IEC 3166-1 alpha2\n");
		fprintf(stderr, "Special non-alpha2 usable entries:\n");
		fprintf(stderr, "\t00\tWorld Regulatory domain\n");
		return 2;
	}

	alpha2[0] = argv[0][0];
	alpha2[1] = argv[0][1];
	alpha2[2] = '\0';

	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT_STRING(msg, NL80211_ATTR_REG_ALPHA2, alpha2);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(reg, set, "<ISO/IEC 3166-1 alpha2>",
	NL80211_CMD_REQ_SET_REG, 0, CIB_NONE, handle_reg_set,
	"Notify the kernel about the current regulatory domain.");

static int print_reg_handler(struct nl_msg *msg, void *arg)

{
#define PARSE_FLAG(nl_flag, string_value)  do { \
		if ((flags & nl_flag)) { \
			printf(", %s", string_value); \
		} \
	} while (0)
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *alpha2;
	struct nlattr *nl_rule;
	int rem_rule;
	static struct nla_policy reg_rule_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_ATTR_REG_RULE_FLAGS]		= { .type = NLA_U32 },
		[NL80211_ATTR_FREQ_RANGE_START]		= { .type = NLA_U32 },
		[NL80211_ATTR_FREQ_RANGE_END]		= { .type = NLA_U32 },
		[NL80211_ATTR_FREQ_RANGE_MAX_BW]	= { .type = NLA_U32 },
		[NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN]	= { .type = NLA_U32 },
		[NL80211_ATTR_POWER_RULE_MAX_EIRP]	= { .type = NLA_U32 },
	};

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb_msg[NL80211_ATTR_REG_ALPHA2]) {
		printf("No alpha2\n");
		return NL_SKIP;
	}

	if (!tb_msg[NL80211_ATTR_REG_RULES]) {
		printf("No reg rules\n");
		return NL_SKIP;
	}

	alpha2 = nla_data(tb_msg[NL80211_ATTR_REG_ALPHA2]);
	printf("country %c%c:\n", alpha2[0], alpha2[1]);

	nla_for_each_nested(nl_rule, tb_msg[NL80211_ATTR_REG_RULES], rem_rule) {
		struct nlattr *tb_rule[NL80211_FREQUENCY_ATTR_MAX + 1];
		__u32 flags, start_freq_khz, end_freq_khz, max_bw_khz, max_ant_gain_mbi, max_eirp_mbm;

		nla_parse(tb_rule, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_rule), nla_len(nl_rule), reg_rule_policy);

		flags = nla_get_u32(tb_rule[NL80211_ATTR_REG_RULE_FLAGS]);
		start_freq_khz = nla_get_u32(tb_rule[NL80211_ATTR_FREQ_RANGE_START]);
		end_freq_khz = nla_get_u32(tb_rule[NL80211_ATTR_FREQ_RANGE_END]);
		max_bw_khz = nla_get_u32(tb_rule[NL80211_ATTR_FREQ_RANGE_MAX_BW]);
		max_ant_gain_mbi = nla_get_u32(tb_rule[NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN]);
		max_eirp_mbm = nla_get_u32(tb_rule[NL80211_ATTR_POWER_RULE_MAX_EIRP]);


		printf("\t(%d - %d @ %d), (",
			KHZ_TO_MHZ(start_freq_khz), KHZ_TO_MHZ(end_freq_khz), KHZ_TO_MHZ(max_bw_khz));

		if (MBI_TO_DBI(max_ant_gain_mbi))
			printf("%d", MBI_TO_DBI(max_ant_gain_mbi));
		else
			printf("N/A");

		printf(", %d)", MBM_TO_DBM(max_eirp_mbm));

		if (!flags) {
			printf("\n");
			continue;
		}

		/* Sync this output format to match that of dbparse.py from wireless-regdb.git */
		PARSE_FLAG(NL80211_RRF_NO_OFDM, "NO-OFDM");
		PARSE_FLAG(NL80211_RRF_NO_CCK, "NO-CCK");
		PARSE_FLAG(NL80211_RRF_NO_INDOOR, "NO-INDOOR");
		PARSE_FLAG(NL80211_RRF_NO_OUTDOOR, "NO-OUTDOOR");
		PARSE_FLAG(NL80211_RRF_DFS, "DFS");
		PARSE_FLAG(NL80211_RRF_PTP_ONLY, "PTP-ONLY");
		PARSE_FLAG(NL80211_RRF_PASSIVE_SCAN, "PASSIVE-SCAN");
		PARSE_FLAG(NL80211_RRF_NO_IBSS, "NO-IBSS");

		printf("\n");
	}
	return NL_OK;
#undef PARSE_FLAG
}

static int handle_reg_get(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_reg_handler, NULL);
	return 0;
}
COMMAND(reg, get, NULL, NL80211_CMD_GET_REG, 0, CIB_NONE, handle_reg_get,
	"Print out the kernel's current regulatory domain information.");
