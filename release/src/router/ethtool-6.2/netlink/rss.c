/*
 * rss.c - netlink implementation of RSS context commands
 *
 * Implementation of "ethtool -x <dev>"
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "strset.h"
#include "parser.h"

struct cb_args {
	struct nl_context	*nlctx;
	u32			num_rings;
};

void dump_json_rss_info(struct cmd_context *ctx, u32 *indir_table,
			u32 indir_size, u8 *hkey, u32 hkey_size,
			const struct stringset *hash_funcs, u8 hfunc)
{
	unsigned int i;

	open_json_object(NULL);
	print_string(PRINT_JSON, "ifname", NULL, ctx->devname);
	if (indir_size) {
		open_json_array("rss-indirection-table", NULL);
		for (i = 0; i < indir_size; i++)
			print_uint(PRINT_JSON, NULL, NULL, indir_table[i]);
		close_json_array("\n");
	}

	if (hkey_size) {
		open_json_array("rss-hash-key", NULL);
		for (i = 0; i < hkey_size; i++)
			print_uint(PRINT_JSON, NULL, NULL, (u8)hkey[i]);
		close_json_array("\n");
	}

	if (hfunc) {
		for (i = 0; i < get_count(hash_funcs); i++) {
			if (hfunc & (1 << i)) {
				print_string(PRINT_JSON, "rss-hash-function",
					     NULL, get_string(hash_funcs, i));
				break;
			}
		}
	}

	close_json_object();
}

int get_channels_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_CHANNELS_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct cb_args *args = data;
	struct nl_context *nlctx = args->nlctx;
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
	if (tb[ETHTOOL_A_CHANNELS_COMBINED_COUNT])
		args->num_rings = mnl_attr_get_u32(tb[ETHTOOL_A_CHANNELS_COMBINED_COUNT]);
	if (tb[ETHTOOL_A_CHANNELS_RX_COUNT])
		args->num_rings += mnl_attr_get_u32(tb[ETHTOOL_A_CHANNELS_RX_COUNT]);
	return MNL_CB_OK;
}

int rss_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_RSS_MAX + 1] = {};
	unsigned int indir_bytes = 0, hkey_bytes = 0;
	DECLARE_ATTR_TB_INFO(tb);
	struct cb_args *args = data;
	struct nl_context *nlctx = args->nlctx;
	const struct stringset *hash_funcs;
	u32 rss_hfunc = 0, indir_size;
	u32 *indir_table = NULL;
	u8 *hkey = NULL;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_RSS_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	show_cr();

	if (tb[ETHTOOL_A_RSS_HFUNC])
		rss_hfunc = mnl_attr_get_u32(tb[ETHTOOL_A_RSS_HFUNC]);

	if (tb[ETHTOOL_A_RSS_INDIR]) {
		indir_bytes = mnl_attr_get_payload_len(tb[ETHTOOL_A_RSS_INDIR]);
		indir_table = mnl_attr_get_payload(tb[ETHTOOL_A_RSS_INDIR]);
	}

	if (tb[ETHTOOL_A_RSS_HKEY]) {
		hkey_bytes = mnl_attr_get_payload_len(tb[ETHTOOL_A_RSS_HKEY]);
		hkey = mnl_attr_get_payload(tb[ETHTOOL_A_RSS_HKEY]);
	}

	/* Fetch RSS hash functions and their status and print */
	if (!nlctx->is_monitor) {
		ret = netlink_init_ethnl2_socket(nlctx);
		if (ret < 0)
			return MNL_CB_ERROR;
	}
	hash_funcs = global_stringset(ETH_SS_RSS_HASH_FUNCS,
				      nlctx->ethnl2_socket);

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return silent ? MNL_CB_OK : MNL_CB_ERROR;

	nlctx->devname = get_dev_name(tb[ETHTOOL_A_RSS_HEADER]);
	if (!dev_ok(nlctx))
		return MNL_CB_OK;

	/* Fetch ring count info into args->num_rings */
	ret = nlsock_prep_get_request(nlctx->ethnl2_socket,
				      ETHTOOL_MSG_CHANNELS_GET,
				      ETHTOOL_A_CHANNELS_HEADER, 0);
	if (ret < 0)
		return MNL_CB_ERROR;

	ret = nlsock_sendmsg(nlctx->ethnl2_socket, NULL);
	if (ret < 0)
		return MNL_CB_ERROR;

	ret = nlsock_process_reply(nlctx->ethnl2_socket, get_channels_cb, args);
	if (ret < 0)
		return MNL_CB_ERROR;

	indir_size = indir_bytes / sizeof(u32);
	if (is_json_context()) {
		dump_json_rss_info(nlctx->ctx, (u32 *)indir_table, indir_size,
				   hkey, hkey_bytes, hash_funcs, rss_hfunc);
	} else {
		print_indir_table(nlctx->ctx, args->num_rings,
				  indir_size, (u32 *)indir_table);
		print_rss_hkey(hkey, hkey_bytes);
		printf("RSS hash function:\n");
		if (!rss_hfunc) {
			printf("    Operation not supported\n");
			return 0;
		}
		for (unsigned int i = 0; i < get_count(hash_funcs); i++) {
			printf("    %s: %s\n", get_string(hash_funcs, i),
			       (rss_hfunc & (1 << i)) ? "on" : "off");
		}
	}

	return MNL_CB_OK;
}

/* RSS_GET */
static const struct param_parser grss_params[] = {
	{
		.arg		= "context",
		.type		= ETHTOOL_A_RSS_CONTEXT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_grss(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	struct nl_msg_buff *msgbuff;
	struct cb_args args = {};
	int ret;

	nlctx->cmd = "-x";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_RSS_GET, true))
		return -EOPNOTSUPP;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_RSS_GET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 1;

	if (ethnla_fill_header(msgbuff, ETHTOOL_A_RSS_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, grss_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		goto err;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		goto err;

	args.nlctx = nlctx;
	new_json_obj(ctx->json);
	ret = nlsock_process_reply(nlsk, rss_reply_cb, &args);
	delete_json_obj();

	if (ret == 0)
		return 0;
err:
	return nlctx->exit_code ?: 1;
}
