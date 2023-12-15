/*
 * rings.c - netlink implementation of ring commands
 *
 * Implementation of "ethtool -g <dev>" and "ethtool -G <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* RINGS_GET */

int rings_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_RINGS_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	unsigned char tcp_hds;
	char *tcp_hds_fmt;
	char *tcp_hds_key;
	char tcp_hds_buf[256];
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_RINGS_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	open_json_object(NULL);

	if (silent)
		show_cr();
	print_string(PRINT_ANY, "ifname", "Ring parameters for %s:\n",
		     nlctx->devname);
	print_string(PRINT_FP, NULL, "Pre-set maximums:\n", NULL);
	show_u32("rx-max", "RX:\t\t", tb[ETHTOOL_A_RINGS_RX_MAX]);
	show_u32("rx-mini-max", "RX Mini:\t", tb[ETHTOOL_A_RINGS_RX_MINI_MAX]);
	show_u32("rx-jumbo-max", "RX Jumbo:\t",
		 tb[ETHTOOL_A_RINGS_RX_JUMBO_MAX]);
	show_u32("tx-max", "TX:\t\t", tb[ETHTOOL_A_RINGS_TX_MAX]);
	print_string(PRINT_FP, NULL, "Current hardware settings:\n", NULL);
	show_u32("rx", "RX:\t\t", tb[ETHTOOL_A_RINGS_RX]);
	show_u32("rx-mini", "RX Mini:\t", tb[ETHTOOL_A_RINGS_RX_MINI]);
	show_u32("rx-jumbo", "RX Jumbo:\t", tb[ETHTOOL_A_RINGS_RX_JUMBO]);
	show_u32("tx", "TX:\t\t", tb[ETHTOOL_A_RINGS_TX]);
	show_u32("rx-buf-len", "RX Buf Len:\t", tb[ETHTOOL_A_RINGS_RX_BUF_LEN]);
	show_u32("cqe-size", "CQE Size:\t", tb[ETHTOOL_A_RINGS_CQE_SIZE]);
	show_bool("tx-push", "TX Push:\t%s\n", tb[ETHTOOL_A_RINGS_TX_PUSH]);

	tcp_hds_fmt = "TCP data split:\t%s\n";
	tcp_hds_key = "tcp-data-split";
	tcp_hds = tb[ETHTOOL_A_RINGS_TCP_DATA_SPLIT] ?
		mnl_attr_get_u8(tb[ETHTOOL_A_RINGS_TCP_DATA_SPLIT]) : 0;
	switch (tcp_hds) {
	case ETHTOOL_TCP_DATA_SPLIT_UNKNOWN:
		print_string(PRINT_FP, tcp_hds_key, tcp_hds_fmt, "n/a");
		break;
	case ETHTOOL_TCP_DATA_SPLIT_DISABLED:
		print_string(PRINT_ANY, tcp_hds_key, tcp_hds_fmt, "off");
		break;
	case ETHTOOL_TCP_DATA_SPLIT_ENABLED:
		print_string(PRINT_ANY, tcp_hds_key, tcp_hds_fmt, "on");
		break;
	default:
		snprintf(tcp_hds_buf, sizeof(tcp_hds_buf),
			 "unknown(%d)\n", tcp_hds);
		print_string(PRINT_ANY, tcp_hds_key, tcp_hds_fmt, tcp_hds_buf);
		break;
	}

	close_json_object();

	return MNL_CB_OK;
}

int nl_gring(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_RINGS_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_RINGS_GET,
				      ETHTOOL_A_RINGS_HEADER, 0);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, rings_reply_cb);
	delete_json_obj();
	return ret;
}

/* RINGS_SET */

static const struct param_parser sring_params[] = {
	{
		.arg		= "rx",
		.type		= ETHTOOL_A_RINGS_RX,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-mini",
		.type		= ETHTOOL_A_RINGS_RX_MINI,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-jumbo",
		.type		= ETHTOOL_A_RINGS_RX_JUMBO,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx",
		.type		= ETHTOOL_A_RINGS_TX,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg            = "rx-buf-len",
		.type           = ETHTOOL_A_RINGS_RX_BUF_LEN,
		.handler        = nl_parse_direct_u32,
		.min_argc       = 1,
	},
	{
		.arg            = "cqe-size",
		.type           = ETHTOOL_A_RINGS_CQE_SIZE,
		.handler        = nl_parse_direct_u32,
		.min_argc       = 1,
	},
	{
		.arg            = "tx-push",
		.type           = ETHTOOL_A_RINGS_TX_PUSH,
		.handler        = nl_parse_u8bool,
		.min_argc       = 1,
	},
	{}
};

int nl_sring(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_RINGS_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-G";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_RINGS_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_RINGS_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, sring_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 81;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 81;
}
