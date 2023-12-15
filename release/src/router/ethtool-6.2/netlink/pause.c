/*
 * pause.c - netlink implementation of pause commands
 *
 * Implementation of "ethtool -a <dev>" and "ethtool -A <dev> ..."
 */

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "bitset.h"
#include "parser.h"

/* PAUSE_GET */

struct pause_autoneg_status {
	bool	pause;
	bool	asym_pause;
};

static void pause_autoneg_walker(unsigned int idx,
				 const char *name __maybe_unused, bool val,
				 void *data)
{
	struct pause_autoneg_status *status = data;

	if (idx == ETHTOOL_LINK_MODE_Pause_BIT)
		status->pause = val;
	if (idx == ETHTOOL_LINK_MODE_Asym_Pause_BIT)
		status->asym_pause = val;
}

static int pause_autoneg_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_LINKMODES_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct pause_autoneg_status ours = {};
	struct pause_autoneg_status peer = {};
	struct nl_context *nlctx = data;
	uint8_t rx_status = false;
	uint8_t tx_status = false;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;

	if (!tb[ETHTOOL_A_LINKMODES_OURS] || !tb[ETHTOOL_A_LINKMODES_PEER])
		return MNL_CB_OK;
	ret = walk_bitset(tb[ETHTOOL_A_LINKMODES_OURS], NULL,
			  pause_autoneg_walker, &ours);
	if (ret < 0)
		return err_ret;
	ret = walk_bitset(tb[ETHTOOL_A_LINKMODES_PEER], NULL,
			  pause_autoneg_walker, &peer);
	if (ret < 0)
		return err_ret;

	if (ours.pause && peer.pause) {
		rx_status = true;
		tx_status = true;
	} else if (ours.asym_pause && peer.asym_pause) {
		if (ours.pause)
			rx_status = true;
		else if (peer.pause)
			tx_status = true;
	}

	open_json_object("negotiated");
	show_bool_val("rx", "RX negotiated: %s\n", &rx_status);
	show_bool_val("tx", "TX negotiated: %s\n", &tx_status);
	close_json_object();

	return MNL_CB_OK;
}

static int show_pause_autoneg_status(struct nl_context *nlctx)
{
	const char *saved_devname;
	int ret;

	saved_devname = nlctx->ctx->devname;
	nlctx->ctx->devname = nlctx->devname;
	ret = netlink_init_ethnl2_socket(nlctx);
	if (ret < 0)
		goto out;

	ret = nlsock_prep_get_request(nlctx->ethnl2_socket,
				      ETHTOOL_MSG_LINKMODES_GET,
				      ETHTOOL_A_LINKMODES_HEADER,
				      ETHTOOL_FLAG_COMPACT_BITSETS);
	if (ret < 0)
		goto out;
	ret = nlsock_send_get_request(nlctx->ethnl2_socket, pause_autoneg_cb);

out:
	nlctx->ctx->devname = saved_devname;
	return ret;
}

static int show_pause_stats(const struct nlattr *nest)
{
	const struct nlattr *tb[ETHTOOL_A_PAUSE_STAT_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	static const struct {
		unsigned int attr;
		char *name;
	} stats[] = {
		{ ETHTOOL_A_PAUSE_STAT_TX_FRAMES, "tx_pause_frames" },
		{ ETHTOOL_A_PAUSE_STAT_RX_FRAMES, "rx_pause_frames" },
	};
	bool header = false;
	unsigned int i;
	size_t n;
	int ret;

	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	if (ret < 0)
		return ret;

	open_json_object("statistics");
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		char fmt[32];

		if (!tb[stats[i].attr])
			continue;

		if (!header && !is_json_context()) {
			printf("Statistics:\n");
			header = true;
		}

		if (mnl_attr_validate(tb[stats[i].attr], MNL_TYPE_U64)) {
			fprintf(stderr, "malformed netlink message (statistic)\n");
			goto err_close_stats;
		}

		n = snprintf(fmt, sizeof(fmt), "  %s: %%" PRIu64 "\n",
			     stats[i].name);
		if (n >= sizeof(fmt)) {
			fprintf(stderr, "internal error - malformed label\n");
			goto err_close_stats;
		}

		print_u64(PRINT_ANY, stats[i].name, fmt,
			  mnl_attr_get_u64(tb[stats[i].attr]));
	}
	close_json_object();

	return 0;

err_close_stats:
	close_json_object();
	return -1;
}

int pause_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PAUSE_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_PAUSE_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "Pause parameters for %s:\n",
		     nlctx->devname);

	show_bool("autonegotiate", "Autonegotiate:\t%s\n",
		  tb[ETHTOOL_A_PAUSE_AUTONEG]);
	show_bool("rx", "RX:\t\t%s\n", tb[ETHTOOL_A_PAUSE_RX]);
	show_bool("tx", "TX:\t\t%s\n", tb[ETHTOOL_A_PAUSE_TX]);

	if (!nlctx->is_monitor && tb[ETHTOOL_A_PAUSE_AUTONEG] &&
	    mnl_attr_get_u8(tb[ETHTOOL_A_PAUSE_AUTONEG])) {
		ret = show_pause_autoneg_status(nlctx);
		if (ret < 0)
			goto err_close_dev;
	}
	if (tb[ETHTOOL_A_PAUSE_STATS]) {
		ret = show_pause_stats(tb[ETHTOOL_A_PAUSE_STATS]);
		if (ret < 0)
			goto err_close_dev;
	}
	if (!silent)
		print_nl();

	close_json_object();

	return MNL_CB_OK;

err_close_dev:
	close_json_object();
	return err_ret;
}

int nl_gpause(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	u32 flags;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PAUSE_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	flags = get_stats_flag(nlctx, ETHTOOL_MSG_PAUSE_GET,
			       ETHTOOL_A_PAUSE_HEADER);
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_PAUSE_GET,
				      ETHTOOL_A_PAUSE_HEADER, flags);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, pause_reply_cb);
	delete_json_obj();
	return ret;
}

/* PAUSE_SET */

static const struct param_parser spause_params[] = {
	{
		.arg		= "autoneg",
		.type		= ETHTOOL_A_PAUSE_AUTONEG,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "rx",
		.type		= ETHTOOL_A_PAUSE_RX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "tx",
		.type		= ETHTOOL_A_PAUSE_TX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{}
};

int nl_spause(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PAUSE_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-A";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_PAUSE_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_PAUSE_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, spause_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 76;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 76;
}
