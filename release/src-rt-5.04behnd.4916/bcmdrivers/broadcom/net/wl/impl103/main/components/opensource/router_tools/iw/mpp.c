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

SECTION(mpp);

static int print_mpp_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char dst[20], next_hop[20], dev[20];

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending mpath notifications.
	 */

	mac_addr_n2a(dst, nla_data(tb[NL80211_ATTR_MAC]));
	mac_addr_n2a(next_hop, nla_data(tb[NL80211_ATTR_MPATH_NEXT_HOP]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("%s %s %s\n", dst, next_hop, dev);

	return NL_SKIP;
}

static int handle_mpp_get(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	unsigned char dst[ETH_ALEN];

	if (argc < 1)
		return 1;

	if (mac_addr_a2n(dst, argv[0])) {
		fprintf(stderr, "invalid mac address\n");
		return 2;
	}
	argc--;
	argv++;

	if (argc)
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, dst);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_mpp_handler, NULL);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(mpp, get, "<MAC address>",
	NL80211_CMD_GET_MPP, 0, CIB_NETDEV, handle_mpp_get,
	"Get information on mesh proxy path to the given node.");

static int handle_mpp_dump(struct nl80211_state *state,
			     struct nl_cb *cb,
			     struct nl_msg *msg,
			     int argc, char **argv,
			     enum id_input id)
{
	printf("DEST ADDR         PROXY NODE        IFACE\n");
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_mpp_handler, NULL);
	return 0;
}
COMMAND(mpp, dump, NULL,
	NL80211_CMD_GET_MPP, NLM_F_DUMP, CIB_NETDEV, handle_mpp_dump,
	"List known mesh proxy paths.");
