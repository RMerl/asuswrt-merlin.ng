/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * res.h	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */
#ifndef _RDMA_TOOL_RES_H_
#define _RDMA_TOOL_RES_H_

#include "rdma.h"

int _res_send_msg(struct rd *rd, uint32_t command, mnl_cb_t callback);
int _res_send_idx_msg(struct rd *rd, uint32_t command, mnl_cb_t callback,
		      uint32_t idx, uint32_t id);

int res_pd_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_pd_idx_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_mr_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_mr_idx_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_cq_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_cq_idx_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_cm_id_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_cm_id_idx_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_qp_parse_cb(const struct nlmsghdr *nlh, void *data);
int res_qp_idx_parse_cb(const struct nlmsghdr *nlh, void *data);

static inline uint32_t res_get_command(uint32_t command, struct rd *rd)
{
	if (!rd->show_raw)
		return command;

	switch (command) {
	case RDMA_NLDEV_CMD_RES_QP_GET:
		return RDMA_NLDEV_CMD_RES_QP_GET_RAW;
	case RDMA_NLDEV_CMD_RES_CQ_GET:
		return RDMA_NLDEV_CMD_RES_CQ_GET_RAW;
	case RDMA_NLDEV_CMD_RES_MR_GET:
		return RDMA_NLDEV_CMD_RES_MR_GET_RAW;
	default:
		return command;
	}
}

#define RES_FUNC(name, command, valid_filters, strict_port, id)                        \
	static inline int _##name(struct rd *rd)                                       \
	{                                                                              \
		uint32_t idx, _command;                                                \
		int ret;                                                               \
		_command = res_get_command(command, rd);			       \
		if (id) {                                                              \
			ret = rd_doit_index(rd, &idx);                                 \
			if (ret) {                                                     \
				rd->suppress_errors = true;                            \
				ret = _res_send_idx_msg(rd, _command,                  \
							name##_idx_parse_cb,           \
							idx, id);                      \
				if (!ret || rd->show_raw)                              \
					return ret;                                    \
				/* Fallback for old systems without .doit callbacks.   \
				 * Kernel that supports raw, for sure supports doit.   \
				 */						       \
			}                                                              \
		}                                                                      \
		return _res_send_msg(rd, _command, name##_parse_cb);                   \
	}                                                                              \
	static inline int name(struct rd *rd)                                          \
	{                                                                              \
		int ret = rd_build_filter(rd, valid_filters);                          \
		if (ret)                                                               \
			return ret;                                                    \
		if ((uintptr_t)valid_filters != (uintptr_t)NULL) {                     \
			ret = rd_set_arg_to_devname(rd);                               \
			if (ret)                                                       \
				return ret;                                            \
		}                                                                      \
		if (strict_port)                                                       \
			return rd_exec_dev(rd, _##name);                               \
		else                                                                   \
			return rd_exec_link(rd, _##name, strict_port);                 \
	}

static const
struct filters pd_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "dev", .is_number = false },
	{ .name = "users", .is_number = true },
	{ .name = "pid", .is_number = true },
	{ .name = "ctxn", .is_number = true },
	{ .name = "pdn", .is_number = true, .is_doit = true },
	{ .name = "ctxn", .is_number = true }
};

RES_FUNC(res_pd, RDMA_NLDEV_CMD_RES_PD_GET, pd_valid_filters, true,
	 RDMA_NLDEV_ATTR_RES_PDN);

static const
struct filters mr_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "dev", .is_number = false },
	{ .name = "rkey", .is_number = true },
	{ .name = "lkey", .is_number = true },
	{ .name = "mrlen", .is_number = true },
	{ .name = "pid", .is_number = true },
	{ .name = "mrn", .is_number = true, .is_doit = true },
	{ .name = "pdn", .is_number = true }
};

RES_FUNC(res_mr, RDMA_NLDEV_CMD_RES_MR_GET, mr_valid_filters, true,
	 RDMA_NLDEV_ATTR_RES_MRN);

static const
struct filters cq_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "dev", .is_number = false },
	{ .name = "users", .is_number = true },
	{ .name = "poll-ctx", .is_number = false },
	{ .name = "pid", .is_number = true },
	{ .name = "cqn", .is_number = true, .is_doit = true },
	{ .name = "ctxn", .is_number = true }
};

RES_FUNC(res_cq, RDMA_NLDEV_CMD_RES_CQ_GET, cq_valid_filters, true,
	 RDMA_NLDEV_ATTR_RES_CQN);

static const
struct filters cm_id_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "link", .is_number = false },
	{ .name = "lqpn", .is_number = true },
	{ .name = "qp-type", .is_number = false },
	{ .name = "state", .is_number = false },
	{ .name = "ps", .is_number = false },
	{ .name = "dev-type", .is_number = false },
	{ .name = "transport-type", .is_number = false },
	{ .name = "pid", .is_number = true },
	{ .name = "src-addr", .is_number = false },
	{ .name = "src-port", .is_number = true },
	{ .name = "dst-addr", .is_number = false },
	{ .name = "dst-port", .is_number = true },
	{ .name = "cm-idn", .is_number = true, .is_doit = true }
};

RES_FUNC(res_cm_id, RDMA_NLDEV_CMD_RES_CM_ID_GET, cm_id_valid_filters, false,
	 RDMA_NLDEV_ATTR_RES_CM_IDN);

static const struct
filters qp_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "link", .is_number = false },
	{ .name = "lqpn", .is_number = true, .is_doit = true },
	{ .name = "rqpn", .is_number = true },
	{ .name = "pid",  .is_number = true },
	{ .name = "sq-psn", .is_number = true },
	{ .name = "rq-psn", .is_number = true },
	{ .name = "type", .is_number = false },
	{ .name = "path-mig-state", .is_number = false },
	{ .name = "state", .is_number = false },
	{ .name = "pdn", .is_number = true },
};

RES_FUNC(res_qp, RDMA_NLDEV_CMD_RES_QP_GET, qp_valid_filters, false,
	 RDMA_NLDEV_ATTR_RES_LQPN);

char *get_task_name(uint32_t pid);
void print_dev(struct rd *rd, uint32_t idx, const char *name);
void print_link(struct rd *rd, uint32_t idx, const char *name, uint32_t port,
		struct nlattr **nla_line);
void print_key(struct rd *rd, const char *name, uint64_t val,
	       struct nlattr *nlattr);
void res_print_uint(struct rd *rd, const char *name, uint64_t val,
		    struct nlattr *nlattr);
void print_comm(struct rd *rd, const char *str, struct nlattr **nla_line);
const char *qp_types_to_str(uint8_t idx);
void print_qp_type(struct rd *rd, uint32_t val);
#endif /* _RDMA_TOOL_RES_H_ */
