/*
 * coalesce.c - netlink implementation of coalescing commands
 *
 * Implementation of "ethtool -c <dev>" and "ethtool -C <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* COALESCE_GET */

int coalesce_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_COALESCE_MAX + 1] = {};
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
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_COALESCE_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	open_json_object(NULL);

	if (silent)
		show_cr();
	print_string(PRINT_ANY, "ifname", "Coalesce parameters for %s:\n",
		     nlctx->devname);
	show_bool("rx", "Adaptive RX: %s  ",
		  tb[ETHTOOL_A_COALESCE_USE_ADAPTIVE_RX]);
	show_bool("tx", "TX: %s\n", tb[ETHTOOL_A_COALESCE_USE_ADAPTIVE_TX]);
	show_u32("stats-block-usecs", "stats-block-usecs:\t",
		 tb[ETHTOOL_A_COALESCE_STATS_BLOCK_USECS]);
	show_u32("sample-interval", "sample-interval:\t",
		 tb[ETHTOOL_A_COALESCE_RATE_SAMPLE_INTERVAL]);
	show_u32("pkt-rate-low", "pkt-rate-low:\t\t",
		 tb[ETHTOOL_A_COALESCE_PKT_RATE_LOW]);
	show_u32("pkt-rate-high", "pkt-rate-high:\t\t",
		 tb[ETHTOOL_A_COALESCE_PKT_RATE_HIGH]);
	show_cr();
	show_u32("rx-usecs", "rx-usecs:\t", tb[ETHTOOL_A_COALESCE_RX_USECS]);
	show_u32("rx-frames", "rx-frames:\t",
		 tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES]);
	show_u32("rx-usecs-irq", "rx-usecs-irq:\t",
		 tb[ETHTOOL_A_COALESCE_RX_USECS_IRQ]);
	show_u32("rx-frames-irq", "rx-frames-irq:\t",
		 tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_IRQ]);
	show_cr();
	show_u32("tx-usecs", "tx-usecs:\t", tb[ETHTOOL_A_COALESCE_TX_USECS]);
	show_u32("tx-frames", "tx-frames:\t",
		 tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES]);
	show_u32("tx-usecs-irq", "tx-usecs-irq:\t",
		 tb[ETHTOOL_A_COALESCE_TX_USECS_IRQ]);
	show_u32("tx-frames-irq", "tx-frames-irq:\t",
		 tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_IRQ]);
	show_cr();
	show_u32("rx-usecs-low", "rx-usecs-low:\t",
		 tb[ETHTOOL_A_COALESCE_RX_USECS_LOW]);
	show_u32("rx-frame-low", "rx-frame-low:\t",
		 tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_LOW]);
	show_u32("tx-usecs-low", "tx-usecs-low:\t",
		 tb[ETHTOOL_A_COALESCE_TX_USECS_LOW]);
	show_u32("tx-frame-low", "tx-frame-low:\t",
		 tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_LOW]);
	show_cr();
	show_u32("rx-usecs-high", "rx-usecs-high:\t",
		 tb[ETHTOOL_A_COALESCE_RX_USECS_HIGH]);
	show_u32("rx-frame-high", "rx-frame-high:\t",
		 tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_HIGH]);
	show_u32("tx-usecs-high", "tx-usecs-high:\t",
		 tb[ETHTOOL_A_COALESCE_TX_USECS_HIGH]);
	show_u32("tx-frame-high", "tx-frame-high:\t",
		 tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_HIGH]);
	show_cr();
	show_bool("rx", "CQE mode RX: %s  ",
		  tb[ETHTOOL_A_COALESCE_USE_CQE_MODE_RX]);
	show_bool("tx", "TX: %s\n", tb[ETHTOOL_A_COALESCE_USE_CQE_MODE_TX]);
	show_cr();

	close_json_object();

	return MNL_CB_OK;
}

int nl_gcoalesce(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_COALESCE_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_COALESCE_GET,
				      ETHTOOL_A_COALESCE_HEADER, 0);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, coalesce_reply_cb);
	delete_json_obj();
	return ret;
}

/* COALESCE_SET */

static const struct param_parser scoalesce_params[] = {
	{
		.arg		= "adaptive-rx",
		.type		= ETHTOOL_A_COALESCE_USE_ADAPTIVE_RX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "adaptive-tx",
		.type		= ETHTOOL_A_COALESCE_USE_ADAPTIVE_TX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "sample-interval",
		.type		= ETHTOOL_A_COALESCE_RATE_SAMPLE_INTERVAL,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "stats-block-usecs",
		.type		= ETHTOOL_A_COALESCE_STATS_BLOCK_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "pkt-rate-low",
		.type		= ETHTOOL_A_COALESCE_PKT_RATE_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "pkt-rate-high",
		.type		= ETHTOOL_A_COALESCE_PKT_RATE_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs",
		.type		= ETHTOOL_A_COALESCE_RX_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-irq",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-irq",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs",
		.type		= ETHTOOL_A_COALESCE_TX_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-irq",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-irq",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-low",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-low",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-low",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-low",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-high",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-high",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-high",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-high",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "cqe-mode-rx",
		.type		= ETHTOOL_A_COALESCE_USE_CQE_MODE_RX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "cqe-mode-tx",
		.type		= ETHTOOL_A_COALESCE_USE_CQE_MODE_TX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{}
};

int nl_scoalesce(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_COALESCE_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-C";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_COALESCE_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_COALESCE_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, scoalesce_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 1;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 1;
}
