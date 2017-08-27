#include <errno.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static int iw_cqm_rssi(struct nl80211_state *state, struct nl_cb *cb,
		       struct nl_msg *msg, int argc, char **argv,
		       enum id_input id)
{
	struct nl_msg *cqm = NULL;
	int thold = 0;
	int hyst = 0;
	int ret = -ENOSPC;

	/* get the required args */
	if (argc < 1 || argc > 2)
		return 1;

	if (strcmp(argv[0], "off")) {
		thold = atoi(argv[0]);

		if (thold == 0)
			return -EINVAL;

		if (argc == 2)
			hyst = atoi(argv[1]);
	}

	/* connection quality monitor attributes */
	cqm = nlmsg_alloc();

	NLA_PUT_U32(cqm, NL80211_ATTR_CQM_RSSI_THOLD, thold);
	NLA_PUT_U32(cqm, NL80211_ATTR_CQM_RSSI_HYST, hyst);

	nla_put_nested(msg, NL80211_ATTR_CQM, cqm);
	ret = 0;

 nla_put_failure:
	nlmsg_free(cqm);
	return ret;
}

TOPLEVEL(cqm, "",
	 0, 0, CIB_NETDEV, NULL,
	 "Configure the WLAN connection quality monitor.\n");

COMMAND(cqm, rssi, "<threshold|off> [<hysteresis>]",
	NL80211_CMD_SET_CQM, 0, CIB_NETDEV, iw_cqm_rssi,
	"Set connection quality monitor RSSI threshold.\n");
