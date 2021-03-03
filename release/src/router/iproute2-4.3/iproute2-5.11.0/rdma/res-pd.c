// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * res-pd.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include "res.h"
#include <inttypes.h>

static int res_pd_line(struct rd *rd, const char *name, int idx,
		       struct nlattr **nla_line)
{
	uint32_t local_dma_lkey = 0, unsafe_global_rkey = 0;
	char *comm = NULL;
	uint32_t ctxn = 0;
	uint32_t pid = 0;
	uint32_t pdn = 0;
	uint64_t users;

	if (!nla_line[RDMA_NLDEV_ATTR_RES_USECNT])
		return MNL_CB_ERROR;

	if (nla_line[RDMA_NLDEV_ATTR_RES_LOCAL_DMA_LKEY])
		local_dma_lkey = mnl_attr_get_u32(
			nla_line[RDMA_NLDEV_ATTR_RES_LOCAL_DMA_LKEY]);

	users = mnl_attr_get_u64(nla_line[RDMA_NLDEV_ATTR_RES_USECNT]);
	if (rd_is_filtered_attr(rd, "users", users,
				nla_line[RDMA_NLDEV_ATTR_RES_USECNT]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_UNSAFE_GLOBAL_RKEY])
		unsafe_global_rkey = mnl_attr_get_u32(
			nla_line[RDMA_NLDEV_ATTR_RES_UNSAFE_GLOBAL_RKEY]);

	if (nla_line[RDMA_NLDEV_ATTR_RES_PID]) {
		pid = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PID]);
		comm = get_task_name(pid);
	}

	if (rd_is_filtered_attr(rd, "pid", pid,
				nla_line[RDMA_NLDEV_ATTR_RES_PID]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_CTXN])
		ctxn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_CTXN]);

	if (rd_is_filtered_attr(rd, "ctxn", ctxn,
				nla_line[RDMA_NLDEV_ATTR_RES_CTXN]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_PDN])
		pdn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PDN]);
	if (rd_is_filtered_attr(rd, "pdn", pdn,
				nla_line[RDMA_NLDEV_ATTR_RES_PDN]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME])
		/* discard const from mnl_attr_get_str */
		comm = (char *)mnl_attr_get_str(
			nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]);

	open_json_object(NULL);
	print_dev(rd, idx, name);
	res_print_uint(rd, "pdn", pdn, nla_line[RDMA_NLDEV_ATTR_RES_PDN]);
	print_key(rd, "local_dma_lkey", local_dma_lkey,
		  nla_line[RDMA_NLDEV_ATTR_RES_LOCAL_DMA_LKEY]);
	res_print_uint(rd, "users", users,
		       nla_line[RDMA_NLDEV_ATTR_RES_USECNT]);
	print_key(rd, "unsafe_global_rkey", unsafe_global_rkey,
		  nla_line[RDMA_NLDEV_ATTR_RES_UNSAFE_GLOBAL_RKEY]);
	res_print_uint(rd, "ctxn", ctxn, nla_line[RDMA_NLDEV_ATTR_RES_CTXN]);
	res_print_uint(rd, "pid", pid, nla_line[RDMA_NLDEV_ATTR_RES_PID]);
	print_comm(rd, comm, nla_line);

	print_driver_table(rd, nla_line[RDMA_NLDEV_ATTR_DRIVER]);
	newline(rd);

out:	if (nla_line[RDMA_NLDEV_ATTR_RES_PID])
		free(comm);
	return MNL_CB_OK;
}

int res_pd_idx_parse_cb(const struct nlmsghdr *nlh, void *data)
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

	return res_pd_line(rd, name, idx, tb);
}

int res_pd_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	int ret = MNL_CB_OK;
	const char *name;
	uint32_t idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_RES_PD])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_RES_PD];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = res_pd_line(rd, name, idx, nla_line);
		if (ret != MNL_CB_OK)
			break;
	}
	return ret;
}
