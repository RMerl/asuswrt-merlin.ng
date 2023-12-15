/*
 * module.c - netlink implementation of module commands
 *
 * Implementation of "ethtool --show-module <dev>" and
 * "ethtool --set-module <dev> ..."
 */

#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* MODULE_GET */

static const char *module_power_mode_policy_name(u8 val)
{
	switch (val) {
	case ETHTOOL_MODULE_POWER_MODE_POLICY_HIGH:
		return "high";
	case ETHTOOL_MODULE_POWER_MODE_POLICY_AUTO:
		return "auto";
	default:
		return "unknown";
	}
}

static const char *module_power_mode_name(u8 val)
{
	switch (val) {
	case ETHTOOL_MODULE_POWER_MODE_LOW:
		return "low";
	case ETHTOOL_MODULE_POWER_MODE_HIGH:
		return "high";
	default:
		return "unknown";
	}
}

int module_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_MODULE_MAX + 1] = {};
	struct nl_context *nlctx = data;
	DECLARE_ATTR_TB_INFO(tb);
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_MODULE_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "Module parameters for %s:\n",
		     nlctx->devname);

	if (tb[ETHTOOL_A_MODULE_POWER_MODE_POLICY]) {
		u8 val;

		val = mnl_attr_get_u8(tb[ETHTOOL_A_MODULE_POWER_MODE_POLICY]);
		print_string(PRINT_ANY, "power-mode-policy",
			     "power-mode-policy: %s\n",
			     module_power_mode_policy_name(val));
	}

	if (tb[ETHTOOL_A_MODULE_POWER_MODE]) {
		u8 val;

		val = mnl_attr_get_u8(tb[ETHTOOL_A_MODULE_POWER_MODE]);
		print_string(PRINT_ANY, "power-mode",
			     "power-mode: %s\n", module_power_mode_name(val));
	}

	close_json_object();

	return MNL_CB_OK;
}

int nl_gmodule(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_MODULE_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	nlsk = nlctx->ethnl_socket;
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_MODULE_GET,
				      ETHTOOL_A_MODULE_HEADER, 0);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, module_reply_cb);
	delete_json_obj();
	return ret;
}

/* MODULE_SET */

static const struct lookup_entry_u8 power_mode_policy_values[] = {
	{ .arg = "high",	.val = ETHTOOL_MODULE_POWER_MODE_POLICY_HIGH },
	{ .arg = "auto",	.val = ETHTOOL_MODULE_POWER_MODE_POLICY_AUTO },
	{}
};

static const struct param_parser smodule_params[] = {
	{
		.arg		= "power-mode-policy",
		.type		= ETHTOOL_A_MODULE_POWER_MODE_POLICY,
		.handler	= nl_parse_lookup_u8,
		.handler_data	= power_mode_policy_values,
		.min_argc	= 1,
	},
	{}
};

int nl_smodule(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_MODULE_SET, false))
		return -EOPNOTSUPP;
	if (!ctx->argc) {
		fprintf(stderr, "ethtool (--set-module): parameters missing\n");
		return 1;
	}

	nlctx->cmd = "--set-module";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_MODULE_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_MODULE_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, smodule_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 83;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 83;
}
