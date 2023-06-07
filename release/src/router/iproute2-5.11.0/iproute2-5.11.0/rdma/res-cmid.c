// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * res-cmid.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include "res.h"
#include <inttypes.h>

static const char *cm_id_state_to_str(uint8_t idx)
{
	static const char *const cm_id_states_str[] = {
		"IDLE",		  "ADDR_QUERY",     "ADDR_RESOLVED",
		"ROUTE_QUERY",    "ROUTE_RESOLVED", "CONNECT",
		"DISCONNECT",     "ADDR_BOUND",     "LISTEN",
		"DEVICE_REMOVAL", "DESTROYING"
	};

	if (idx < ARRAY_SIZE(cm_id_states_str))
		return cm_id_states_str[idx];
	return "UNKNOWN";
}

static const char *cm_id_ps_to_str(uint32_t ps)
{
	switch (ps) {
	case RDMA_PS_IPOIB:
		return "IPoIB";
	case RDMA_PS_IB:
		return "IPoIB";
	case RDMA_PS_TCP:
		return "TCP";
	case RDMA_PS_UDP:
		return "UDP";
	default:
		return "---";
	}
}

static void print_cm_id_state(struct rd *rd, uint8_t state)
{
	print_color_string(PRINT_ANY, COLOR_NONE, "state", "state %s ",
			   cm_id_state_to_str(state));
}

static void print_ps(struct rd *rd, uint32_t ps)
{
	print_color_string(PRINT_ANY, COLOR_NONE, "ps", "ps %s ",
			   cm_id_ps_to_str(ps));
}

static void print_ipaddr(struct rd *rd, const char *key, char *addrstr,
			 uint16_t port)
{
	int name_size = INET6_ADDRSTRLEN + strlen(":65535");
	char json_name[name_size];

	snprintf(json_name, name_size, "%s:%u", addrstr, port);
	print_color_string(PRINT_ANY, COLOR_NONE, key, key, json_name);
	print_color_string(PRINT_FP, COLOR_NONE, NULL, " %s:", addrstr);
	print_color_uint(PRINT_FP, COLOR_NONE, NULL, "%u ", port);
}

static int ss_ntop(struct nlattr *nla_line, char *addr_str, uint16_t *port)
{
	struct __kernel_sockaddr_storage *addr;

	addr = (struct __kernel_sockaddr_storage *)mnl_attr_get_payload(
		nla_line);
	switch (addr->ss_family) {
	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *)addr;

		if (!inet_ntop(AF_INET, (const void *)&sin->sin_addr, addr_str,
			       INET6_ADDRSTRLEN))
			return -EINVAL;
		*port = ntohs(sin->sin_port);
		break;
	}
	case AF_INET6: {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;

		if (!inet_ntop(AF_INET6, (const void *)&sin6->sin6_addr,
			       addr_str, INET6_ADDRSTRLEN))
			return -EINVAL;
		*port = ntohs(sin6->sin6_port);
		break;
	}
	default:
		return -EINVAL;
	}
	return 0;
}
static int res_cm_id_line(struct rd *rd, const char *name, int idx,
		       struct nlattr **nla_line)
{
	char src_addr_str[INET6_ADDRSTRLEN];
	char dst_addr_str[INET6_ADDRSTRLEN];
	uint16_t src_port, dst_port;
	uint32_t port = 0, pid = 0;
	uint8_t type = 0, state;
	uint32_t lqpn = 0, ps;
	uint32_t cm_idn = 0;
	char *comm = NULL;

	if (!nla_line[RDMA_NLDEV_ATTR_RES_STATE] ||
	    !nla_line[RDMA_NLDEV_ATTR_RES_PS])
		return MNL_CB_ERROR;

	if (nla_line[RDMA_NLDEV_ATTR_PORT_INDEX])
		port = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_PORT_INDEX]);

	if (port && port != rd->port_idx)
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_LQPN])
		lqpn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_LQPN]);

	if (rd_is_filtered_attr(rd, "lqpn", lqpn,
				nla_line[RDMA_NLDEV_ATTR_RES_LQPN]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_TYPE])
		type = mnl_attr_get_u8(nla_line[RDMA_NLDEV_ATTR_RES_TYPE]);
	if (rd_is_string_filtered_attr(rd, "qp-type", qp_types_to_str(type),
				       nla_line[RDMA_NLDEV_ATTR_RES_TYPE]))
		goto out;

	ps = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PS]);
	if (rd_is_string_filtered_attr(rd, "ps", cm_id_ps_to_str(ps),
				       nla_line[RDMA_NLDEV_ATTR_RES_PS]))
		goto out;

	state = mnl_attr_get_u8(nla_line[RDMA_NLDEV_ATTR_RES_STATE]);
	if (rd_is_string_filtered_attr(rd, "state", cm_id_state_to_str(state),
				       nla_line[RDMA_NLDEV_ATTR_RES_STATE]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_SRC_ADDR])
		if (ss_ntop(nla_line[RDMA_NLDEV_ATTR_RES_SRC_ADDR],
			    src_addr_str, &src_port))
			goto out;
	if (rd_is_string_filtered_attr(rd, "src-addr", src_addr_str,
				       nla_line[RDMA_NLDEV_ATTR_RES_SRC_ADDR]))
		goto out;
	if (rd_is_filtered_attr(rd, "src-port", src_port,
				nla_line[RDMA_NLDEV_ATTR_RES_SRC_ADDR]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_DST_ADDR])
		if (ss_ntop(nla_line[RDMA_NLDEV_ATTR_RES_DST_ADDR],
			    dst_addr_str, &dst_port))
			goto out;
	if (rd_is_string_filtered_attr(rd, "dst-addr", dst_addr_str,
				       nla_line[RDMA_NLDEV_ATTR_RES_DST_ADDR]))
		goto out;
	if (rd_is_filtered_attr(rd, "dst-port", dst_port,
				nla_line[RDMA_NLDEV_ATTR_RES_DST_ADDR]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_PID]) {
		pid = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PID]);
		comm = get_task_name(pid);
	}

	if (rd_is_filtered_attr(rd, "pid", pid,
				nla_line[RDMA_NLDEV_ATTR_RES_PID]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_CM_IDN])
		cm_idn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_CM_IDN]);
	if (rd_is_filtered_attr(rd, "cm-idn", cm_idn,
				nla_line[RDMA_NLDEV_ATTR_RES_CM_IDN]))
		goto out;

	if (nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]) {
		/* discard const from mnl_attr_get_str */
		comm = (char *)mnl_attr_get_str(
			nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]);
	}

	open_json_object(NULL);
	print_link(rd, idx, name, port, nla_line);
	res_print_uint(rd, "cm-idn", cm_idn,
		       nla_line[RDMA_NLDEV_ATTR_RES_CM_IDN]);
	res_print_uint(rd, "lqpn", lqpn, nla_line[RDMA_NLDEV_ATTR_RES_LQPN]);
	if (nla_line[RDMA_NLDEV_ATTR_RES_TYPE])
		print_qp_type(rd, type);
	print_cm_id_state(rd, state);
	print_ps(rd, ps);
	res_print_uint(rd, "pid", pid, nla_line[RDMA_NLDEV_ATTR_RES_PID]);
	print_comm(rd, comm, nla_line);

	if (nla_line[RDMA_NLDEV_ATTR_RES_SRC_ADDR])
		print_ipaddr(rd, "src-addr", src_addr_str, src_port);
	if (nla_line[RDMA_NLDEV_ATTR_RES_DST_ADDR])
		print_ipaddr(rd, "dst-addr", dst_addr_str, dst_port);

	print_driver_table(rd, nla_line[RDMA_NLDEV_ATTR_DRIVER]);
	newline(rd);

out:	if (nla_line[RDMA_NLDEV_ATTR_RES_PID])
		free(comm);
	return MNL_CB_OK;
}

int res_cm_id_idx_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct rd *rd = data;
	const char *name;
	int idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);

	return res_cm_id_line(rd, name, idx, tb);
}

int res_cm_id_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	int ret = MNL_CB_OK;
	const char *name;
	int idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_RES_CM_ID])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_RES_CM_ID];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = res_cm_id_line(rd, name, idx, nla_line);
		if (ret != MNL_CB_OK)
			break;
	}
	return ret;
}
