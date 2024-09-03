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

SECTION(roc);

static int handle_roc_start(struct nl80211_state *state, struct nl_cb *cb,
			    struct nl_msg *msg, int argc, char **argv,
			    enum id_input id)
{
	char *end;
	int freq, time;

	if (argc != 2)
		return 1;

	freq = strtol(argv[0], &end, 0);
	if (!end || *end)
		return 1;

	time = strtol(argv[1], &end, 0);
	if (!end || *end)
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	NLA_PUT_U32(msg, NL80211_ATTR_DURATION, time);
	return 0;
 nla_put_failure:
	return -ENOBUFS;
}

COMMAND(roc, start, "<freq> <time in ms>", NL80211_CMD_REMAIN_ON_CHANNEL, 0, CIB_NETDEV, handle_roc_start, "");
