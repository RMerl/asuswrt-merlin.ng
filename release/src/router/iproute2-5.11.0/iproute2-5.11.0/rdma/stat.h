/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * stat.h        RDMA tool
 * Authors:      Mark Zhang <markz@mellanox.com>
 *		 Erez Alfasi <ereza@mellanox.com>
 */
#ifndef _RDMA_TOOL_STAT_H_
#define _RDMA_TOOL_STAT_H_

#include "rdma.h"

int res_get_hwcounters(struct rd *rd, struct nlattr *hwc_table,
		       bool print);

int stat_mr_parse_cb(const struct nlmsghdr *nlh, void *data);
int stat_mr_idx_parse_cb(const struct nlmsghdr *nlh, void *data);

static const
struct filters stat_mr_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "mrn", .is_number = true, .is_doit = true },
};

RES_FUNC(stat_mr, RDMA_NLDEV_CMD_STAT_GET, stat_mr_valid_filters, true,
	 RDMA_NLDEV_ATTR_RES_MRN);

#endif /* _RDMA_TOOL_STAT_H_ */
