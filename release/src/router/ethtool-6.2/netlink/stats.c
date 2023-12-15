/*
 * stats.c - netlink implementation of stats
 *
 * Implementation of "ethtool -S <dev> [--groups <types>] etc."
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
#include "strset.h"

static int parse_rmon_hist_one(const char *grp_name, const struct nlattr *hist,
			       const char *dir)
{
	const struct nlattr *tb[ETHTOOL_A_STATS_GRP_HIST_VAL + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	unsigned long long val;
	unsigned int low, hi;
	int ret;

	ret = mnl_attr_parse_nested(hist, attr_cb, &tb_info);
	if (ret < 0) {
		fprintf(stderr, "invalid kernel response - malformed histogram entry\n");
		return 1;
	}

	if (!tb[ETHTOOL_A_STATS_GRP_HIST_BKT_LOW] ||
	    !tb[ETHTOOL_A_STATS_GRP_HIST_BKT_HI] ||
	    !tb[ETHTOOL_A_STATS_GRP_HIST_VAL]) {
		fprintf(stderr, "invalid kernel response - histogram entry missing attributes\n");
		return 1;
	}

	low = mnl_attr_get_u32(tb[ETHTOOL_A_STATS_GRP_HIST_BKT_LOW]);
	hi = mnl_attr_get_u32(tb[ETHTOOL_A_STATS_GRP_HIST_BKT_HI]);
	val = mnl_attr_get_u64(tb[ETHTOOL_A_STATS_GRP_HIST_VAL]);

	if (!is_json_context()) {
		fprintf(stdout, "%s-%s-etherStatsPkts", dir, grp_name);

		if (low && hi) {
			fprintf(stdout, "%uto%uOctets: %llu\n", low, hi, val);
		} else if (hi) {
			fprintf(stdout, "%uOctets: %llu\n", hi, val);
		} else if (low) {
			fprintf(stdout, "%utoMaxOctets: %llu\n", low, val);
		} else {
			fprintf(stderr, "invalid kernel response - bad histogram entry bounds\n");
			return 1;
		}
	} else {
		open_json_object(NULL);
		print_uint(PRINT_JSON, "low", NULL, low);
		print_uint(PRINT_JSON, "high", NULL, hi);
		print_u64(PRINT_JSON, "val", NULL, val);
		close_json_object();
	}

	return 0;
}

static int parse_rmon_hist(const struct nlattr *grp, const char *grp_name,
			   const char *name, const char *dir, unsigned int type)
{
	const struct nlattr *attr;

	open_json_array(name, "");

	mnl_attr_for_each_nested(attr, grp) {
		if (mnl_attr_get_type(attr) == type &&
		    parse_rmon_hist_one(grp_name, attr, dir))
			goto err_close_rmon;
	}
	close_json_array("");

	return 0;

err_close_rmon:
	close_json_array("");
	return 1;
}

static int parse_grp(struct nl_context *nlctx, const struct nlattr *grp,
		     const struct stringset *std_str)
{
	const struct nlattr *tb[ETHTOOL_A_STATS_GRP_SS_ID + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	bool hist_rx = false, hist_tx = false;
	const struct stringset *stat_str;
	const struct nlattr *attr, *stat;
	const char *std_name, *name;
	unsigned int ss_id, id, s;
	unsigned long long val;
	int ret;

	ret = mnl_attr_parse_nested(grp, attr_cb, &tb_info);
	if (ret < 0)
		return 1;

	if (!tb[ETHTOOL_A_STATS_GRP_ID])
		return 1;
	if (!tb[ETHTOOL_A_STATS_GRP_SS_ID])
		return 0;

	id = mnl_attr_get_u32(tb[ETHTOOL_A_STATS_GRP_ID]);
	ss_id = mnl_attr_get_u32(tb[ETHTOOL_A_STATS_GRP_SS_ID]);

	stat_str = global_stringset(ss_id, nlctx->ethnl2_socket);

	std_name = get_string(std_str, id);
	open_json_object(std_name);

	mnl_attr_for_each_nested(attr, grp) {
		switch (mnl_attr_get_type(attr)) {
		case ETHTOOL_A_STATS_GRP_STAT:
			break;
		case ETHTOOL_A_STATS_GRP_HIST_RX:
			hist_rx = true;
			continue;
		case ETHTOOL_A_STATS_GRP_HIST_TX:
			hist_tx = true;
			continue;
		default:
			continue;
		}

		stat = mnl_attr_get_payload(attr);
		ret = mnl_attr_validate(stat, MNL_TYPE_U64);
		if (ret) {
			fprintf(stderr, "invalid kernel response - bad statistic entry\n");
			goto err_close_grp;
		}
		s = mnl_attr_get_type(stat);
		name = get_string(stat_str, s);
		if (!name || !name[0])
			continue;

		if (!is_json_context())
			fprintf(stdout, "%s-%s: ", std_name, name);

		val = mnl_attr_get_u64(stat);
		print_u64(PRINT_ANY, name, "%llu\n", val);
	}

	if (hist_rx)
		parse_rmon_hist(grp, std_name, "rx-pktsNtoM", "rx",
				ETHTOOL_A_STATS_GRP_HIST_RX);
	if (hist_tx)
		parse_rmon_hist(grp, std_name, "tx-pktsNtoM", "tx",
				ETHTOOL_A_STATS_GRP_HIST_TX);

	close_json_object();

	return 0;

err_close_grp:
	close_json_object();
	return 1;
}

static int stats_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_STATS_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	const struct stringset *std_str;
	const struct nlattr *attr;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_STATS_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	ret = netlink_init_ethnl2_socket(nlctx);
	if (ret < 0)
		return err_ret;
	std_str = global_stringset(ETH_SS_STATS_STD, nlctx->ethnl2_socket);

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "Standard stats for %s:\n",
		     nlctx->devname);

	mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
		if (mnl_attr_get_type(attr) == ETHTOOL_A_STATS_GRP) {
			ret = parse_grp(nlctx, attr, std_str);
			if (ret)
				goto err_close_dev;
		}
	}

	close_json_object();

	return MNL_CB_OK;

err_close_dev:
	close_json_object();
	return err_ret;
}

static const struct bitset_parser_data stats_parser_data = {
	.no_mask	= true,
	.force_hex	= false,
};

static int stats_parse_all_groups(struct nl_context *nlctx, uint16_t type,
				  const void *data, struct nl_msg_buff *msgbuff,
				  void *dest)
{
	const struct stringset *std_str;
	struct nlattr *nest;
	int i, ret, nbits;
	uint32_t *bits;

	if (data || dest)
		return -EFAULT;

	/* ethnl2 and strset code already does caching */
	ret = netlink_init_ethnl2_socket(nlctx);
	if (ret < 0)
		return ret;
	std_str = global_stringset(ETH_SS_STATS_STD, nlctx->ethnl2_socket);

	nbits = get_count(std_str);
	bits = calloc(DIV_ROUND_UP(nbits, 32), sizeof(uint32_t));
	if (!bits)
		return -ENOMEM;

	for (i = 0; i < nbits; i++)
		bits[i / 32] |= 1U << (i % 32);

	ret = -EMSGSIZE;
	nest = ethnla_nest_start(msgbuff, type);
	if (!nest)
		goto err_free;

	if (ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_NOMASK, true) ||
	    ethnla_put_u32(msgbuff, ETHTOOL_A_BITSET_SIZE, nbits) ||
	    ethnla_put(msgbuff, ETHTOOL_A_BITSET_VALUE,
		       DIV_ROUND_UP(nbits, 32) * sizeof(uint32_t), bits))
		goto err_cancel;

	ethnla_nest_end(msgbuff, nest);
	free(bits);
	return 0;

err_cancel:
	ethnla_nest_cancel(msgbuff, nest);
err_free:
	free(bits);
	return ret;
}

static const struct param_parser stats_params[] = {
	{
		.arg		= "--groups",
		.type		= ETHTOOL_A_STATS_GROUPS,
		.handler	= nl_parse_bitset,
		.handler_data	= &stats_parser_data,
		.min_argc	= 1,
		.alt_group	= 1,
	},
	{
		.arg		= "--all-groups",
		.type		= ETHTOOL_A_STATS_GROUPS,
		.handler	= stats_parse_all_groups,
		.alt_group	= 1,
	},
	{}
};

int nl_gstats(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_STATS_GET,
				      ETHTOOL_A_STATS_HEADER, 0);
	if (ret < 0)
		return ret;

	nlctx->cmd = "-S";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;

	ret = nl_parser(nlctx, stats_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, stats_reply_cb);
	delete_json_obj();
	return ret;
}

bool nl_gstats_chk(struct cmd_context *ctx)
{
	return ctx->argc;
}
