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

SECTION(survey);

static int print_survey_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	char dev[20];

	static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
		[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Survey data from %s\n", dev);

	if (!tb[NL80211_ATTR_SURVEY_INFO]) {
		fprintf(stderr, "survey data missing!\n");
		return NL_SKIP;
	}

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
			     tb[NL80211_ATTR_SURVEY_INFO],
			     survey_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if (sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		printf("\tfrequency:\t\t\t%u MHz%s\n",
			nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]),
			sinfo[NL80211_SURVEY_INFO_IN_USE] ? " [in use]" : "");
	if (sinfo[NL80211_SURVEY_INFO_NOISE])
		printf("\tnoise:\t\t\t\t%d dBm\n",
			(int8_t)nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]));
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME])
		printf("\tchannel active time:\t\t%llu ms\n",
			(unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]));
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY])
		printf("\tchannel busy time:\t\t%llu ms\n",
			(unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]));
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY])
		printf("\textension channel busy time:\t%llu ms\n",
			(unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY]));
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX])
		printf("\tchannel receive time:\t\t%llu ms\n",
			(unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX]));
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX])
		printf("\tchannel transmit time:\t\t%llu ms\n",
			(unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX]));
	return NL_SKIP;
}

static int handle_survey_dump(struct nl80211_state *state,
			      struct nl_cb *cb,
			      struct nl_msg *msg,
			      int argc, char **argv,
			      enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_survey_handler, NULL);
	return 0;
}
COMMAND(survey, dump, NULL,
	NL80211_CMD_GET_SURVEY, NLM_F_DUMP, CIB_NETDEV, handle_survey_dump,
	"List all gathered channel survey data");

