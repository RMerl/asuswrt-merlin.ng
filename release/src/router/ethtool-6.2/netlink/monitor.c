/*
 * monitor.c - netlink notification monitor
 *
 * Implementation of "ethtool --monitor" for watching netlink notifications.
 */

#include <errno.h>

#include "../internal.h"
#include "netlink.h"
#include "nlsock.h"
#include "strset.h"

static struct {
	uint8_t		cmd;
	mnl_cb_t	cb;
} monitor_callbacks[] = {
	{
		.cmd	= ETHTOOL_MSG_LINKMODES_NTF,
		.cb	= linkmodes_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_LINKINFO_NTF,
		.cb	= linkinfo_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_WOL_NTF,
		.cb	= wol_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_DEBUG_NTF,
		.cb	= debug_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_FEATURES_NTF,
		.cb	= features_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_PRIVFLAGS_NTF,
		.cb	= privflags_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_RINGS_NTF,
		.cb	= rings_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_CHANNELS_NTF,
		.cb	= channels_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_COALESCE_NTF,
		.cb	= coalesce_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_PAUSE_NTF,
		.cb	= pause_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_EEE_NTF,
		.cb	= eee_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_CABLE_TEST_NTF,
		.cb	= cable_test_ntf_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_CABLE_TEST_TDR_NTF,
		.cb	= cable_test_tdr_ntf_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_FEC_NTF,
		.cb	= fec_reply_cb,
	},
	{
		.cmd	= ETHTOOL_MSG_MODULE_NTF,
		.cb	= module_reply_cb,
	},
};

static void clear_filter(struct nl_context *nlctx)
{
	unsigned int i;

	for (i = 0; i < CMDMASK_WORDS; i++)
		nlctx->filter_cmds[i] = 0;
}

static bool test_filter_cmd(const struct nl_context *nlctx, unsigned int cmd)
{
	return nlctx->filter_cmds[cmd / 32] & (1U << (cmd % 32));
}

static void set_filter_cmd(struct nl_context *nlctx, unsigned int cmd)
{
	nlctx->filter_cmds[cmd / 32] |= (1U << (cmd % 32));
}

static void set_filter_all(struct nl_context *nlctx)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(monitor_callbacks); i++)
		set_filter_cmd(nlctx, monitor_callbacks[i].cmd);
}

static int monitor_any_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct genlmsghdr *ghdr = (const struct genlmsghdr *)(nlhdr + 1);
	struct nl_context *nlctx = data;
	unsigned int i;

	if (!test_filter_cmd(nlctx, ghdr->cmd))
		return MNL_CB_OK;

	for (i = 0; i < MNL_ARRAY_SIZE(monitor_callbacks); i++)
		if (monitor_callbacks[i].cmd == ghdr->cmd)
			return monitor_callbacks[i].cb(nlhdr, data);

	return MNL_CB_OK;
}

struct monitor_option {
	const char	*pattern;
	uint8_t		cmd;
	uint32_t	info_mask;
};

static struct monitor_option monitor_opts[] = {
	{
		.pattern	= "|--all",
		.cmd		= 0,
	},
	{
		.pattern	= "-s|--change",
		.cmd		= ETHTOOL_MSG_LINKINFO_NTF,
	},
	{
		.pattern	= "-s|--change",
		.cmd		= ETHTOOL_MSG_LINKMODES_NTF,
	},
	{
		.pattern	= "-s|--change",
		.cmd		= ETHTOOL_MSG_WOL_NTF,
	},
	{
		.pattern	= "-s|--change",
		.cmd		= ETHTOOL_MSG_DEBUG_NTF,
	},
	{
		.pattern	= "-k|--show-features|--show-offload|-K|--features|--offload",
		.cmd		= ETHTOOL_MSG_FEATURES_NTF,
	},
	{
		.pattern	= "--show-priv-flags|--set-priv-flags",
		.cmd		= ETHTOOL_MSG_PRIVFLAGS_NTF,
	},
	{
		.pattern	= "-g|--show-ring|-G|--set-ring",
		.cmd		= ETHTOOL_MSG_RINGS_NTF,
	},
	{
		.pattern	= "-l|--show-channels|-L|--set-channels",
		.cmd		= ETHTOOL_MSG_CHANNELS_NTF,
	},
	{
		.pattern	= "-c|--show-coalesce|-C|--coalesce",
		.cmd		= ETHTOOL_MSG_COALESCE_NTF,
	},
	{
		.pattern	= "-a|--show-pause|-A|--pause",
		.cmd		= ETHTOOL_MSG_PAUSE_NTF,
	},
	{
		.pattern	= "--show-eee|--set-eee",
		.cmd		= ETHTOOL_MSG_EEE_NTF,
	},
	{
		.pattern	= "--cable-test",
		.cmd		= ETHTOOL_MSG_CABLE_TEST_NTF,
	},
	{
		.pattern	= "--cable-test-tdr",
		.cmd		= ETHTOOL_MSG_CABLE_TEST_TDR_NTF,
	},
	{
		.pattern	= "--show-module|--set-module",
		.cmd		= ETHTOOL_MSG_MODULE_NTF,
	},
};

static bool pattern_match(const char *s, const char *pattern)
{
	const char *opt = pattern;
	const char *next;
	int slen = strlen(s);
	int optlen;

	do {
		next = opt;
		while (*next && *next != '|')
			next++;
		optlen = next - opt;
		if (slen == optlen && !strncmp(s, opt, optlen))
			return true;

		opt = next;
		if (*opt == '|')
			opt++;
	} while (*opt);

	return false;
}

static int parse_monitor(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	char **argp = ctx->argp;
	int argc = ctx->argc;
	const char *opt = "";
	bool opt_found;
	unsigned int i;

	if (*argp && argp[0][0] == '-') {
		opt = *argp;
		argp++;
		argc--;
	}
	opt_found = false;
	clear_filter(nlctx);
	for (i = 0; i < MNL_ARRAY_SIZE(monitor_opts); i++) {
		if (pattern_match(opt, monitor_opts[i].pattern)) {
			unsigned int cmd = monitor_opts[i].cmd;

			if (!cmd)
				set_filter_all(nlctx);
			else
				set_filter_cmd(nlctx, cmd);
			opt_found = true;
		}
	}
	if (!opt_found) {
		fprintf(stderr, "monitoring for option '%s' not supported\n",
			*argp);
		return -1;
	}

	if (*argp && strcmp(*argp, WILDCARD_DEVNAME))
		ctx->devname = *argp;
	return 0;
}

int nl_monitor(struct cmd_context *ctx)
{
	struct nl_context *nlctx;
	struct nl_socket *nlsk;
	uint32_t grpid;
	bool is_dev;
	int ret;

	ret = netlink_init(ctx);
	if (ret < 0) {
		fprintf(stderr, "Netlink interface initialization failed, option --monitor not supported.\n");
		return ret;
	}
	nlctx = ctx->nlctx;
	nlsk = nlctx->ethnl_socket;
	grpid = nlctx->ethnl_mongrp;
	if (!grpid) {
		fprintf(stderr, "multicast group 'monitor' not found\n");
		return -EOPNOTSUPP;
	}

	if (parse_monitor(ctx) < 0)
		return 1;
	is_dev = ctx->devname && strcmp(ctx->devname, WILDCARD_DEVNAME);

	ret = preload_global_strings(nlsk);
	if (ret < 0)
		return ret;
	ret = mnl_socket_setsockopt(nlsk->sk, NETLINK_ADD_MEMBERSHIP,
				    &grpid, sizeof(grpid));
	if (ret < 0)
		return ret;
	if (is_dev) {
		ret = preload_perdev_strings(nlsk, ctx->devname);
		if (ret < 0)
			goto out_strings;
	}

	nlctx->filter_devname = ctx->devname;
	nlctx->is_monitor = true;
	nlsk->port = 0;
	nlsk->seq = 0;

	fputs("listening...\n", stdout);
	fflush(stdout);
	ret = nlsock_process_reply(nlsk, monitor_any_cb, nlctx);

out_strings:
	cleanup_all_strings();
	return ret;
}

void nl_monitor_usage(void)
{
	unsigned int i;
	const char *p;

	fputs("        ethtool --monitor               Show kernel notifications\n",
	      stdout);
	fputs("                ( [ --all ]", stdout);
	for (i = 1; i < MNL_ARRAY_SIZE(monitor_opts); i++) {
		if (!strcmp(monitor_opts[i].pattern, monitor_opts[i - 1].pattern))
			continue;
		fputs("\n                  | ", stdout);
		for (p = monitor_opts[i].pattern; *p; p++)
			if (*p == '|')
				fputs(" | ", stdout);
			else
				fputc(*p, stdout);
	}
	fputs(" )\n", stdout);
	fputs("                [ DEVNAME | * ]\n", stdout);
}
