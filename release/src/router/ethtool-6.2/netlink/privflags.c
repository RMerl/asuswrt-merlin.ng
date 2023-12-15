/*
 * privflags.c - netlink implementation of private flags commands
 *
 * Implementation of "ethtool --show-priv-flags <dev>" and
 * "ethtool --set-priv-flags <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "strset.h"
#include "bitset.h"
#include "parser.h"

/* PRIVFLAGS_GET */

static void privflags_maxlen_walk_cb(unsigned int idx, const char *name,
				     bool val __maybe_unused, void *data)
{
	unsigned int *maxlen = data;
	unsigned int len, n;

	if (name)
		len = strlen(name);
	else {
		len = 3; /* strlen("bit") */
		for (n = idx ?: 1; n; n /= 10)
			len++; /* plus number of ditigs */
	}
	if (len > *maxlen)
		*maxlen = len;
}

static void privflags_dump_walk_cb(unsigned int idx, const char *name, bool val,
				   void *data)
{
	unsigned int *maxlen = data;
	char buff[16];

	if (!name) {
		snprintf(buff, sizeof(buff) - 1, "bit%u", idx);
		name = buff;
	}
	printf("%-*s: %s\n", *maxlen, name, val ? "on" : "off");
}

int privflags_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PRIVFLAGS_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	const struct stringset *flag_names = NULL;
	struct nl_context *nlctx = data;
	unsigned int maxlen = 0;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0 || !tb[ETHTOOL_A_PRIVFLAGS_FLAGS])
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_PRIVFLAGS_HEADER]);
	if (!dev_ok(nlctx))
		return MNL_CB_OK;

	if (bitset_is_compact(tb[ETHTOOL_A_PRIVFLAGS_FLAGS])) {
		ret = netlink_init_ethnl2_socket(nlctx);
		if (ret < 0)
			return err_ret;
		flag_names = perdev_stringset(nlctx->devname, ETH_SS_PRIV_FLAGS,
					      nlctx->ethnl2_socket);
	}

	ret = walk_bitset(tb[ETHTOOL_A_PRIVFLAGS_FLAGS], flag_names,
			  privflags_maxlen_walk_cb, &maxlen);
	if (ret < 0)
		return err_ret;
	if (silent)
		putchar('\n');
	printf("Private flags for %s:\n", nlctx->devname);
	ret = walk_bitset(tb[ETHTOOL_A_PRIVFLAGS_FLAGS], flag_names,
			  privflags_dump_walk_cb, &maxlen);
	return (ret < 0) ? err_ret : MNL_CB_OK;
}

int nl_gprivflags(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PRIVFLAGS_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_PRIVFLAGS_GET,
				      ETHTOOL_A_PRIVFLAGS_HEADER, 0);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, privflags_reply_cb);
}

/* PRIVFLAGS_SET */

static const struct bitset_parser_data privflags_parser_data = {
	.force_hex	= false,
	.no_mask	= false,
};

int nl_sprivflags(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PRIVFLAGS_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "--set-priv-flags";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_PRIVFLAGS_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_PRIVFLAGS_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parse_bitset(nlctx, ETHTOOL_A_PRIVFLAGS_FLAGS,
			      &privflags_parser_data, msgbuff, NULL);
	if (ret < 0)
		return -EINVAL;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 2;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 1;
}
