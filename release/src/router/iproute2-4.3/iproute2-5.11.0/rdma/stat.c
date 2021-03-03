// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * rdma.c	RDMA tool
 * Authors:     Mark Zhang <markz@mellanox.com>
 */

#include "rdma.h"
#include "res.h"
#include "stat.h"
#include <inttypes.h>

static int stat_help(struct rd *rd)
{
	pr_out("Usage: %s [ OPTIONS ] statistic { COMMAND | help }\n", rd->filename);
	pr_out("       %s statistic OBJECT show\n", rd->filename);
	pr_out("       %s statistic OBJECT show link [ DEV/PORT_INDEX ] [ FILTER-NAME FILTER-VALUE ]\n", rd->filename);
	pr_out("       %s statistic OBJECT mode\n", rd->filename);
	pr_out("       %s statistic OBJECT set COUNTER_SCOPE [DEV/PORT_INDEX] auto {CRITERIA | off}\n", rd->filename);
	pr_out("       %s statistic OBJECT bind COUNTER_SCOPE [DEV/PORT_INDEX] [OBJECT-ID] [COUNTER-ID]\n", rd->filename);
	pr_out("       %s statistic OBJECT unbind COUNTER_SCOPE [DEV/PORT_INDEX] [COUNTER-ID]\n", rd->filename);
	pr_out("       %s statistic show\n", rd->filename);
	pr_out("       %s statistic show link [ DEV/PORT_INDEX ]\n", rd->filename);
	pr_out("where  OBJECT: = { qp }\n");
	pr_out("       CRITERIA : = { type }\n");
	pr_out("       COUNTER_SCOPE: = { link | dev }\n");
	pr_out("       FILTER_NAME: = { cntn | lqpn | pid }\n");
	pr_out("Examples:\n");
	pr_out("       %s statistic qp show\n", rd->filename);
	pr_out("       %s statistic qp show link mlx5_2/1\n", rd->filename);
	pr_out("       %s statistic qp mode\n", rd->filename);
	pr_out("       %s statistic qp mode link mlx5_0\n", rd->filename);
	pr_out("       %s statistic qp set link mlx5_2/1 auto type on\n", rd->filename);
	pr_out("       %s statistic qp set link mlx5_2/1 auto off\n", rd->filename);
	pr_out("       %s statistic qp bind link mlx5_2/1 lqpn 178\n", rd->filename);
	pr_out("       %s statistic qp bind link mlx5_2/1 lqpn 178 cntn 4\n", rd->filename);
	pr_out("       %s statistic qp unbind link mlx5_2/1 cntn 4\n", rd->filename);
	pr_out("       %s statistic qp unbind link mlx5_2/1 cntn 4 lqpn 178\n", rd->filename);
	pr_out("       %s statistic show\n", rd->filename);
	pr_out("       %s statistic show link mlx5_2/1\n", rd->filename);

	return 0;
}

struct counter_param {
	char *name;
	uint32_t attr;
};

static struct counter_param auto_params[] = {
	{ "type", RDMA_COUNTER_MASK_QP_TYPE, },
	{ "pid", RDMA_COUNTER_MASK_PID, },
	{ NULL },
};

static int prepare_auto_mode_str(struct nlattr **tb, uint32_t mask,
				 char *output, int len)
{
	char s[] = "qp auto";
	int i, outlen = strlen(s);
	bool first = true;

	memset(output, 0, len);
	snprintf(output, len, "%s", s);

	if (mask) {
		for (i = 0; auto_params[i].name != NULL; i++) {
			if (mask & auto_params[i].attr) {
				outlen += strlen(auto_params[i].name) + 1;
				if (outlen >= len)
					return -EINVAL;
				if (first) {
					strcat(output, " ");
					first = false;
				} else
					strcat(output, ",");

				strcat(output, auto_params[i].name);
			}
		}

		if (outlen + strlen(" on") >= len)
			return -EINVAL;
		strcat(output, " on");
	} else {
		if (outlen + strlen(" off") >= len)
			return -EINVAL;
		strcat(output, " off");
	}

	return 0;
}

static int qp_link_get_mode_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	uint32_t mode = 0, mask = 0;
	char output[128] = {};
	struct rd *rd = data;
	uint32_t idx, port;
	const char *name;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME])
		return MNL_CB_ERROR;

	if (!tb[RDMA_NLDEV_ATTR_PORT_INDEX]) {
		pr_err("This tool doesn't support switches yet\n");
		return MNL_CB_ERROR;
	}

	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	port = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_PORT_INDEX]);
	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	if (tb[RDMA_NLDEV_ATTR_STAT_MODE])
		mode = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_STAT_MODE]);

	if (mode == RDMA_COUNTER_MODE_AUTO) {
		if (!tb[RDMA_NLDEV_ATTR_STAT_AUTO_MODE_MASK])
			return MNL_CB_ERROR;
		mask = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_STAT_AUTO_MODE_MASK]);
		prepare_auto_mode_str(tb, mask, output, sizeof(output));
	} else {
		snprintf(output, sizeof(output), "qp auto off");
	}

	open_json_object(NULL);
	print_link(rd, idx, name, port, tb);
	print_color_string(PRINT_ANY, COLOR_NONE, "mode", "mode %s ", output);
	newline(rd);
	return MNL_CB_OK;
}

static int stat_one_qp_link_get_mode(struct rd *rd)
{
	uint32_t seq;
	int ret;

	if (!rd->port_idx)
		return 0;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_GET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));

	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	/* Make RDMA_NLDEV_ATTR_STAT_MODE valid so that kernel knows
	 * return only mode instead of all counters
	 */
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_MODE,
			 RDMA_COUNTER_MODE_MANUAL);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	ret = rd_recv_msg(rd, qp_link_get_mode_parse_cb, rd, seq);
	return ret;
}

static int stat_qp_link_get_mode(struct rd *rd)
{
	return rd_exec_link(rd, stat_one_qp_link_get_mode, false);
}

static int stat_qp_get_mode(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_qp_link_get_mode },
		{ "link",	stat_qp_link_get_mode },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

int res_get_hwcounters(struct rd *rd, struct nlattr *hwc_table, bool print)
{
	struct nlattr *nla_entry;
	const char *nm;
	uint64_t v;
	int err;

	mnl_attr_for_each_nested(nla_entry, hwc_table) {
		struct nlattr *hw_line[RDMA_NLDEV_ATTR_MAX] = {};

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, hw_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME] ||
		    !hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_VALUE]) {
			return -EINVAL;
		}

		if (!print)
			continue;

		nm = mnl_attr_get_str(hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME]);
		v = mnl_attr_get_u64(hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_VALUE]);
		if (rd->pretty_output && !rd->json_output)
			newline_indent(rd);
		res_print_uint(rd, nm, v, hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME]);
	}

	return MNL_CB_OK;
}

static int res_counter_line(struct rd *rd, const char *name, int index,
		       struct nlattr **nla_line)
{
	uint32_t cntn, port = 0, pid = 0, qpn, qp_type = 0;
	struct nlattr *hwc_table, *qp_table;
	struct nlattr *nla_entry;
	const char *comm = NULL;
	bool isfirst;
	int err;

	if (nla_line[RDMA_NLDEV_ATTR_PORT_INDEX])
		port = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_PORT_INDEX]);

	hwc_table = nla_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS];
	qp_table = nla_line[RDMA_NLDEV_ATTR_RES_QP];
	if (!hwc_table || !qp_table ||
	    !nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID])
		return MNL_CB_ERROR;

	cntn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID]);
	if (rd_is_filtered_attr(rd, "cntn", cntn,
				nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID]))
		return MNL_CB_OK;

	if (nla_line[RDMA_NLDEV_ATTR_RES_TYPE])
		qp_type = mnl_attr_get_u8(nla_line[RDMA_NLDEV_ATTR_RES_TYPE]);

	if (rd_is_string_filtered_attr(rd, "qp-type", qp_types_to_str(qp_type),
				       nla_line[RDMA_NLDEV_ATTR_RES_TYPE]))
		return MNL_CB_OK;

	if (nla_line[RDMA_NLDEV_ATTR_RES_PID]) {
		pid = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PID]);
		comm = get_task_name(pid);
	}
	if (rd_is_filtered_attr(rd, "pid", pid,
				nla_line[RDMA_NLDEV_ATTR_RES_PID]))
		return MNL_CB_OK;

	if (nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME])
		comm = (char *)mnl_attr_get_str(
			nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]);

	mnl_attr_for_each_nested(nla_entry, qp_table) {
		struct nlattr *qp_line[RDMA_NLDEV_ATTR_MAX] = {};

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, qp_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!qp_line[RDMA_NLDEV_ATTR_RES_LQPN])
			return -EINVAL;

		qpn = mnl_attr_get_u32(qp_line[RDMA_NLDEV_ATTR_RES_LQPN]);
		if (rd_is_filtered_attr(rd, "lqpn", qpn,
					qp_line[RDMA_NLDEV_ATTR_RES_LQPN]))
			return MNL_CB_OK;
	}

	err = res_get_hwcounters(rd, hwc_table, false);
	if (err != MNL_CB_OK)
		return err;
	open_json_object(NULL);
	print_link(rd, index, name, port, nla_line);
	print_color_uint(PRINT_ANY, COLOR_NONE, "cntn", "cntn %u ", cntn);
	if (nla_line[RDMA_NLDEV_ATTR_RES_TYPE])
		print_qp_type(rd, qp_type);
	res_print_uint(rd, "pid", pid, nla_line[RDMA_NLDEV_ATTR_RES_PID]);
	print_comm(rd, comm, nla_line);
	res_get_hwcounters(rd, hwc_table, true);
	isfirst = true;
	open_json_array(PRINT_JSON, "lqpn");
	print_color_string(PRINT_FP, COLOR_NONE, NULL, "\n    LQPN: <", NULL);
	mnl_attr_for_each_nested(nla_entry, qp_table) {
		struct nlattr *qp_line[RDMA_NLDEV_ATTR_MAX] = {};
		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, qp_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!qp_line[RDMA_NLDEV_ATTR_RES_LQPN])
			return -EINVAL;

		qpn = mnl_attr_get_u32(qp_line[RDMA_NLDEV_ATTR_RES_LQPN]);
		if (!isfirst)
			print_color_string(PRINT_FP, COLOR_NONE, NULL, ",",
					   NULL);
		print_color_uint(PRINT_ANY, COLOR_NONE, NULL, "%d", qpn);
		isfirst = false;
	}
	close_json_array(PRINT_ANY, ">");
	newline(rd);
	return MNL_CB_OK;
}

static int stat_qp_show_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	const char *name;
	uint32_t idx;
	int ret;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_STAT_COUNTER])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_STAT_COUNTER];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = res_counter_line(rd, name, idx, nla_line);
		if (ret != MNL_CB_OK)
			break;
	}

	return ret;
}

static const struct filters stat_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "cntn", .is_number = true },
	{ .name = "lqpn", .is_number = true },
	{ .name = "pid", .is_number = true },
	{ .name = "qp-type", .is_number = false },
};

static int stat_qp_show_one_link(struct rd *rd)
{
	int flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	uint32_t seq;
	int ret;

	if (!rd->port_idx)
		return 0;

	ret = rd_build_filter(rd, stat_valid_filters);
	if (ret)
		return ret;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_GET, &seq, flags);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	ret = rd_recv_msg(rd, stat_qp_show_parse_cb, rd, seq);
	return ret;
}

static int stat_qp_show_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_qp_show_one_link, false);
}

static int stat_qp_show(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_qp_show_link },
		{ "link",	stat_qp_show_link },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_qp_set_link_auto_sendmsg(struct rd *rd, uint32_t mask)
{
	uint32_t seq;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_SET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));

	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_MODE,
			 RDMA_COUNTER_MODE_AUTO);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_AUTO_MODE_MASK, mask);

	return rd_sendrecv_msg(rd, seq);
}

static int stat_get_auto_mode_mask(struct rd *rd)
{
	char *modes = rd_argv(rd), *mode, *saved_ptr;
	const char *delim = ",";
	int mask = 0, found, i;

	if (!modes)
		return mask;

	mode = strtok_r(modes, delim, &saved_ptr);
	do {
		if (!mode)
			break;

		found = false;
		for (i = 0;  auto_params[i].name != NULL; i++) {
			if (!strcmp(mode, auto_params[i].name)) {
				mask |= auto_params[i].attr;
				found = true;
				break;
			}
		}

		if (!found) {
			pr_err("Unknown auto mode '%s'.\n", mode);
			mask = 0;
			break;
		}

		mode = strtok_r(NULL, delim, &saved_ptr);
	} while(1);

	if (mask)
		rd_arg_inc(rd);

	return mask;
}

static int stat_one_qp_set_link_auto(struct rd *rd)
{
	int auto_mask = 0;

	if (!rd_argc(rd))
		return -EINVAL;

	if (!strcmpx(rd_argv(rd), "off")) {
		rd_arg_inc(rd);
		return stat_qp_set_link_auto_sendmsg(rd, 0);
	}

	auto_mask = stat_get_auto_mode_mask(rd);
	if (!auto_mask || !rd_argc(rd))
		return -EINVAL;

	if (!strcmpx(rd_argv(rd), "on")) {
		rd_arg_inc(rd);
		return stat_qp_set_link_auto_sendmsg(rd, auto_mask);
	} else {
		pr_err("Unknown parameter '%s'.\n", rd_argv(rd));
		return -EINVAL;
	}
}

static int stat_one_qp_set_link(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_one_qp_link_get_mode },
		{ "auto",	stat_one_qp_set_link_auto },
		{ 0 }
	};

	if (!rd->port_idx)
		return 0;

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_qp_set_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_one_qp_set_link, false);
}

static int stat_qp_set(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_help },
		{ "link",	stat_qp_set_link },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_get_arg(struct rd *rd, const char *arg)
{
	int value = 0;
	char *endp;

	if (strcmpx(rd_argv(rd), arg) != 0)
		return -EINVAL;

	rd_arg_inc(rd);

	if (rd_is_multiarg(rd)) {
		pr_err("The parameter %s shouldn't include range\n", arg);
		return -EINVAL;
	}

	value = strtol(rd_argv(rd), &endp, 10);
	rd_arg_inc(rd);

	return value;
}

static int stat_one_qp_bind(struct rd *rd)
{
	int lqpn = 0, cntn = 0, ret;
	uint32_t seq;

	if (rd_no_arg(rd)) {
		stat_help(rd);
		return -EINVAL;
	}

	ret = rd_build_filter(rd, stat_valid_filters);
	if (ret)
		return ret;

	lqpn = stat_get_arg(rd, "lqpn");
	if (lqpn < 0)
		return lqpn;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_SET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));

	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_MODE,
			 RDMA_COUNTER_MODE_MANUAL);

	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_RES_LQPN, lqpn);

	if (rd_argc(rd)) {
		cntn = stat_get_arg(rd, "cntn");
		if (cntn < 0)
			return cntn;

		mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_COUNTER_ID,
				 cntn);
	}

	return rd_sendrecv_msg(rd, seq);
}

static int do_stat_qp_unbind_lqpn(struct rd *rd, uint32_t cntn, uint32_t lqpn)
{
	uint32_t seq;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_DEL,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));

	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_MODE,
			 RDMA_COUNTER_MODE_MANUAL);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_COUNTER_ID, cntn);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_RES_LQPN, lqpn);

	return rd_sendrecv_msg(rd, seq);
}

static int stat_get_counter_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	uint32_t lqpn, cntn;
	int err;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);

	if (!tb[RDMA_NLDEV_ATTR_STAT_COUNTER_ID])
		return MNL_CB_ERROR;
	cntn = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_STAT_COUNTER_ID]);

	nla_table = tb[RDMA_NLDEV_ATTR_RES_QP];
	if (!nla_table)
		return MNL_CB_ERROR;

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!nla_line[RDMA_NLDEV_ATTR_RES_LQPN])
			return -EINVAL;

		lqpn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_LQPN]);
		err = do_stat_qp_unbind_lqpn(rd, cntn, lqpn);
		if (err)
			return MNL_CB_ERROR;
	}

	return MNL_CB_OK;
}

static int stat_one_qp_unbind(struct rd *rd)
{
	int flags = NLM_F_REQUEST | NLM_F_ACK, ret;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int lqpn = 0, cntn = 0;
	unsigned int portid;
	uint32_t seq;

	if (rd_no_arg(rd)) {
		stat_help(rd);
		return -EINVAL;
	}

	ret = rd_build_filter(rd, stat_valid_filters);
	if (ret)
		return ret;

	cntn = stat_get_arg(rd, "cntn");
	if (cntn < 0)
		return cntn;

	if (rd_argc(rd)) {
		lqpn = stat_get_arg(rd, "lqpn");
		if (lqpn < 0)
			return lqpn;
		return do_stat_qp_unbind_lqpn(rd, cntn, lqpn);
	}

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_GET, &seq, flags);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_COUNTER_ID, cntn);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;


	/* Can't use rd_recv_msg() since the callback also calls it (recursively),
	 * then rd_recv_msg() always return -1 here
	 */
	portid = mnl_socket_get_portid(rd->nl);
	ret = mnl_socket_recvfrom(rd->nl, buf, sizeof(buf));
	if (ret <= 0)
		return ret;

	ret = mnl_cb_run(buf, ret, seq, portid, stat_get_counter_parse_cb, rd);
	mnl_socket_close(rd->nl);
	if (ret != MNL_CB_OK)
		return ret;

	return 0;
}

static int stat_qp_bind_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_one_qp_bind, true);
}

static int stat_qp_bind(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_help },
		{ "link",	stat_qp_bind_link },
		{ "help",	stat_help },
		{ 0 },
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_qp_unbind_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_one_qp_unbind, true);
}

static int stat_qp_unbind(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_help },
		{ "link",	stat_qp_unbind_link },
		{ "help",	stat_help },
		{ 0 },
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_qp(struct rd *rd)
{
	const struct rd_cmd cmds[] =  {
		{ NULL,		stat_qp_show },
		{ "show",	stat_qp_show },
		{ "list",	stat_qp_show },
		{ "mode",	stat_qp_get_mode },
		{ "set",	stat_qp_set },
		{ "bind",	stat_qp_bind },
		{ "unbind",	stat_qp_unbind },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_show_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct rd *rd = data;
	const char *name;
	uint32_t port;
	int ret;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_PORT_INDEX] ||
	    !tb[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	port = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_PORT_INDEX]);
	open_json_object(NULL);
	print_color_string(PRINT_ANY, COLOR_NONE, "ifname", "link %s/", name);
	print_color_uint(PRINT_ANY, COLOR_NONE, "port", "%u ", port);
	ret = res_get_hwcounters(rd, tb[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS], true);

	newline(rd);
	return ret;
}

static int stat_show_one_link(struct rd *rd)
{
	int flags = NLM_F_REQUEST | NLM_F_ACK;
	uint32_t seq;
	int ret;

	if (!rd->port_idx)
		return 0;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_GET, &seq,  flags);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	return rd_recv_msg(rd, stat_show_parse_cb, rd, seq);
}

static int stat_show_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_show_one_link, false);
}

static int stat_show(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_show_link },
		{ "link",	stat_show_link },
		{ "mr",		stat_mr },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

int cmd_stat(struct rd *rd)
{
	const struct rd_cmd cmds[] =  {
		{ NULL,		stat_show },
		{ "show",	stat_show },
		{ "list",	stat_show },
		{ "help",	stat_help },
		{ "qp",		stat_qp },
		{ "mr",		stat_mr },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "statistic command");
}
