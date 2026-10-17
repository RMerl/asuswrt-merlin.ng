#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static int set_power_save(struct nl80211_state *state,
			  struct nl_cb *cb,
			  struct nl_msg *msg,
			  int argc, char **argv,
			  enum id_input id)
{
	enum nl80211_ps_state ps_state;

	if (argc != 1) {
		printf("Invalid parameters!\n");
		return 2;
	}

	if (strcmp(argv[0], "on") == 0)
		ps_state = NL80211_PS_ENABLED;
	else if (strcmp(argv[0], "off") == 0)
		ps_state = NL80211_PS_DISABLED;
	else {
		printf("Invalid parameter: %s\n", argv[0]);
		return 2;
	}

	NLA_PUT_U32(msg, NL80211_ATTR_PS_STATE, ps_state);

	return 0;

 nla_put_failure:
	return -ENOBUFS;
}

COMMAND(set, power_save, "<on|off>",
	NL80211_CMD_SET_POWER_SAVE, 0, CIB_NETDEV, set_power_save,
	"Set power save state to on or off.");

static int print_power_save_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	const char *s;

	nla_parse(attrs, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[NL80211_ATTR_PS_STATE])
		return NL_SKIP;

	switch (nla_get_u32(attrs[NL80211_ATTR_PS_STATE])) {
	case NL80211_PS_ENABLED:
		s = "on";
		break;
	case NL80211_PS_DISABLED:
	default:
		s = "off";
		break;
	}

	printf("Power save: %s\n", s);

	return NL_SKIP;
}

static int get_power_save(struct nl80211_state *state,
				   struct nl_cb *cb,
				   struct nl_msg *msg,
				   int argc, char **argv,
				   enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
		  print_power_save_handler, NULL);
	return 0;
}

COMMAND(get, power_save, "<param>",
	NL80211_CMD_GET_POWER_SAVE, 0, CIB_NETDEV, get_power_save,
	"Retrieve power save state.");
