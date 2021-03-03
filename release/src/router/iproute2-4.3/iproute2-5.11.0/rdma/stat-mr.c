// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * stat-mr.c     RDMA tool
 * Authors:     Erez Alfasi <ereza@mellanox.com>
 */

#include "res.h"
#include "stat.h"
#include <inttypes.h>

static int stat_mr_line(struct rd *rd, const char *name, int idx,
			struct nlattr **nla_line)
{
	uint32_t mrn = 0;
	int ret;

	if (nla_line[RDMA_NLDEV_ATTR_RES_MRN])
		mrn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_MRN]);
	if (rd_is_filtered_attr(rd, "mrn", mrn,
				nla_line[RDMA_NLDEV_ATTR_RES_MRN]))
		goto out;

	open_json_object(NULL);
	print_dev(rd, idx, name);
	res_print_uint(rd, "mrn", mrn, nla_line[RDMA_NLDEV_ATTR_RES_MRN]);

	if (nla_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS]) {
		ret = res_get_hwcounters(
			rd, nla_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS], true);
		if (ret != MNL_CB_OK)
			return ret;
	}

	newline(rd);
out:
	return MNL_CB_OK;
}

int stat_mr_idx_parse_cb(const struct nlmsghdr *nlh, void *data)
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

	return stat_mr_line(rd, name, idx, tb);
}

int stat_mr_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	int ret = MNL_CB_OK;
	const char *name;
	uint32_t idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_RES_MR])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_RES_MR];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = stat_mr_line(rd, name, idx, nla_line);
		if (ret != MNL_CB_OK)
			break;
	}
	return ret;
}
