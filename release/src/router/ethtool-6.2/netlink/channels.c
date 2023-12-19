/*
 * channels.c - netlink implementation of channel commands
 *
 * Implementation of "ethtool -l <dev>" and "ethtool -L <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* CHANNELS_GET */

int channels_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_CHANNELS_MAX + 1] = {};
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
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_CHANNELS_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');
	printf("Channel parameters for %s:\n", nlctx->devname);
	printf("Pre-set maximums:\n");
	show_u32("rx-max", "RX:\t\t", tb[ETHTOOL_A_CHANNELS_RX_MAX]);
	show_u32("tx-max", "TX:\t\t", tb[ETHTOOL_A_CHANNELS_TX_MAX]);
	show_u32("other-max", "Other:\t\t", tb[ETHTOOL_A_CHANNELS_OTHER_MAX]);
	show_u32("combined-max", "Combined:\t",
		 tb[ETHTOOL_A_CHANNELS_COMBINED_MAX]);
	printf("Current hardware settings:\n");
	show_u32("rx", "RX:\t\t", tb[ETHTOOL_A_CHANNELS_RX_COUNT]);
	show_u32("tx", "TX:\t\t", tb[ETHTOOL_A_CHANNELS_TX_COUNT]);
	show_u32("other", "Other:\t\t", tb[ETHTOOL_A_CHANNELS_OTHER_COUNT]);
	show_u32("combined", "Combined:\t",
		 tb[ETHTOOL_A_CHANNELS_COMBINED_COUNT]);

	return MNL_CB_OK;
}

int nl_gchannels(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_CHANNELS_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_CHANNELS_GET,
				      ETHTOOL_A_CHANNELS_HEADER, 0);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, channels_reply_cb);
}

/* CHANNELS_SET */

static const struct param_parser schannels_params[] = {
	{
		.arg		= "rx",
		.type		= ETHTOOL_A_CHANNELS_RX_COUNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx",
		.type		= ETHTOOL_A_CHANNELS_TX_COUNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "other",
		.type		= ETHTOOL_A_CHANNELS_OTHER_COUNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "combined",
		.type		= ETHTOOL_A_CHANNELS_COMBINED_COUNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_schannels(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_CHANNELS_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-L";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_CHANNELS_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_CHANNELS_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, schannels_params, NULL, PARSER_GROUP_NONE, NULL);
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
