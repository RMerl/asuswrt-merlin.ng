// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * res-cq.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include "res.h"
#include <inttypes.h>

static const char *poll_ctx_to_str(uint8_t idx)
{
	static const char * const cm_id_states_str[] = {
		"DIRECT", "SOFTIRQ", "WORKQUEUE", "UNBOUND_WORKQUEUE"};

	if (idx < ARRAY_SIZE(cm_id_states_str))
		return cm_id_states_str[idx];
	return "UNKNOWN";
}

static void print_poll_ctx(struct rd *rd, uint8_t poll_ctx, struct nlattr *attr)
{
	if (!attr)
		return;
	print_color_string(PRINT_ANY, COLOR_NONE, "poll-ctx", "poll-ctx %s ",
			   poll_ctx_to_str(poll_ctx));
}

static void print_cq_dim_setting(struct rd *rd, struct nlattr *attr)
{
	uint8_t dim_setting;

	if (!attr)
		return;

	dim_setting = mnl_attr_get_u8(attr);
	if (dim_setting > 1)
		return;

	print_on_off(PRINT_ANY, "adaptive-moderation", "adaptive-moderation %s ", dim_setting);
}

static int res_cq_line_raw(struct rd *rd, const char *name, int idx,
			   struct nlattr **nla_line)
{
	if (!nla_line[RDMA_NLDEV_ATTR_RES_RAW])
		return MNL_CB_ERROR;

	open_json_object(NULL);
	print_dev(rd, idx, name);
	print_raw_data(rd, nla_line);
	newline(rd);

	return MNL_CB_OK;
}

static int res_cq_line(struct rd *rd, const char *name, int idx,
		       struct nlattr **nla_line)
{
	char *comm = NULL;
	uint32_t pid = 0;
	uint8_t poll_ctx = 0;
	uint32_t ctxn = 0;
	uint32_t cqn = 0;
	uint64_t users;
	uint32_t cqe;

	if (!nla_line[RDMA_NLDEV_ATTR_RES_CQE] ||
	    !nla_line[RDMA_NLDEV_ATTR_RES_USECNT])
		return MNL_CB_ERROR;

	cqe = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_CQE]);

	users = mnl_attr_get_u64(nla_line[RDMA_NLDEV_ATTR_RES_USECNT]);
	if (rd_is_filtered_attr(rd, "users", users,
				nla_line[RDMA_NLDEV_ATTR_RES_USECNT]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_POLL_CTX])
		poll_ctx =
			mnl_attr_get_u8(nla_line[RDMA_NLDEV_ATTR_RES_POLL_CTX]);
	if (rd_is_string_filtered_attr(rd, "poll-ctx",
				       poll_ctx_to_str(poll_ctx),
				       nla_line[RDMA_NLDEV_ATTR_RES_POLL_CTX]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_PID]) {
		pid = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PID]);
		comm = get_task_name(pid);
	}

	if (rd_is_filtered_attr(rd, "pid", pid,
				nla_line[RDMA_NLDEV_ATTR_RES_PID]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_CQN])
		cqn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_CQN]);
	if (rd_is_filtered_attr(rd, "cqn", cqn,
				nla_line[RDMA_NLDEV_ATTR_RES_CQN]))
		goto out;
	if (nla_line[RDMA_NLDEV_ATTR_RES_CTXN])
		ctxn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_CTXN]);
	if (rd_is_filtered_attr(rd, "ctxn", ctxn,
				nla_line[RDMA_NLDEV_ATTR_RES_CTXN]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME])
		/* discard const from mnl_attr_get_str */
		comm = (char *)mnl_attr_get_str(
			nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]);

	open_json_object(NULL);
	print_dev(rd, idx, name);
	res_print_uint(rd, "cqn", cqn, nla_line[RDMA_NLDEV_ATTR_RES_CQN]);
	res_print_uint(rd, "cqe", cqe, nla_line[RDMA_NLDEV_ATTR_RES_CQE]);
	res_print_uint(rd, "users", users,
		       nla_line[RDMA_NLDEV_ATTR_RES_USECNT]);
	print_poll_ctx(rd, poll_ctx, nla_line[RDMA_NLDEV_ATTR_RES_POLL_CTX]);
	print_cq_dim_setting(rd, nla_line[RDMA_NLDEV_ATTR_DEV_DIM]);
	res_print_uint(rd, "ctxn", ctxn, nla_line[RDMA_NLDEV_ATTR_RES_CTXN]);
	res_print_uint(rd, "pid", pid, nla_line[RDMA_NLDEV_ATTR_RES_PID]);
	print_comm(rd, comm, nla_line);

	print_driver_table(rd, nla_line[RDMA_NLDEV_ATTR_DRIVER]);
	newline(rd);

out:	if (nla_line[RDMA_NLDEV_ATTR_RES_PID])
		free(comm);
	return MNL_CB_OK;
}

int res_cq_idx_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct rd *rd = data;
	const char *name;
	uint32_t idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);

	return (rd->show_raw) ? res_cq_line_raw(rd, name, idx, tb) :
		res_cq_line(rd, name, idx, tb);
}

int res_cq_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	int ret = MNL_CB_OK;
	const char *name;
	uint32_t idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_RES_CQ])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_RES_CQ];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = (rd->show_raw) ? res_cq_line_raw(rd, name, idx, nla_line) :
			res_cq_line(rd, name, idx, nla_line);

		if (ret != MNL_CB_OK)
			break;
	}
	return ret;
}
