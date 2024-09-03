#include <errno.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static int offchannel(struct nl80211_state *state, struct nl_cb *cb,
		      struct nl_msg *msg, int argc, char **argv,
		      enum id_input id)
{
	char *end;

	/* freq */
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ,
		    strtoul(argv[0], &end, 10));
	if (*end != '\0')
		return 1;
	argv++;
	argc--;

	/* duration */
	NLA_PUT_U32(msg, NL80211_ATTR_DURATION,
		    strtoul(argv[0], &end, 10));
	if (*end != '\0')
		return 1;
	argv++;
	argc--;

	if (argc)
		return 1;

	return 0;
 nla_put_failure:
	return -ENOSPC;
}

TOPLEVEL(offchannel, "<freq> <duration>", NL80211_CMD_REMAIN_ON_CHANNEL, 0,
	 CIB_NETDEV, offchannel,
	 "Leave operating channel and go to the given channel for a while.");
