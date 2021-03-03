// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * link.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include "rdma.h"

static int link_help(struct rd *rd)
{
	pr_out("Usage: %s link show [DEV/PORT_INDEX]\n", rd->filename);
	pr_out("Usage: %s link add NAME type TYPE netdev NETDEV\n",
	       rd->filename);
	pr_out("Usage: %s link delete NAME\n", rd->filename);
	return 0;
}

static const char *caps_to_str(uint32_t idx)
{
#define RDMA_PORT_FLAGS_LOW(x) \
	x(RESERVED, 0) \
	x(SM, 1) \
	x(NOTICE, 2) \
	x(TRAP, 3) \
	x(OPT_IPD, 4) \
	x(AUTO_MIGR, 5) \
	x(SL_MAP, 6) \
	x(MKEY_NVRAM, 7) \
	x(PKEY_NVRAM, 8) \
	x(LED_INFO, 9) \
	x(SM_DISABLED, 10) \
	x(SYS_IMAGE_GUID, 11) \
	x(PKEY_SW_EXT_PORT_TRAP, 12) \
	x(CABLE_INFO, 13) \
	x(EXTENDED_SPEEDS, 14) \
	x(CAP_MASK2, 15) \
	x(CM, 16) \
	x(SNMP_TUNNEL, 17) \
	x(REINIT, 18) \
	x(DEVICE_MGMT, 19) \
	x(VENDOR_CLASS, 20) \
	x(DR_NOTICE, 21) \
	x(CAP_MASK_NOTICE, 22) \
	x(BOOT_MGMT, 23) \
	x(LINK_LATENCY, 24) \
	x(CLIENT_REG, 25) \
	x(OTHER_LOCAL_CHANGES, 26) \
	x(LINK_SPPED_WIDTH, 27) \
	x(VENDOR_SPECIFIC_MADS, 28) \
	x(MULT_PKER_TRAP, 29) \
	x(MULT_FDB, 30) \
	x(HIERARCHY_INFO, 31)

#define RDMA_PORT_FLAGS_HIGH(x) \
	x(SET_NODE_DESC, 0) \
	x(EXT_INFO, 1) \
	x(VIRT, 2) \
	x(SWITCH_POR_STATE_TABLE, 3) \
	x(LINK_WIDTH_2X, 4) \
	x(LINK_SPEED_HDR, 5)

	/*
	 * Separation below is needed to allow compilation of rdmatool
	 * on 32bits systems. On such systems, C-enum is limited to be
	 * int and can't hold more than 32 bits.
	 */
	enum { RDMA_PORT_FLAGS_LOW(RDMA_BITMAP_ENUM) };
	enum { RDMA_PORT_FLAGS_HIGH(RDMA_BITMAP_ENUM) };

	static const char * const
		rdma_port_names_low[] = { RDMA_PORT_FLAGS_LOW(RDMA_BITMAP_NAMES) };
	static const char * const
		rdma_port_names_high[] = { RDMA_PORT_FLAGS_HIGH(RDMA_BITMAP_NAMES) };
	uint32_t high_idx;
	#undef RDMA_PORT_FLAGS_LOW
	#undef RDMA_PORT_FLAGS_HIGH

	if (idx < ARRAY_SIZE(rdma_port_names_low) && rdma_port_names_low[idx])
		return rdma_port_names_low[idx];

	high_idx = idx - ARRAY_SIZE(rdma_port_names_low);
	if (high_idx < ARRAY_SIZE(rdma_port_names_high) &&
	    rdma_port_names_high[high_idx])
		return rdma_port_names_high[high_idx];

	return "UNKNOWN";
}

static void link_print_caps(struct rd *rd, struct nlattr **tb)
{
	uint64_t caps;
	uint32_t idx;

	if (!tb[RDMA_NLDEV_ATTR_CAP_FLAGS])
		return;

	caps = mnl_attr_get_u64(tb[RDMA_NLDEV_ATTR_CAP_FLAGS]);

	print_color_string(PRINT_FP, COLOR_NONE, NULL, "\n    caps: <", NULL);
	open_json_array(PRINT_JSON, "caps");
	for (idx = 0; caps; idx++) {
		if (caps & 0x1)
			print_color_string(PRINT_ANY, COLOR_NONE, NULL,
					   caps >> 0x1 ? "%s, " : "%s",
					   caps_to_str(idx));
		caps >>= 0x1;
	}
	close_json_array(PRINT_ANY, ">");
}

static void link_print_subnet_prefix(struct rd *rd, struct nlattr **tb)
{
	uint64_t subnet_prefix;
	uint16_t vp[4];
	char str[32];

	if (!tb[RDMA_NLDEV_ATTR_SUBNET_PREFIX])
		return;

	subnet_prefix = mnl_attr_get_u64(tb[RDMA_NLDEV_ATTR_SUBNET_PREFIX]);
	memcpy(vp, &subnet_prefix, sizeof(uint64_t));
	snprintf(str, 32, "%04x:%04x:%04x:%04x", vp[3], vp[2], vp[1], vp[0]);
	print_color_string(PRINT_ANY, COLOR_NONE, "subnet_prefix",
			   "subnet_prefix %s ", str);
}

static void link_print_lid(struct rd *rd, struct nlattr **tb)
{
	uint32_t lid;

	if (!tb[RDMA_NLDEV_ATTR_LID])
		return;

	lid = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_LID]);
	print_color_uint(PRINT_ANY, COLOR_NONE, "lid", "lid %u ", lid);
}

static void link_print_sm_lid(struct rd *rd, struct nlattr **tb)
{
	uint32_t sm_lid;

	if (!tb[RDMA_NLDEV_ATTR_SM_LID])
		return;

	sm_lid = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_SM_LID]);
	print_color_uint(PRINT_ANY, COLOR_NONE, "sm_lid", "sm_lid %u ", sm_lid);
}

static void link_print_lmc(struct rd *rd, struct nlattr **tb)
{
	uint8_t lmc;

	if (!tb[RDMA_NLDEV_ATTR_LMC])
		return;

	lmc = mnl_attr_get_u8(tb[RDMA_NLDEV_ATTR_LMC]);
	print_color_uint(PRINT_ANY, COLOR_NONE, "lmc", "lmc %u ", lmc);
}

static const char *link_state_to_str(uint8_t link_state)
{
	static const char * const link_state_str[] = { "NOP", "DOWN",
						       "INIT", "ARMED",
						       "ACTIVE",
						       "ACTIVE_DEFER" };
	if (link_state < ARRAY_SIZE(link_state_str))
		return link_state_str[link_state];
	return "UNKNOWN";
}

static void link_print_state(struct rd *rd, struct nlattr **tb)
{
	uint8_t state;

	if (!tb[RDMA_NLDEV_ATTR_PORT_STATE])
		return;

	state = mnl_attr_get_u8(tb[RDMA_NLDEV_ATTR_PORT_STATE]);
	print_color_string(PRINT_ANY, COLOR_NONE, "state", "state %s ",
			   link_state_to_str(state));
}

static const char *phys_state_to_str(uint8_t phys_state)
{
	static const char * const phys_state_str[] = { "NOP", "SLEEP",
						       "POLLING", "DISABLED",
						       "ARMED", "LINK_UP",
						       "LINK_ERROR_RECOVER",
						       "PHY_TEST", "UNKNOWN",
						       "OPA_OFFLINE",
						       "UNKNOWN", "OPA_TEST" };
	if (phys_state < ARRAY_SIZE(phys_state_str))
		return phys_state_str[phys_state];
	return "UNKNOWN";
};

static void link_print_phys_state(struct rd *rd, struct nlattr **tb)
{
	uint8_t phys_state;

	if (!tb[RDMA_NLDEV_ATTR_PORT_PHYS_STATE])
		return;

	phys_state = mnl_attr_get_u8(tb[RDMA_NLDEV_ATTR_PORT_PHYS_STATE]);
	print_color_string(PRINT_ANY, COLOR_NONE, "physical_state",
			   "physical_state %s ", phys_state_to_str(phys_state));
}

static void link_print_netdev(struct rd *rd, struct nlattr **tb)
{
	const char *netdev_name;
	uint32_t idx;

	if (!tb[RDMA_NLDEV_ATTR_NDEV_NAME] || !tb[RDMA_NLDEV_ATTR_NDEV_INDEX])
		return;

	netdev_name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_NDEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_NDEV_INDEX]);
	print_color_string(PRINT_ANY, COLOR_NONE, "netdev", "netdev %s ",
			   netdev_name);
	print_color_uint(PRINT_ANY, COLOR_NONE, "netdev_index",
			 rd->show_details ? "netdev_index %u " : "", idx);
}

static int link_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct rd *rd = data;
	uint32_t port, idx;
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

	open_json_object(NULL);
	print_color_uint(PRINT_JSON, COLOR_NONE, "ifindex", NULL, idx);
	print_color_string(PRINT_ANY, COLOR_NONE, "ifname", "link %s/", name);
	print_color_uint(PRINT_ANY, COLOR_NONE, "port", "%u ", port);
	link_print_subnet_prefix(rd, tb);
	link_print_lid(rd, tb);
	link_print_sm_lid(rd, tb);
	link_print_lmc(rd, tb);
	link_print_state(rd, tb);
	link_print_phys_state(rd, tb);
	link_print_netdev(rd, tb);
	if (rd->show_details)
		link_print_caps(rd, tb);

	newline(rd);
	return MNL_CB_OK;
}

static int link_no_args(struct rd *rd)
{
	uint32_t seq;
	int ret;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_PORT_GET, &seq,
		       (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	ret = rd_recv_msg(rd, link_parse_cb, rd, seq);
	return ret;
}

static int link_one_show(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		link_no_args},
		{ 0 }
	};

	if (!rd->port_idx)
		return 0;

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int link_show(struct rd *rd)
{
	return rd_exec_link(rd, link_one_show, true);
}

static int link_add_netdev(struct rd *rd)
{
	char *link_netdev;
	uint32_t seq;

	if (rd_no_arg(rd)) {
		pr_err("Please provide a net device name.\n");
		return -EINVAL;
	}

	link_netdev = rd_argv(rd);
	rd_prepare_msg(rd, RDMA_NLDEV_CMD_NEWLINK, &seq,
		       (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_strz(rd->nlh, RDMA_NLDEV_ATTR_DEV_NAME, rd->link_name);
	mnl_attr_put_strz(rd->nlh, RDMA_NLDEV_ATTR_LINK_TYPE, rd->link_type);
	mnl_attr_put_strz(rd->nlh, RDMA_NLDEV_ATTR_NDEV_NAME, link_netdev);
	return rd_sendrecv_msg(rd, seq);
}

static int link_add_type(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		link_help},
		{ "netdev",	link_add_netdev},
		{ 0 }
	};

	if (rd_no_arg(rd)) {
		pr_err("Please provide a link type name.\n");
		return -EINVAL;
	}
	rd->link_type = rd_argv(rd);
	rd_arg_inc(rd);
	return rd_exec_cmd(rd, cmds, "parameter");
}

static int link_add(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		link_help},
		{ "type",	link_add_type},
		{ 0 }
	};

	if (rd_no_arg(rd)) {
		pr_err("Please provide a link name to add.\n");
		return -EINVAL;
	}
	rd->link_name = rd_argv(rd);
	rd_arg_inc(rd);

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int _link_del(struct rd *rd)
{
	uint32_t seq;

	if (!rd_no_arg(rd)) {
		pr_err("Unknown parameter %s\n", rd_argv(rd));
		return -EINVAL;
	}
	rd_prepare_msg(rd, RDMA_NLDEV_CMD_DELLINK, &seq,
		       (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	return rd_sendrecv_msg(rd, seq);
}

static int link_del(struct rd *rd)
{
	return rd_exec_require_dev(rd, _link_del);
}

int cmd_link(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		link_show },
		{ "add",	link_add },
		{ "delete",	link_del },
		{ "show",	link_show },
		{ "list",	link_show },
		{ "help",	link_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "link command");
}
