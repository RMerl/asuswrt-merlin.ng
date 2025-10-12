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

SECTION(p2p);

static int handle_p2p_start(struct nl80211_state *state, struct nl_cb *cb,
			    struct nl_msg *msg, int argc, char **argv,
			    enum id_input id)
{
	return 0;
}
COMMAND(p2p, start, "", NL80211_CMD_START_P2P_DEVICE, 0, CIB_WDEV, handle_p2p_start, "");

static int handle_p2p_stop(struct nl80211_state *state, struct nl_cb *cb,
			   struct nl_msg *msg, int argc, char **argv,
			   enum id_input id)
{
	return 0;
}
COMMAND(p2p, stop, "", NL80211_CMD_STOP_P2P_DEVICE, 0, CIB_WDEV, handle_p2p_stop, "");
