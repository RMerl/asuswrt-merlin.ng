// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * dev.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */

#include <fcntl.h>
#include "rdma.h"

static int dev_help(struct rd *rd)
{
	pr_out("Usage: %s dev show [DEV]\n", rd->filename);
	pr_out("       %s dev set [DEV] name DEVNAME\n", rd->filename);
	pr_out("       %s dev set [DEV] netns NSNAME\n", rd->filename);
	pr_out("       %s dev set [DEV] adaptive-moderation [on|off]\n", rd->filename);
	return 0;
}

static const char *dev_caps_to_str(uint32_t idx)
{
#define RDMA_DEV_FLAGS_LOW(x) \
	x(RESIZE_MAX_WR, 0) \
	x(BAD_PKEY_CNTR, 1) \
	x(BAD_QKEY_CNTR, 2) \
	x(RAW_MULTI, 3) \
	x(AUTO_PATH_MIG, 4) \
	x(CHANGE_PHY_PORT, 5) \
	x(UD_AV_PORT_ENFORCE_PORT_ENFORCE, 6) \
	x(CURR_QP_STATE_MOD, 7) \
	x(SHUTDOWN_PORT, 8) \
	x(INIT_TYPE, 9) \
	x(PORT_ACTIVE_EVENT, 10) \
	x(SYS_IMAGE_GUID, 11) \
	x(RC_RNR_NAK_GEN, 12) \
	x(SRQ_RESIZE, 13) \
	x(N_NOTIFY_CQ, 14) \
	x(LOCAL_DMA_LKEY, 15) \
	x(MEM_WINDOW, 17) \
	x(UD_IP_CSUM, 18) \
	x(UD_TSO, 19) \
	x(XRC, 20) \
	x(MEM_MGT_EXTENSIONS, 21) \
	x(BLOCK_MULTICAST_LOOPBACK, 22) \
	x(MEM_WINDOW_TYPE_2A, 23) \
	x(MEM_WINDOW_TYPE_2B, 24) \
	x(RC_IP_CSUM, 25) \
	x(RAW_IP_CSUM, 26) \
	x(CROSS_CHANNEL, 27) \
	x(MANAGED_FLOW_STEERING, 29) \
	x(SIGNATURE_HANDOVER, 30) \
	x(ON_DEMAND_PAGING, 31)

#define RDMA_DEV_FLAGS_HIGH(x) \
	x(SG_GAPS_REG, 0) \
	x(VIRTUAL_FUNCTION, 1) \
	x(RAW_SCATTER_FCS, 2) \
	x(RDMA_NETDEV_OPA_VNIC, 3) \
	x(PCI_WRITE_END_PADDING, 4)

	/*
	 * Separation below is needed to allow compilation of rdmatool
	 * on 32bits systems. On such systems, C-enum is limited to be
	 * int and can't hold more than 32 bits.
	 */
	enum { RDMA_DEV_FLAGS_LOW(RDMA_BITMAP_ENUM) };
	enum { RDMA_DEV_FLAGS_HIGH(RDMA_BITMAP_ENUM) };

	static const char * const
		rdma_dev_names_low[] = { RDMA_DEV_FLAGS_LOW(RDMA_BITMAP_NAMES) };
	static const char * const
		rdma_dev_names_high[] = { RDMA_DEV_FLAGS_HIGH(RDMA_BITMAP_NAMES) };
	uint32_t high_idx;
	#undef RDMA_DEV_FLAGS_LOW
	#undef RDMA_DEV_FLAGS_HIGH

	if (idx < ARRAY_SIZE(rdma_dev_names_low) && rdma_dev_names_low[idx])
		return rdma_dev_names_low[idx];

	high_idx = idx - ARRAY_SIZE(rdma_dev_names_low);
	if (high_idx <  ARRAY_SIZE(rdma_dev_names_high) &&
	    rdma_dev_names_high[high_idx])
		return rdma_dev_names_high[high_idx];

	return "UNKNOWN";
}

static void dev_print_caps(struct rd *rd, struct nlattr **tb)
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
					   dev_caps_to_str(idx));
		caps >>= 0x1;
	}
	close_json_array(PRINT_ANY, ">");
}

static void dev_print_fw(struct rd *rd, struct nlattr **tb)
{
	const char *str;
	if (!tb[RDMA_NLDEV_ATTR_FW_VERSION])
		return;

	str = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_FW_VERSION]);
	print_color_string(PRINT_ANY, COLOR_NONE, "fw", "fw %s ", str);
}

static void dev_print_node_guid(struct rd *rd, struct nlattr **tb)
{
	uint64_t node_guid;
	uint16_t vp[4];
	char str[32];

	if (!tb[RDMA_NLDEV_ATTR_NODE_GUID])
		return;

	node_guid = mnl_attr_get_u64(tb[RDMA_NLDEV_ATTR_NODE_GUID]);
	memcpy(vp, &node_guid, sizeof(uint64_t));
	snprintf(str, 32, "%04x:%04x:%04x:%04x", vp[3], vp[2], vp[1], vp[0]);
	print_color_string(PRINT_ANY, COLOR_NONE, "node_guid", "node_guid %s ",
			   str);
}

static void dev_print_sys_image_guid(struct rd *rd, struct nlattr **tb)
{
	uint64_t sys_image_guid;
	uint16_t vp[4];
	char str[32];

	if (!tb[RDMA_NLDEV_ATTR_SYS_IMAGE_GUID])
		return;

	sys_image_guid = mnl_attr_get_u64(tb[RDMA_NLDEV_ATTR_SYS_IMAGE_GUID]);
	memcpy(vp, &sys_image_guid, sizeof(uint64_t));
	snprintf(str, 32, "%04x:%04x:%04x:%04x", vp[3], vp[2], vp[1], vp[0]);
	print_color_string(PRINT_ANY, COLOR_NONE, "sys_image_guid",
			   "sys_image_guid %s ", str);
}

static void dev_print_dim_setting(struct rd *rd, struct nlattr **tb)
{
	uint8_t dim_setting;

	if (!tb[RDMA_NLDEV_ATTR_DEV_DIM])
		return;

	dim_setting = mnl_attr_get_u8(tb[RDMA_NLDEV_ATTR_DEV_DIM]);
	if (dim_setting > 1)
		return;

	print_on_off(PRINT_ANY, "adaptive-moderation", "adaptive-moderation %s ", dim_setting);

}

static const char *node_type_to_str(uint8_t node_type)
{
	static const char * const node_type_str[] = { "unknown", "ca",
						      "switch", "router",
						      "rnic", "usnic",
						      "usnic_udp",
						      "unspecified" };
	if (node_type < ARRAY_SIZE(node_type_str))
		return node_type_str[node_type];
	return "unknown";
}

static void dev_print_node_type(struct rd *rd, struct nlattr **tb)
{
	const char *node_str;
	uint8_t node_type;

	if (!tb[RDMA_NLDEV_ATTR_DEV_NODE_TYPE])
		return;

	node_type = mnl_attr_get_u8(tb[RDMA_NLDEV_ATTR_DEV_NODE_TYPE]);
	node_str = node_type_to_str(node_type);
	print_color_string(PRINT_ANY, COLOR_NONE, "node_type", "node_type %s ",
			   node_str);
}

static int dev_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct rd *rd = data;
	const char *name;
	uint32_t idx;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME])
		return MNL_CB_ERROR;
	open_json_object(NULL);
	idx =  mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	print_color_uint(PRINT_ANY, COLOR_NONE, "ifindex", "%u: ", idx);
	print_color_string(PRINT_ANY, COLOR_NONE, "ifname", "%s: ", name);

	dev_print_node_type(rd, tb);
	dev_print_fw(rd, tb);
	dev_print_node_guid(rd, tb);
	dev_print_sys_image_guid(rd, tb);
	if (rd->show_details) {
		dev_print_dim_setting(rd, tb);
		dev_print_caps(rd, tb);
	}

	newline(rd);
	return MNL_CB_OK;
}

static int dev_no_args(struct rd *rd)
{
	uint32_t seq;
	int ret;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_GET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	ret = rd_recv_msg(rd, dev_parse_cb, rd, seq);
	return ret;
}

static int dev_one_show(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		dev_no_args},
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int dev_set_name(struct rd *rd)
{
	uint32_t seq;

	if (rd_no_arg(rd)) {
		pr_err("Please provide device new name.\n");
		return -EINVAL;
	}

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_SET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_strz(rd->nlh, RDMA_NLDEV_ATTR_DEV_NAME, rd_argv(rd));

	return rd_sendrecv_msg(rd, seq);
}

static int dev_set_netns(struct rd *rd)
{
	char *netns_path;
	uint32_t seq;
	int netns;
	int ret;

	if (rd_no_arg(rd)) {
		pr_err("Please provide device name.\n");
		return -EINVAL;
	}

	if (asprintf(&netns_path, "%s/%s", NETNS_RUN_DIR, rd_argv(rd)) < 0)
		return -ENOMEM;

	netns = open(netns_path, O_RDONLY | O_CLOEXEC);
	if (netns < 0) {
		fprintf(stderr, "Cannot open network namespace \"%s\": %s\n",
			rd_argv(rd), strerror(errno));
		ret = -EINVAL;
		goto done;
	}

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_SET,
		       &seq, (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_NET_NS_FD, netns);
	ret = rd_sendrecv_msg(rd, seq);
	close(netns);
done:
	free(netns_path);
	return ret;
}

static int dev_set_dim_sendmsg(struct rd *rd, uint8_t dim_setting)
{
	uint32_t seq;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_SET, &seq,
		       (NLM_F_REQUEST | NLM_F_ACK));
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u8(rd->nlh, RDMA_NLDEV_ATTR_DEV_DIM, dim_setting);

	return rd_sendrecv_msg(rd, seq);
}

static int dev_set_dim_off(struct rd *rd)
{
	return dev_set_dim_sendmsg(rd, 0);
}

static int dev_set_dim_on(struct rd *rd)
{
	return dev_set_dim_sendmsg(rd, 1);
}

static int dev_set_dim(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		dev_help},
		{ "on",		dev_set_dim_on},
		{ "off",	dev_set_dim_off},
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int dev_one_set(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		dev_help},
		{ "name",	dev_set_name},
		{ "netns",	dev_set_netns},
		{ "adaptive-moderation",	dev_set_dim},
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int dev_show(struct rd *rd)
{
	return rd_exec_dev(rd, dev_one_show);
}

static int dev_set(struct rd *rd)
{
	return rd_exec_require_dev(rd, dev_one_set);
}

int cmd_dev(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		dev_show },
		{ "show",	dev_show },
		{ "list",	dev_show },
		{ "set",	dev_set },
		{ "help",	dev_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "dev command");
}
