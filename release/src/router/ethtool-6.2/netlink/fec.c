/*
 * fec.c - netlink implementation of FEC commands
 *
 * Implementation of "ethtool --show-fec <dev>" and
 * "ethtool --set-fec <dev> ..."
 */

#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "bitset.h"
#include "parser.h"

/* FEC_GET */

static void
fec_mode_walk(unsigned int idx, const char *name, bool val, void *data)
{
	bool *empty = data;

	if (!val)
		return;
	if (empty)
		*empty = false;

	/* Rename None to Off - in legacy ioctl None means "not supported"
	 * rather than supported but disabled.
	 */
	if (idx == ETHTOOL_LINK_MODE_FEC_NONE_BIT)
		name = "Off";
	/* Rename to match the ioctl letter case */
	else if (idx == ETHTOOL_LINK_MODE_FEC_BASER_BIT)
		name = "BaseR";

	print_string(PRINT_ANY, NULL, " %s", name);
}

static int fec_show_stats(const struct nlattr *nest)
{
	const struct nlattr *tb[ETHTOOL_A_FEC_STAT_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	static const struct {
		unsigned int attr;
		char *name;
	} stats[] = {
		{ ETHTOOL_A_FEC_STAT_CORRECTED, "corrected_blocks" },
		{ ETHTOOL_A_FEC_STAT_UNCORR, "uncorrectable_blocks" },
		{ ETHTOOL_A_FEC_STAT_CORR_BITS, "corrected_bits" },
	};
	bool header = false;
	unsigned int i;
	int ret;

	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	if (ret < 0)
		return ret;

	open_json_object("statistics");
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		uint64_t *vals;
		int lanes, l;

		if (!tb[stats[i].attr] ||
		    !mnl_attr_get_payload_len(tb[stats[i].attr]))
			continue;

		if (!header && !is_json_context()) {
			printf("Statistics:\n");
			header = true;
		}

		if (mnl_attr_get_payload_len(tb[stats[i].attr]) % 8) {
			fprintf(stderr, "malformed netlink message (statistic)\n");
			goto err_close_stats;
		}

		vals = mnl_attr_get_payload(tb[stats[i].attr]);
		lanes = mnl_attr_get_payload_len(tb[stats[i].attr]) / 8 - 1;

		if (!is_json_context()) {
			fprintf(stdout, "  %s: %" PRIu64 "\n",
				stats[i].name, *vals++);
		} else {
			open_json_object(stats[i].name);
			print_u64(PRINT_JSON, "total", NULL, *vals++);
		}

		if (lanes)
			open_json_array("lanes", "");
		for (l = 0; l < lanes; l++) {
			if (!is_json_context())
				fprintf(stdout, "    Lane %d: %" PRIu64 "\n",
					l, *vals++);
			else
				print_u64(PRINT_JSON, NULL, NULL, *vals++);
		}
		if (lanes)
			close_json_array("");

		close_json_object();
	}
	close_json_object();

	return 0;

err_close_stats:
	close_json_object();
	return -1;
}

int fec_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_FEC_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	const struct stringset *lm_strings;
	const char *name;
	bool fa, empty;
	bool silent;
	int err_ret;
	u32 active;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_FEC_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	ret = netlink_init_ethnl2_socket(nlctx);
	if (ret < 0)
		return err_ret;
	lm_strings = global_stringset(ETH_SS_LINK_MODES, nlctx->ethnl2_socket);

	active = 0;
	if (tb[ETHTOOL_A_FEC_ACTIVE])
		active = mnl_attr_get_u32(tb[ETHTOOL_A_FEC_ACTIVE]);

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "FEC parameters for %s:\n",
		     nlctx->devname);

	open_json_array("config", "Supported/Configured FEC encodings:");
	fa = tb[ETHTOOL_A_FEC_AUTO] && mnl_attr_get_u8(tb[ETHTOOL_A_FEC_AUTO]);
	if (fa)
		print_string(PRINT_ANY, NULL, " %s", "Auto");
	empty = !fa;

	ret = walk_bitset(tb[ETHTOOL_A_FEC_MODES], lm_strings, fec_mode_walk,
			  &empty);
	if (ret < 0)
		goto err_close_dev;
	if (empty)
		print_string(PRINT_ANY, NULL, " %s", "None");
	close_json_array("\n");

	open_json_array("active", "Active FEC encoding:");
	if (active) {
		name = get_string(lm_strings, active);
		if (name)
			/* Take care of renames */
			fec_mode_walk(active, name, true, NULL);
		else
			print_uint(PRINT_ANY, NULL, " BIT%u", active);
	} else {
		print_string(PRINT_ANY, NULL, " %s", "None");
	}
	close_json_array("\n");

	if (tb[ETHTOOL_A_FEC_STATS]) {
		ret = fec_show_stats(tb[ETHTOOL_A_FEC_STATS]);
		if (ret < 0)
			goto err_close_dev;
	}

	close_json_object();

	return MNL_CB_OK;

err_close_dev:
	close_json_object();
	return err_ret;
}

int nl_gfec(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	u32 flags;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_FEC_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	flags = get_stats_flag(nlctx, ETHTOOL_MSG_FEC_GET,
			       ETHTOOL_A_FEC_HEADER);
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_FEC_GET,
				      ETHTOOL_A_FEC_HEADER, flags);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, fec_reply_cb);
	delete_json_obj();
	return ret;
}

/* FEC_SET */

static void strupc(char *dst, const char *src)
{
	while (*src)
		*dst++ = toupper(*src++);
	*dst = '\0';
}

static int fec_parse_bitset(struct nl_context *nlctx, uint16_t type,
			    const void *data __maybe_unused,
			    struct nl_msg_buff *msgbuff, void *dest)
{
	struct nlattr *bitset_attr;
	struct nlattr *bits_attr;
	struct nlattr *bit_attr;
	char upper[ETH_GSTRING_LEN];
	bool fec_auto = false;
	int ret;

	if (!type || dest) {
		fprintf(stderr, "ethtool (%s): internal error parsing '%s'\n",
			nlctx->cmd, nlctx->param);
		return -EFAULT;
	}

	bitset_attr = ethnla_nest_start(msgbuff, type);
	if (!bitset_attr)
		return -EMSGSIZE;
	ret = -EMSGSIZE;
	if (ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_NOMASK, true))
		goto err;
	bits_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_BITSET_BITS);
	if (!bits_attr)
		goto err;

	while (nlctx->argc > 0) {
		const char *name = *nlctx->argp;

		if (!strcmp(name, "--")) {
			nlctx->argp++;
			nlctx->argc--;
			break;
		}

		if (!strcasecmp(name, "auto")) {
			fec_auto = true;
			goto next;
		}
		if (!strcasecmp(name, "off")) {
			name = "None";
		} else {
			strupc(upper, name);
			name = upper;
		}

		ret = -EMSGSIZE;
		bit_attr = ethnla_nest_start(msgbuff,
					     ETHTOOL_A_BITSET_BITS_BIT);
		if (!bit_attr)
			goto err;
		if (ethnla_put_strz(msgbuff, ETHTOOL_A_BITSET_BIT_NAME, name))
			goto err;
		ethnla_nest_end(msgbuff, bit_attr);

next:
		nlctx->argp++;
		nlctx->argc--;
	}

	ethnla_nest_end(msgbuff, bits_attr);
	ethnla_nest_end(msgbuff, bitset_attr);

	if (ethnla_put_u8(msgbuff, ETHTOOL_A_FEC_AUTO, fec_auto))
		goto err;

	return 0;
err:
	ethnla_nest_cancel(msgbuff, bitset_attr);
	return ret;
}

static const struct param_parser sfec_params[] = {
	{
		.arg		= "encoding",
		.type		= ETHTOOL_A_FEC_MODES,
		.handler	= fec_parse_bitset,
		.min_argc	= 1,
	},
	{}
};

int nl_sfec(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_FEC_SET, false))
		return -EOPNOTSUPP;
	if (!ctx->argc) {
		fprintf(stderr, "ethtool (--set-fec): parameters missing\n");
		return 1;
	}

	nlctx->cmd = "--set-fec";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_FEC_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_FEC_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, sfec_params, NULL, PARSER_GROUP_NONE, NULL);
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
