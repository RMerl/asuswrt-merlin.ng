#include <errno.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

static int iw_conn(struct nl80211_state *state, struct nl_cb *cb,
		   struct nl_msg *msg, int argc, char **argv,
		   enum id_input id)
{
	char *end;
	unsigned char bssid[6];
	int freq;

	if (argc < 1)
		return 1;

	/* SSID */
	NLA_PUT(msg, NL80211_ATTR_SSID, strlen(argv[0]), argv[0]);
	argv++;
	argc--;

	/* freq */
	if (argc) {
		freq = strtoul(argv[0], &end, 10);
		if (*end == '\0') {
			NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
			argv++;
			argc--;
		}
	}

	/* bssid */
	if (argc) {
		if (mac_addr_a2n(bssid, argv[0]) == 0) {
			NLA_PUT(msg, NL80211_ATTR_MAC, 6, bssid);
			argv++;
			argc--;
		}
	}

	if (!argc)
		return 0;

	if (strcmp(*argv, "key") != 0 && strcmp(*argv, "keys") != 0)
		return 1;

	argv++;
	argc--;

	return parse_keys(msg, argv, argc);
 nla_put_failure:
	return -ENOSPC;
}

static int disconnect(struct nl80211_state *state,
		      struct nl_cb *cb,
		      struct nl_msg *msg,
		      int argc, char **argv,
		      enum id_input id)
{
	return 0;
}
TOPLEVEL(disconnect, NULL,
	NL80211_CMD_DISCONNECT, 0, CIB_NETDEV, disconnect,
	"Disconnect from the current network.");

static int iw_connect(struct nl80211_state *state, struct nl_cb *cb,
		      struct nl_msg *msg, int argc, char **argv,
		      enum id_input id)
{
	char **conn_argv, *dev = argv[0];
	static const __u32 cmds[] = {
		NL80211_CMD_CONNECT,
	};
	struct print_event_args printargs = { };
	int conn_argc, err;
	bool wait = false;
	int i;

	/* strip "wlan0 connect" */
	argc -= 2;
	argv += 2;

	/* check -w */
	if (argc && strcmp(argv[0], "-w") == 0) {
		wait = true;
		argc--;
		argv++;
	}

	conn_argc = 3 + argc;
	conn_argv = calloc(conn_argc, sizeof(*conn_argv));
	if (!conn_argv)
		return -ENOMEM;

	err = __prepare_listen_events(state);
	if (err)
		return err;

	conn_argv[0] = dev;
	conn_argv[1] = "connect";
	conn_argv[2] = "establish";
	for (i = 0; i < argc; i++)
		conn_argv[i + 3] = argv[i];
	err = handle_cmd(state, id, conn_argc, conn_argv);
	free(conn_argv);
	if (err)
		return err;

	if (!wait)
		return 0;

	/*
	 * WARNING: DO NOT COPY THIS CODE INTO YOUR APPLICATION
	 *
	 * This code has a bug:
	 *
	 * It is possible for a connect result message from another
	 * connect attempt to be processed here first, because we
	 * start listening to the multicast group before starting
	 * our own connect request, which may succeed but we get a
	 * fail message from a previous attempt that raced with us,
	 * or similar.
	 *
	 * The only proper way to fix this would be to listen to events
	 * before sending the command, and for the kernel to send the
	 * connect request or a cookie along with the event, so that you
	 * can match up whether the connect _you_ requested was finished
	 * or aborted.
	 *
	 * Alas, the kernel doesn't do that (yet).
	 */

	__do_listen_events(state, ARRAY_SIZE(cmds), cmds, &printargs);
	return 0;
}
TOPLEVEL(connect, "[-w] <SSID> [<freq in MHz>] [<bssid>] [key 0:abcde d:1:6162636465]",
	0, 0, CIB_NETDEV, iw_connect,
	"Join the network with the given SSID (and frequency, BSSID).\n"
	"With -w, wait for the connect to finish or fail.");
HIDDEN(connect, establish, "", NL80211_CMD_CONNECT, 0, CIB_NETDEV, iw_conn);
