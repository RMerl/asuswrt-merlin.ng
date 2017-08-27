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

/* These enums need to be kept in sync with the kernel */
enum hwsim_testmode_attr {
	__HWSIM_TM_ATTR_INVALID	= 0,
	HWSIM_TM_ATTR_CMD	= 1,
	HWSIM_TM_ATTR_PS	= 2,

	/* keep last */
	__HWSIM_TM_ATTR_AFTER_LAST,
	HWSIM_TM_ATTR_MAX	= __HWSIM_TM_ATTR_AFTER_LAST - 1
};

enum hwsim_testmode_cmd {
	HWSIM_TM_CMD_SET_PS		= 0,
	HWSIM_TM_CMD_GET_PS		= 1,
	HWSIM_TM_CMD_STOP_QUEUES	= 2,
	HWSIM_TM_CMD_WAKE_QUEUES	= 3,
};


SECTION(hwsim);

static int print_hwsim_ps_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[NL80211_ATTR_MAX + 1];
	struct nlattr *tb[HWSIM_TM_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(attrs, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[NL80211_ATTR_TESTDATA])
		return NL_SKIP;

	nla_parse(tb, HWSIM_TM_ATTR_MAX, nla_data(attrs[NL80211_ATTR_TESTDATA]),
		  nla_len(attrs[NL80211_ATTR_TESTDATA]), NULL);

	printf("HWSIM PS: %d\n", nla_get_u32(tb[HWSIM_TM_ATTR_PS]));

	return NL_SKIP;
}

static int handle_hwsim_getps(struct nl80211_state *state, struct nl_cb *cb,
			      struct nl_msg *msg, int argc, char **argv,
			      enum id_input id)
{
	struct nlattr *tmdata;

	tmdata = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
	if (!tmdata)
		goto nla_put_failure;

	NLA_PUT_U32(msg, HWSIM_TM_ATTR_CMD, HWSIM_TM_CMD_GET_PS);

	nla_nest_end(msg, tmdata);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
		  print_hwsim_ps_handler, NULL);
	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(hwsim, getps, "", NL80211_CMD_TESTMODE, 0, CIB_PHY, handle_hwsim_getps, "");

static int handle_hwsim_setps(struct nl80211_state *state, struct nl_cb *cb,
			      struct nl_msg *msg, int argc, char **argv,
			      enum id_input id)
{
	struct nlattr *tmdata;
	__u32 ps;
	char *end;

	if (argc != 1)
		return 1;

	ps = strtoul(argv[0], &end, 0);
	if (*end)
		return 1;

	tmdata = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
	if (!tmdata)
		goto nla_put_failure;

	NLA_PUT_U32(msg, HWSIM_TM_ATTR_CMD, HWSIM_TM_CMD_SET_PS);
	NLA_PUT_U32(msg, HWSIM_TM_ATTR_PS, ps);

	nla_nest_end(msg, tmdata);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
		  print_hwsim_ps_handler, NULL);
	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(hwsim, setps, "<value>", NL80211_CMD_TESTMODE, 0, CIB_PHY, handle_hwsim_setps, "");

static int handle_hwsim_stop_queues(struct nl80211_state *state, struct nl_cb *cb,
				    struct nl_msg *msg, int argc, char **argv,
				    enum id_input id)
{
	struct nlattr *tmdata;

	if (argc != 0)
		return 1;

	tmdata = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
	if (!tmdata)
		goto nla_put_failure;

	NLA_PUT_U32(msg, HWSIM_TM_ATTR_CMD, HWSIM_TM_CMD_STOP_QUEUES);

	nla_nest_end(msg, tmdata);
	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(hwsim, stopqueues, "", NL80211_CMD_TESTMODE, 0, CIB_PHY, handle_hwsim_stop_queues, "");

static int handle_hwsim_wake_queues(struct nl80211_state *state, struct nl_cb *cb,
				    struct nl_msg *msg, int argc, char **argv,
				    enum id_input id)
{
	struct nlattr *tmdata;

	if (argc != 0)
		return 1;

	tmdata = nla_nest_start(msg, NL80211_ATTR_TESTDATA);
	if (!tmdata)
		goto nla_put_failure;

	NLA_PUT_U32(msg, HWSIM_TM_ATTR_CMD, HWSIM_TM_CMD_WAKE_QUEUES);

	nla_nest_end(msg, tmdata);
	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
COMMAND(hwsim, wakequeues, "", NL80211_CMD_TESTMODE, 0, CIB_PHY, handle_hwsim_wake_queues, "");
