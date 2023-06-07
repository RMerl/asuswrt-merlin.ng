// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019-2020  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdbool.h>

#include <ell/ell.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"

#include "mesh/mesh-defs.h"
#include "mesh/util.h"
#include "mesh/crypto.h"

#include "tools/mesh/util.h"
#include "tools/mesh/model.h"
#include "tools/mesh/keys.h"
#include "tools/mesh/mesh-db.h"
#include "tools/mesh/remote.h"
#include "tools/mesh/config-model.h"
#include "tools/mesh/cfgcli.h"

#define MIN_COMPOSITION_LEN 16
#define NO_RESPONSE 0xFFFFFFFF

/* Default timeout for getting a response to a sent config command (seconds) */
#define DEFAULT_TIMEOUT 2

struct cfg_cmd {
	uint32_t opcode;
	uint32_t rsp;
	const char *desc;
};

struct pending_req {
	struct l_timeout *timer;
	const struct cfg_cmd *cmd;
	uint16_t addr;
};

static struct l_queue *requests;
static struct l_queue *groups;

static void *send_data;
static model_send_msg_func_t send_msg;
static delete_remote_func_t mgr_del_remote;

static void *key_data;
static key_send_func_t send_key_msg;

static uint32_t rsp_timeout = DEFAULT_TIMEOUT;
static uint16_t target = UNASSIGNED_ADDRESS;
static uint32_t parms[8];

static struct cfg_cmd cmds[] = {
	{ OP_APPKEY_ADD, OP_APPKEY_STATUS, "AppKeyAdd" },
	{ OP_APPKEY_DELETE, OP_APPKEY_STATUS, "AppKeyDelete" },
	{ OP_APPKEY_GET, OP_APPKEY_LIST, "AppKeyGet" },
	{ OP_APPKEY_LIST, NO_RESPONSE, "AppKeyList" },
	{ OP_APPKEY_STATUS, NO_RESPONSE, "AppKeyStatus" },
	{ OP_APPKEY_UPDATE, OP_APPKEY_STATUS, "AppKeyUpdate" },
	{ OP_DEV_COMP_GET, OP_DEV_COMP_STATUS, "DeviceCompositionGet" },
	{ OP_DEV_COMP_STATUS, NO_RESPONSE, "DeviceCompositionStatus" },
	{ OP_CONFIG_BEACON_GET, OP_CONFIG_BEACON_STATUS, "BeaconGet" },
	{ OP_CONFIG_BEACON_SET, OP_CONFIG_BEACON_STATUS, "BeaconSet" },
	{ OP_CONFIG_BEACON_STATUS, NO_RESPONSE, "BeaconStatus" },
	{ OP_CONFIG_DEFAULT_TTL_GET, OP_CONFIG_DEFAULT_TTL_STATUS,
							"DefaultTTLGet" },
	{ OP_CONFIG_DEFAULT_TTL_SET, OP_CONFIG_DEFAULT_TTL_STATUS,
							"DefaultTTLSet" },
	{ OP_CONFIG_DEFAULT_TTL_STATUS, NO_RESPONSE, "DefaultTTLStatus" },
	{ OP_CONFIG_FRIEND_GET, OP_CONFIG_FRIEND_STATUS, "FriendGet" },
	{ OP_CONFIG_FRIEND_SET, OP_CONFIG_FRIEND_STATUS, "FrienSet" },
	{ OP_CONFIG_FRIEND_STATUS, NO_RESPONSE, "FriendStatus" },
	{ OP_CONFIG_PROXY_GET, OP_CONFIG_PROXY_STATUS, "ProxyGet" },
	{ OP_CONFIG_PROXY_SET, OP_CONFIG_PROXY_STATUS, "ProxySet" },
	{ OP_CONFIG_PROXY_STATUS, NO_RESPONSE, "ProxyStatus" },
	{ OP_CONFIG_KEY_REFRESH_PHASE_GET, OP_CONFIG_KEY_REFRESH_PHASE_STATUS,
							"KeyRefreshPhaseGet" },
	{ OP_CONFIG_KEY_REFRESH_PHASE_SET, OP_CONFIG_KEY_REFRESH_PHASE_STATUS,
							"KeyRefreshPhaseSet" },
	{ OP_CONFIG_KEY_REFRESH_PHASE_STATUS, NO_RESPONSE,
						"KeyRefreshPhaseStatus" },
	{ OP_CONFIG_MODEL_PUB_GET, OP_CONFIG_MODEL_PUB_STATUS, "ModelPubGet" },
	{ OP_CONFIG_MODEL_PUB_SET, OP_CONFIG_MODEL_PUB_STATUS, "ModelPubSet" },
	{ OP_CONFIG_MODEL_PUB_STATUS, NO_RESPONSE, "ModelPubStatus" },
	{ OP_CONFIG_MODEL_PUB_VIRT_SET, OP_CONFIG_MODEL_PUB_STATUS,
							"ModelPubVirtualSet" },
	{ OP_CONFIG_MODEL_SUB_ADD, OP_CONFIG_MODEL_SUB_STATUS, "ModelSubAdd" },
	{ OP_CONFIG_MODEL_SUB_DELETE, OP_CONFIG_MODEL_SUB_STATUS,
							"ModelSubDelete" },
	{ OP_CONFIG_MODEL_SUB_DELETE_ALL, OP_CONFIG_MODEL_SUB_STATUS,
							"ModelSubDeleteAll" },
	{ OP_CONFIG_MODEL_SUB_OVERWRITE, OP_CONFIG_MODEL_SUB_STATUS,
							"ModelSubOverwrite" },
	{ OP_CONFIG_MODEL_SUB_STATUS, NO_RESPONSE, "ModelSubStatus" },
	{ OP_CONFIG_MODEL_SUB_VIRT_ADD, OP_CONFIG_MODEL_SUB_STATUS,
							"ModelSubVirtAdd" },
	{ OP_CONFIG_MODEL_SUB_VIRT_DELETE, OP_CONFIG_MODEL_SUB_STATUS,
							"ModelSubVirtDelete" },
	{ OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE, OP_CONFIG_MODEL_SUB_STATUS,
						"ModelSubVirtOverwrite" },
	{ OP_CONFIG_NETWORK_TRANSMIT_GET, OP_CONFIG_NETWORK_TRANSMIT_STATUS,
							"NetworkTransmitGet" },
	{ OP_CONFIG_NETWORK_TRANSMIT_SET, OP_CONFIG_NETWORK_TRANSMIT_STATUS,
							"NetworkTransmitSet" },
	{ OP_CONFIG_NETWORK_TRANSMIT_STATUS, NO_RESPONSE,
						"NetworkTransmitStatus" },
	{ OP_CONFIG_RELAY_GET, OP_CONFIG_RELAY_STATUS, "RelayGet" },
	{ OP_CONFIG_RELAY_SET, OP_CONFIG_RELAY_STATUS, "RelaySet" },
	{ OP_CONFIG_RELAY_STATUS, NO_RESPONSE, "RelayStatus" },
	{ OP_CONFIG_MODEL_SUB_GET, OP_CONFIG_MODEL_SUB_LIST, "ModelSubGet" },
	{ OP_CONFIG_MODEL_SUB_LIST, NO_RESPONSE, "ModelSubList" },
	{ OP_CONFIG_VEND_MODEL_SUB_GET, OP_CONFIG_VEND_MODEL_SUB_LIST,
							"VendorModelSubGet" },
	{ OP_CONFIG_VEND_MODEL_SUB_LIST, NO_RESPONSE, "VendorModelSubList" },
	{ OP_CONFIG_POLL_TIMEOUT_LIST, OP_CONFIG_POLL_TIMEOUT_STATUS,
							"PollTimeoutList" },
	{ OP_CONFIG_POLL_TIMEOUT_STATUS, NO_RESPONSE, "PollTimeoutStatus" },
	{ OP_CONFIG_HEARTBEAT_PUB_GET, OP_CONFIG_HEARTBEAT_PUB_STATUS,
							"HeartbeatPubGet" },
	{ OP_CONFIG_HEARTBEAT_PUB_SET, OP_CONFIG_HEARTBEAT_PUB_STATUS,
							"HeartbeatPubSet" },
	{ OP_CONFIG_HEARTBEAT_PUB_STATUS, NO_RESPONSE, "HeartbeatPubStatus" },
	{ OP_CONFIG_HEARTBEAT_SUB_GET, OP_CONFIG_HEARTBEAT_SUB_STATUS,
							"HeartbeatSubGet" },
	{ OP_CONFIG_HEARTBEAT_SUB_SET, OP_CONFIG_HEARTBEAT_SUB_STATUS,
							"HeartbeatSubSet" },
	{ OP_CONFIG_HEARTBEAT_SUB_STATUS, NO_RESPONSE, "HeartbeatSubStatus" },
	{ OP_MODEL_APP_BIND, OP_MODEL_APP_STATUS, "ModelAppBind" },
	{ OP_MODEL_APP_STATUS, NO_RESPONSE, "ModelAppStatus" },
	{ OP_MODEL_APP_UNBIND, OP_MODEL_APP_STATUS, "ModelAppUnbind" },
	{ OP_NETKEY_ADD, OP_NETKEY_STATUS, "NetKeyAdd" },
	{ OP_NETKEY_DELETE, OP_NETKEY_STATUS, "NetKeyDelete" },
	{ OP_NETKEY_GET, OP_NETKEY_LIST, "NetKeyGet" },
	{ OP_NETKEY_LIST, NO_RESPONSE, "NetKeyList" },
	{ OP_NETKEY_STATUS, NO_RESPONSE, "NetKeyStatus" },
	{ OP_NETKEY_UPDATE, OP_NETKEY_STATUS, "NetKeyUpdate" },
	{ OP_NODE_IDENTITY_GET, OP_NODE_IDENTITY_STATUS, "NodeIdentityGet" },
	{ OP_NODE_IDENTITY_SET, OP_NODE_IDENTITY_STATUS, "NodeIdentitySet" },
	{ OP_NODE_IDENTITY_STATUS, NO_RESPONSE, "NodeIdentityStatus" },
	{ OP_NODE_RESET, OP_NODE_RESET_STATUS, "NodeReset" },
	{ OP_NODE_RESET_STATUS, NO_RESPONSE, "NodeResetStatus" },
	{ OP_MODEL_APP_GET, OP_MODEL_APP_LIST, "ModelAppGet" },
	{ OP_MODEL_APP_LIST, NO_RESPONSE, "ModelAppList" },
	{ OP_VEND_MODEL_APP_GET, OP_VEND_MODEL_APP_LIST, "VendorModelAppGet" },
	{ OP_VEND_MODEL_APP_LIST, NO_RESPONSE, "VendorModelAppList" }
};

static const struct cfg_cmd *get_cmd(uint32_t opcode)
{
	uint32_t n;

	for (n = 0; n < L_ARRAY_SIZE(cmds); n++) {
		if (opcode == cmds[n].opcode)
			return &cmds[n];
	}

	return NULL;
}

static const char *opcode_str(uint32_t opcode)
{
	const struct cfg_cmd *cmd;

	cmd = get_cmd(opcode);
	if (!cmd)
		return "Unknown";

	return cmd->desc;
}

static void reset_remote_node(uint16_t addr)
{
	uint8_t ele_cnt = remote_del_node(addr);

	bt_shell_printf("Remote removed (primary %4.4x)\n", addr);
	if (ele_cnt && mgr_del_remote)
		mgr_del_remote(addr, ele_cnt);
}

static void free_request(void *a)
{
	struct pending_req *req = a;

	l_timeout_remove(req->timer);
	l_free(req);
}

static struct pending_req *get_req_by_rsp(uint16_t addr, uint32_t rsp)
{
	const struct l_queue_entry *entry;

	entry = l_queue_get_entries(requests);

	for (; entry; entry = entry->next) {
		struct pending_req *req = entry->data;

		if (req->addr == addr && req->cmd->rsp == rsp)
			return req;
	}

	return NULL;
}

static void wait_rsp_timeout(struct l_timeout *timeout, void *user_data)
{
	struct pending_req *req = user_data;

	bt_shell_printf("No response for \"%s\" from %4.4x\n",
						req->cmd->desc, req->addr);

	/* Node reset case: delete the remote even if there is no response */
	if (req->cmd->opcode == OP_NODE_RESET)
		reset_remote_node(req->addr);

	l_queue_remove(requests, req);
	free_request(req);
}

static void add_request(uint32_t opcode)
{
	struct pending_req *req;
	const struct cfg_cmd *cmd;

	cmd = get_cmd(opcode);
	if (!cmd)
		return;

	req = l_new(struct pending_req, 1);
	req->cmd = cmd;
	req->addr = target;
	req->timer = l_timeout_create(rsp_timeout,
				wait_rsp_timeout, req, NULL);
	l_queue_push_tail(requests, req);
}

static uint32_t print_mod_id(uint8_t *data, bool vendor, const char *offset)
{
	uint32_t mod_id;

	if (!vendor) {
		mod_id = get_le16(data);
		bt_shell_printf("%sModel ID\t%4.4x \"%s\"\n",
				offset, mod_id, sig_model_string(mod_id));
		mod_id = VENDOR_ID_MASK | mod_id;
	} else {
		mod_id = get_le16(data + 2);
		bt_shell_printf("%sModel ID\t%4.4x %4.4x\n", offset,
							get_le16(data), mod_id);
		mod_id = get_le16(data) << 16 | mod_id;
	}

	return mod_id;
}

static void print_composition(uint8_t *data, uint16_t len)
{
	uint16_t features;
	int i = 0;

	bt_shell_printf("Received composion:\n");

	/* skip page -- We only support Page Zero */
	data++;
	len--;

	bt_shell_printf("\tCID: %4.4x", get_le16(&data[0]));
	bt_shell_printf("\tPID: %4.4x", get_le16(&data[2]));
	bt_shell_printf("\tVID: %4.4x", get_le16(&data[4]));
	bt_shell_printf("\tCRPL: %4.4x", get_le16(&data[6]));

	features = get_le16(&data[8]);
	data += 10;
	len -= 10;

	bt_shell_printf("\tFeature support:\n");
	bt_shell_printf("\t\trelay: %s\n", (features & FEATURE_RELAY) ?
								"yes" : "no");
	bt_shell_printf("\t\tproxy: %s\n", (features & FEATURE_PROXY) ?
								"yes" : "no");
	bt_shell_printf("\t\tfriend: %s\n", (features & FEATURE_FRIEND) ?
								"yes" : "no");
	bt_shell_printf("\t\tlpn: %s\n", (features & FEATURE_LPN) ?
								"yes" : "no");

	while (len) {
		uint8_t m, v;

		bt_shell_printf("\t Element %d:\n", i);
		bt_shell_printf("\t\tlocation: %4.4x\n", get_le16(data));
		data += 2;
		len -= 2;

		m = *data++;
		v = *data++;
		len -= 2;

		if (m)
			bt_shell_printf("\t\tSIG defined models:\n");

		while (len >= 2 && m--) {
			print_mod_id(data, false, "\t\t  ");
			data += 2;
			len -= 2;
		}

		if (v)
			bt_shell_printf("\t\t Vendor defined models:\n");

		while (len >= 4 && v--) {
			print_mod_id(data, true, "\t\t  ");
			data += 4;
			len -= 4;
		}

		i++;
	}
}

static void print_pub(uint16_t ele_addr, uint32_t mod_id,
						struct model_pub *pub)
{
	bt_shell_printf("\tElement: %4.4x\n", ele_addr);
	bt_shell_printf("\tPub Addr: %4.4x\n", pub->u.addr16);

	if (mod_id < VENDOR_ID_MASK)
		bt_shell_printf("\tModel: %8.8x\n", mod_id);
	else
		bt_shell_printf("\tModel: %4.4x\n",
				(uint16_t) (mod_id & 0xffff));

	bt_shell_printf("\tApp Key Idx: %u (0x%3.3x)\n", pub->app_idx,
								pub->app_idx);
	bt_shell_printf("\tTTL: %2.2x\n", pub->ttl);
}

static void print_sub_list(uint16_t addr, bool is_vendor, uint8_t *data,
								uint16_t len)
{
	uint16_t i;

	bt_shell_printf("\nNode %4.4x Subscription List status %s\n",
						addr, mesh_status_str(data[0]));

	bt_shell_printf("Element Addr\t%4.4x\n", get_le16(data + 1));
	print_mod_id(data + 3, is_vendor, "");

	i = (is_vendor ? 7 : 5);

	bt_shell_printf("Subscriptions:\n");

	for (; i < len; i += 2)
		bt_shell_printf("\t\t%4.4x\n ", get_le16(data + i));
}

static void print_appkey_list(uint16_t len, uint8_t *data)
{
	uint16_t app_idx;

	bt_shell_printf("AppKeys:\n");

	while (len >= 3) {
		app_idx = l_get_le16(data) & 0xfff;
		bt_shell_printf("\t%u (0x%3.3x)\n", app_idx, app_idx);
		app_idx = l_get_le16(data + 1) >> 4;
		bt_shell_printf("\t%u (0x%3.3x)\n", app_idx, app_idx);
		data += 3;
		len -= 3;
	}

	if (len == 2) {
		app_idx = l_get_le16(data) & 0xfff;
		bt_shell_printf("\t %u (0x%3.3x)\n", app_idx, app_idx);
	}
}

static bool msg_recvd(uint16_t src, uint16_t idx, uint8_t *data,
							uint16_t len)
{
	uint32_t opcode;
	const struct cfg_cmd *cmd;
	uint16_t app_idx, net_idx, addr;
	uint16_t ele_addr;
	uint32_t mod_id;
	struct model_pub pub;
	int n;
	struct pending_req *req;

	if (mesh_opcode_get(data, len, &opcode, &n)) {
		len -= n;
		data += n;
	} else
		return false;

	bt_shell_printf("Received %s (len %u)\n", opcode_str(opcode), len);

	req = get_req_by_rsp(src, opcode);
	if (req) {
		cmd = req->cmd;
		l_queue_remove(requests, req);
		free_request(req);
	} else
		cmd = NULL;

	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_STATUS:
		if (len < MIN_COMPOSITION_LEN)
			break;

		print_composition(data, len);

		if (!mesh_db_node_set_composition(src, data, len))
			bt_shell_printf("Failed to save node composition!\n");
		break;

	case OP_APPKEY_STATUS:
		if (len != 4)
			break;

		bt_shell_printf("Node %4.4x AppKey status %s\n", src,
						mesh_status_str(data[0]));
		net_idx = get_le16(data + 1) & 0xfff;
		app_idx = get_le16(data + 2) >> 4;

		bt_shell_printf("NetKey\t%u (0x%3.3x)\n", net_idx, net_idx);
		bt_shell_printf("AppKey\t%u (0x%3.3x)\n", app_idx, app_idx);

		if (data[0] != MESH_STATUS_SUCCESS)
			break;

		if (!cmd)
			break;

		if (cmd->opcode == OP_APPKEY_ADD) {
			if (remote_add_app_key(src, app_idx))
				mesh_db_node_app_key_add(src, app_idx);
		} else if (cmd->opcode == OP_APPKEY_DELETE) {
			if (remote_del_app_key(src, app_idx))
				mesh_db_node_app_key_del(src, app_idx);
		}

		break;

	case OP_APPKEY_LIST:
		if (len < 3)
			break;

		bt_shell_printf("AppKey List (node %4.4x) Status %s\n",
						src, mesh_status_str(data[0]));

		net_idx = l_get_le16(&data[1]);
		bt_shell_printf("NetKey %u (0x%3.3x)\n", net_idx, net_idx);
		len -= 3;

		if (data[0] != MESH_STATUS_SUCCESS)
			break;

		data += 3;
		print_appkey_list(len, data);

		break;

	case OP_NETKEY_STATUS:
		if (len != 3)
			break;

		bt_shell_printf("Node %4.4x NetKey status %s\n", src,
						mesh_status_str(data[0]));
		net_idx = get_le16(data + 1) & 0xfff;

		bt_shell_printf("\tNetKey %u (0x%3.3x)\n", net_idx, net_idx);

		if (data[0] != MESH_STATUS_SUCCESS)
			break;

		if (!cmd)
			break;

		if (cmd->opcode == OP_NETKEY_ADD) {
			if (remote_add_net_key(src, net_idx))
				mesh_db_node_net_key_add(src, net_idx);
		} else if (cmd->opcode == OP_NETKEY_DELETE) {
			if (remote_del_net_key(src, net_idx))
				mesh_db_node_net_key_del(src, net_idx);
		}

		break;

	case OP_NETKEY_LIST:
		if (len < 2)
			break;

		bt_shell_printf("NetKey List (node %4.4x):\n", src);

		while (len >= 3) {
			net_idx = l_get_le16(data) & 0xfff;
			bt_shell_printf("\t%u (0x%3.3x)\n", net_idx, net_idx);
			net_idx = l_get_le16(data + 1) >> 4;
			bt_shell_printf("\t%u (0x%3.3x)\n", net_idx, net_idx);
			data += 3;
			len -= 3;
		}

		if (len == 2) {
			net_idx = l_get_le16(data) & 0xfff;
			bt_shell_printf("\t %u (0x%3.3x)\n", net_idx, net_idx);
		}

		break;

	case OP_CONFIG_KEY_REFRESH_PHASE_STATUS:
		if (len != 4)
			break;

		bt_shell_printf("Node %4.4x Key Refresh Phase status %s\n", src,
						mesh_status_str(data[0]));
		net_idx = get_le16(data + 1) & 0xfff;

		bt_shell_printf("\tNetKey %u (0x%3.3x)\n", net_idx, net_idx);
		bt_shell_printf("\tKR Phase %2.2x\n", data[3]);
		break;

	case OP_MODEL_APP_STATUS:
		if (len != 7 && len != 9)
			break;

		bt_shell_printf("Node %4.4x: Model App status %s\n", src,
						mesh_status_str(data[0]));
		addr = get_le16(data + 1);
		app_idx = get_le16(data + 3);

		bt_shell_printf("Element Addr\t%4.4x\n", addr);

		print_mod_id(data + 5, len == 9, "");

		bt_shell_printf("AppIdx\t\t%u (0x%3.3x)\n ", app_idx, app_idx);

		break;

	case OP_NODE_IDENTITY_STATUS:
		if (len != 4)
			return true;

		bt_shell_printf("NetIdx %4.4x, NodeIdState 0x%02x, status %s\n",
				get_le16(data + 1), data[3],
				mesh_status_str(data[0]));
		break;

	case OP_CONFIG_BEACON_STATUS:
		if (len != 1)
			return true;

		bt_shell_printf("Node %4.4x: Config Beacon Status 0x%02x\n",
				src, data[0]);
		break;

	case OP_CONFIG_RELAY_STATUS:
		if (len != 2)
			return true;

		bt_shell_printf("Node %4.4x: Relay 0x%02x, cnt %d, steps %d\n",
				src, data[0], data[1] & 0x7, data[1] >> 3);
		break;

	case OP_CONFIG_PROXY_STATUS:
		if (len != 1)
			return true;

		bt_shell_printf("Node %4.4x Proxy state 0x%02x\n",
				src, data[0]);
		break;

	case OP_CONFIG_DEFAULT_TTL_STATUS:
		if (len != 1)
			return true;

		bt_shell_printf("Node %4.4x Default TTL %d\n", src, data[0]);
		mesh_db_node_ttl_set(src, data[0]);

		break;

	case OP_CONFIG_MODEL_PUB_STATUS:
		if (len != 12 && len != 14)
			return true;

		bt_shell_printf("\nNode %4.4x Publication status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		ele_addr = get_le16(data + 1);

		mod_id = print_mod_id(data + 10, len == 14, "");

		pub.u.addr16 = get_le16(data + 3);
		pub.app_idx = get_le16(data + 5);
		pub.ttl = data[7];
		pub.period = data[8];
		n = (data[8] & 0x3f);

		print_pub(ele_addr, mod_id, &pub);

		switch (data[8] >> 6) {
		case 0:
			bt_shell_printf("Period\t\t%d ms\n", n * 100);
			break;
		case 2:
			n *= 10;
			/* fall through */
		case 1:
			bt_shell_printf("Period\t\t%d sec\n", n);
			break;
		case 3:
			bt_shell_printf("Period\t\t%d min\n", n * 10);
			break;
		}

		bt_shell_printf("Rexmit count\t%d\n", data[9] & 0x7);
		bt_shell_printf("Rexmit steps\t%d\n", data[9] >> 3);

		break;

	/* Per Mesh Profile 4.3.2.19 */
	case OP_CONFIG_MODEL_SUB_STATUS:
		if (len != 7 && len != 9)
			return true;

		bt_shell_printf("\nNode %4.4x Subscription status %s\n",
				src, mesh_status_str(data[0]));

		ele_addr = get_le16(data + 1);
		addr = get_le16(data + 3);
		bt_shell_printf("Element Addr\t%4.4x\n", ele_addr);

		print_mod_id(data + 5, len == 9, "");

		bt_shell_printf("Subscr Addr\t%4.4x\n", addr);

		break;

	/* Per Mesh Profile 4.3.2.27 */
	case OP_CONFIG_MODEL_SUB_LIST:
		if (len < 5)
			return true;

		print_sub_list(src, false, data, len);
		break;

	case OP_CONFIG_VEND_MODEL_SUB_LIST:
		if (len < 7)
			return true;

		print_sub_list(src, true, data, len);
		break;

	/* Per Mesh Profile 4.3.2.50 */
	case OP_MODEL_APP_LIST:
		if (len < 5)
			return true;

		bt_shell_printf("\nNode %4.4x Model AppIdx status %s\n",
						src, mesh_status_str(data[0]));

		bt_shell_printf("Element Addr\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Model ID\t%4.4x\n", get_le16(data + 3));

		data += 5;
		len -= 5;
		print_appkey_list(len, data);

		break;

	case OP_VEND_MODEL_APP_LIST:
		if (len < 7)
			return true;

		bt_shell_printf("\nNode %4.4x Vendor Model AppIdx status %s\n",
						src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		bt_shell_printf("Element Addr\t%4.4x\n", get_le16(data + 1));
		print_mod_id(data + 3, true, "");

		data += 7;
		len -= 7;
		print_appkey_list(len, data);

		break;

	/* Per Mesh Profile 4.3.2.63 */
	case OP_CONFIG_HEARTBEAT_PUB_STATUS:
		if (len != 10)
			return true;

		bt_shell_printf("\nNode %4.4x Heartbeat publish status %s\n",
				src, mesh_status_str(data[0]));

		bt_shell_printf("Destination\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Count\t\t%2.2x\n", data[3]);
		bt_shell_printf("Period\t\t%2.2x\n", data[4]);
		bt_shell_printf("TTL\t\t%2.2x\n", data[5]);
		bt_shell_printf("Features\t%4.4x\n", get_le16(data + 6));
		net_idx = get_le16(data + 8);
		bt_shell_printf("Net_Idx\t%u (0x%3.3x)\n", net_idx, net_idx);
		break;

	/* Per Mesh Profile 4.3.2.66 */
	case OP_CONFIG_HEARTBEAT_SUB_STATUS:
		if (len != 9)
			return true;

		bt_shell_printf("\nNode %4.4x Heartbeat subscribe status %s\n",
				src, mesh_status_str(data[0]));

		bt_shell_printf("Source\t\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Destination\t%4.4x\n", get_le16(data + 3));
		bt_shell_printf("Period\t\t%2.2x\n", data[5]);
		bt_shell_printf("Count\t\t%2.2x\n", data[6]);
		bt_shell_printf("Min Hops\t%2.2x\n", data[7]);
		bt_shell_printf("Max Hops\t%2.2x\n", data[8]);
		break;

	/* Per Mesh Profile 4.3.2.71 */
	case OP_CONFIG_NETWORK_TRANSMIT_STATUS:
		if (len != 1)
			return true;

		bt_shell_printf("Node %4.4x: Net transmit cnt %d, steps %d\n",
				src, data[0] & 7, data[0] >> 3);
		break;

	/* Per Mesh Profile 4.3.2.54 */
	case OP_NODE_RESET_STATUS:

		bt_shell_printf("Node %4.4x is reset\n", src);
		reset_remote_node(src);

		break;

	/* Per Mesh Profile 4.3.2.57 */
	case OP_CONFIG_FRIEND_STATUS:
		if (len != 1)
			return true;

		bt_shell_printf("Node %4.4x Friend state 0x%02x\n",
				src, data[0]);
		break;
	}

	return true;
}

static uint16_t put_model_id(uint8_t *buf, uint32_t *args, bool vendor)
{
	uint16_t n = 2;

	if (vendor) {
		put_le16(args[1], buf);
		buf += 2;
		n = 4;
	}

	put_le16(args[0], buf);

	return n;
}

static uint32_t read_input_parameters(int argc, char *argv[])
{
	uint32_t i;

	if (!argc)
		return 0;

	--argc;
	++argv;

	if (!argc || argv[0][0] == '\0')
		return 0;

	for (i = 0; i < L_ARRAY_SIZE(parms) && i < (uint32_t) argc; i++) {
		if (sscanf(argv[i], "%x", &parms[i]) != 1)
			break;
	}

	return i;
}

static bool match_group_addr(const void *a, const void *b)
{
	const struct mesh_group *grp = a;
	uint16_t addr = L_PTR_TO_UINT(b);

	return grp->addr == addr;
}

static int compare_group_addr(const void *a, const void *b, void *user_data)
{
	const struct mesh_group *grp0 = a;
	const struct mesh_group *grp1 = b;

	if (grp0->addr < grp1->addr)
		return -1;

	if (grp0->addr > grp1->addr)
		return 1;

	return 0;
}

static void print_virtual_not_found(uint16_t addr)
{
	bt_shell_printf("Virtual group with hash %4.4x not found\n", addr);
	bt_shell_printf("To see available, use \"group-list\"\n");
	bt_shell_printf("To create new, use \"virt-add\"\n");
}

static struct mesh_group *add_group(uint16_t addr)
{
	struct mesh_group *grp;

	if (!IS_GROUP(addr) || addr >= FIXED_GROUP_LOW)
		return NULL;

	grp = l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(addr));
	if (grp)
		return grp;

	grp = l_new(struct mesh_group, 1);
	grp->addr = addr;
	l_queue_insert(groups, grp, compare_group_addr, NULL);

	mesh_db_add_group(grp);

	return grp;
}

static void cmd_timeout_set(int argc, char *argv[])
{
	if (read_input_parameters(argc, argv) != 1)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	rsp_timeout = parms[0];

	bt_shell_printf("Timeout to wait for remote node's response: %d secs\n",
								rsp_timeout);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_dst_set(int argc, char *argv[])
{
	uint32_t dst;
	char *end;

	dst = strtol(argv[1], &end, 16);

	if (end != (argv[1] + 4)) {
		bt_shell_printf("Bad unicast address %s: "
				"expected format 4 digit hex\n", argv[1]);
		target = UNASSIGNED_ADDRESS;

		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Configuring node %4.4x\n", dst);
	target = dst;
	set_menu_prompt("config", argv[1]);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static bool config_send(uint8_t *buf, uint16_t len, uint32_t opcode)
{
	const struct cfg_cmd *cmd;
	bool res;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return false;
	}

	cmd = get_cmd(opcode);
	if (!cmd)
		return false;

	if (get_req_by_rsp(target, cmd->rsp)) {
		bt_shell_printf("Another command is pending\n");
		return false;
	}

	res = send_msg(send_data, target, APP_IDX_DEV_REMOTE, buf, len);
	if (!res)
		bt_shell_printf("Failed to send \"%s\"\n", opcode_str(opcode));

	if (cmd->rsp != NO_RESPONSE)
		add_request(opcode);

	return res;
}

static void cmd_default(uint32_t opcode)
{
	uint16_t n;
	uint8_t msg[32];

	n = mesh_opcode_set(opcode, msg);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_composition_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];

	n = mesh_opcode_set(OP_DEV_COMP_GET, msg);

	/* By default, use page 0 */
	msg[n++] = (read_input_parameters(argc, argv) == 1) ? parms[0] : 0;

	if (!config_send(msg, n, OP_DEV_COMP_GET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_netkey_del(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_NETKEY_DELETE, msg);

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;

	if (!config_send(msg, n, OP_NETKEY_DELETE))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_kr_phase_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_KEY_REFRESH_PHASE_GET, msg);

	if (read_input_parameters(argc, argv) != 1)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	put_le16(parms[0], msg + n);
	n += 2;

	if (!config_send(msg, n, OP_CONFIG_KEY_REFRESH_PHASE_GET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_kr_phase_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint8_t phase;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_KEY_REFRESH_PHASE_SET, msg);

	if (read_input_parameters(argc, argv) != 2)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (parms[1] != KEY_REFRESH_PHASE_TWO &&
				parms[1] != KEY_REFRESH_PHASE_THREE) {
		bt_shell_printf("Invalid KR transition value %u\n", parms[1]);
		bt_shell_printf("Allowed values: 2 or 3\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!keys_get_net_key_phase((uint16_t) parms[0], &phase)) {
		bt_shell_printf("Subnet KR state not found\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (phase != (parms[1] % KEY_REFRESH_PHASE_THREE)) {
		bt_shell_printf("Subnet's phase must be updated first!\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;

	msg[n++] = parms[1];

	if (!config_send(msg, n, OP_CONFIG_KEY_REFRESH_PHASE_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_appkey_del(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint16_t app_idx, net_idx;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_APPKEY_DELETE, msg);

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	app_idx = (uint16_t) parms[0];
	net_idx = keys_get_bound_key(app_idx);

	/* Pack bound NetKey and AppKey into 3 octets */
	msg[n] = net_idx;
	msg[n + 1] = ((net_idx >> 8) & 0xf) | ((app_idx << 4) & 0xf0);
	msg[n + 2] = app_idx >> 4;

	n += 3;

	if (!config_send(msg, n, OP_APPKEY_DELETE))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_appkey_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_APPKEY_GET, msg);

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;

	if (!config_send(msg, n, OP_APPKEY_GET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_key_add(uint32_t opcode, int argc, char *argv[])
{
	uint16_t key_idx;
	bool is_appkey, update;
	const struct cfg_cmd *cmd;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!send_key_msg) {
		bt_shell_printf("Send key callback not set\n");
		return;
	}

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd = get_cmd(opcode);
	if (!cmd)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (get_req_by_rsp(target, cmd->rsp)) {
		bt_shell_printf("Another key command is pending\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	key_idx = (uint16_t) parms[0];

	update = (opcode == OP_NETKEY_UPDATE || opcode == OP_APPKEY_UPDATE);
	is_appkey = (opcode == OP_APPKEY_ADD || opcode == OP_APPKEY_UPDATE);

	if (!send_key_msg(key_data, target, key_idx, is_appkey, update))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	add_request(opcode);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_netkey_add(int argc, char *argv[])
{
	cmd_key_add(OP_NETKEY_ADD, argc, argv);
}

static void cmd_netkey_update(int argc, char *argv[])
{
	cmd_key_add(OP_NETKEY_UPDATE, argc, argv);
}

static void cmd_appkey_add(int argc, char *argv[])
{
	cmd_key_add(OP_APPKEY_ADD, argc, argv);
}

static void cmd_appkey_update(int argc, char *argv[])
{
	cmd_key_add(OP_APPKEY_UPDATE, argc, argv);
}

static void cmd_bind(uint32_t opcode, int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3 && parm_cnt != 4) {
		bt_shell_printf("Bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(opcode, msg);

	put_le16(parms[0], msg + n);
	n += 2;
	put_le16(parms[1], msg + n);
	n += 2;

	n += put_model_id(msg + n, &parms[2], parm_cnt == 4);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_add_binding(int argc, char *argv[])
{
	cmd_bind(OP_MODEL_APP_BIND, argc, argv);
}

static void cmd_del_binding(int argc, char *argv[])
{
	cmd_bind(OP_MODEL_APP_UNBIND, argc, argv);
}

static void cmd_beacon_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_BEACON_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];

	if (!config_send(msg, n, OP_CONFIG_BEACON_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_beacon_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_BEACON_GET);
}

static void cmd_ident_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 3 + 4];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_NODE_IDENTITY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;
	msg[n++] = parms[1];

	if (!config_send(msg, n, OP_NODE_IDENTITY_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ident_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 2 + 4];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_NODE_IDENTITY_GET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;

	if (!config_send(msg, n, OP_NODE_IDENTITY_GET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_proxy_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_PROXY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];

	if (!config_send(msg, n, OP_CONFIG_PROXY_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_proxy_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_PROXY_GET);
}

static void cmd_relay_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 2 + 4];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_RELAY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];
	msg[n++] = parms[1] | (parms[2] << 3);

	if (!config_send(msg, n, OP_CONFIG_RELAY_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_relay_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_RELAY_GET);
}

static void cmd_ttl_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	parm_cnt = read_input_parameters(argc, argv);
	if (!parm_cnt || parms[0] > TTL_MASK) {
		bt_shell_printf("Bad TTL value\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_DEFAULT_TTL_SET, msg);
	msg[n++] = parms[0];

	if (!config_send(msg, n, OP_CONFIG_DEFAULT_TTL_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_pub_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[48];
	uint32_t parm_cnt;
	struct mesh_group *grp;
	uint32_t opcode;
	uint16_t pub_addr;

	parm_cnt = read_input_parameters(argc, argv);

	if (parm_cnt != 6 && parm_cnt != 7) {
		bt_shell_printf("Bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (parms[1] > ALL_NODES_ADDRESS) {
		bt_shell_printf("Bad publication address %x\n", parms[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	pub_addr = parms[1];

	grp = l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(pub_addr));
	if (!grp)
		grp = add_group(pub_addr);

	if (!grp && IS_VIRTUAL(pub_addr)) {
		print_virtual_not_found(pub_addr);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	opcode = (!IS_VIRTUAL(pub_addr)) ? OP_CONFIG_MODEL_PUB_SET :
						OP_CONFIG_MODEL_PUB_VIRT_SET;

	n = mesh_opcode_set(opcode, msg);

	put_le16(parms[0], msg + n);
	n += 2;

	/* Publish address */
	if (!IS_VIRTUAL(pub_addr)) {
		put_le16(pub_addr, msg + n);
		n += 2;
	} else {
		memcpy(msg + n, grp->label, 16);
		n += 16;
	}

	/* AppKey index + credential (set to 0) */
	put_le16(parms[2], msg + n);
	n += 2;
	/* TTL */
	msg[n++] = DEFAULT_TTL;
	/* Publish period  step count and step resolution */
	msg[n++] = parms[3];
	/* Publish retransmit count & interval steps */
	msg[n++] = parms[4];

	/* Model Id */
	n += put_model_id(msg + n, &parms[5], parm_cnt == 7);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_pub_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_MODEL_PUB_GET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2 && parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;

	/* Model Id */
	n += put_model_id(msg + n, &parms[1], parm_cnt == 3);

	if (!config_send(msg, n, OP_CONFIG_MODEL_PUB_GET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void subscription_cmd(int argc, char *argv[], uint32_t opcode)
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;
	struct mesh_group *grp;
	uint16_t sub_addr;

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3 && parm_cnt != 4) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if ((!IS_GROUP(parms[1]) || IS_ALL_NODES(parms[1])) &&
							!IS_VIRTUAL(parms[1])) {
		bt_shell_printf("Bad subscription address %x\n", parms[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	sub_addr = parms[1];

	grp = l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(sub_addr));

	if (!grp && IS_VIRTUAL(sub_addr)) {
		print_virtual_not_found(sub_addr);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!grp && opcode != OP_CONFIG_MODEL_SUB_DELETE)
		grp = add_group(sub_addr);

	if (IS_VIRTUAL(sub_addr)) {
		if (opcode == OP_CONFIG_MODEL_SUB_ADD)
			opcode = OP_CONFIG_MODEL_SUB_VIRT_ADD;
		else if (opcode == OP_CONFIG_MODEL_SUB_DELETE)
			opcode = OP_CONFIG_MODEL_SUB_VIRT_DELETE;
		else if (opcode == OP_CONFIG_MODEL_SUB_OVERWRITE)
			opcode = OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE;
	}

	n = mesh_opcode_set(opcode, msg);

	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;

	/* Subscription Address */
	if (!IS_VIRTUAL(sub_addr)) {
		put_le16(sub_addr, msg + n);
		n += 2;
	} else {
		memcpy(msg + n, grp->label, 16);
		n += 16;
	}

	/* Model ID */
	n += put_model_id(msg + n, &parms[2], parm_cnt == 4);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_sub_add(int argc, char *argv[])
{
	subscription_cmd(argc, argv, OP_CONFIG_MODEL_SUB_ADD);
}

static void cmd_sub_del(int argc, char *argv[])
{
	subscription_cmd(argc, argv, OP_CONFIG_MODEL_SUB_DELETE);
}

static void cmd_sub_ovwrt(int argc, char *argv[])
{
	subscription_cmd(argc, argv, OP_CONFIG_MODEL_SUB_OVERWRITE);
}

static void cmd_sub_del_all(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_MODEL_SUB_DELETE_ALL, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2 && parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;

	/* Model ID */
	n += put_model_id(msg + n, &parms[1], parm_cnt == 3);

	if (!config_send(msg, n, OP_CONFIG_MODEL_SUB_DELETE_ALL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_sub_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;
	bool vendor;
	uint32_t opcode;

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2 && parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	vendor = (parm_cnt == 3);
	opcode = !vendor ? OP_CONFIG_MODEL_SUB_GET :
						OP_CONFIG_VEND_MODEL_SUB_GET;
	n = mesh_opcode_set(opcode, msg);

	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Model ID */
	n += put_model_id(msg + n, &parms[1], vendor);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_mod_appidx_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;
	bool vendor;
	uint32_t opcode;

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2 && parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	vendor = (parm_cnt == 3);
	opcode = !vendor ? OP_MODEL_APP_GET : OP_VEND_MODEL_APP_GET;
	n = mesh_opcode_set(opcode, msg);

	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Model ID */
	n += put_model_id(msg + n, &parms[1], vendor);

	if (!config_send(msg, n, opcode))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_hb_pub_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_HEARTBEAT_PUB_SET, msg);

	if (!l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(parms[1])))
		add_group(parms[1]);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 6) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Per Mesh Profile 4.3.2.62 */
	/* Publish address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Count Log */
	msg[n++] = parms[1];
	/* Period Log */
	msg[n++] = parms[2];
	/* Heartbeat TTL */
	msg[n++] = parms[3];
	/* Features */
	put_le16(parms[4], msg + n);
	n += 2;
	/* NetKey Index */
	put_le16(parms[5], msg + n);
	n += 2;

	if (!config_send(msg, n, OP_CONFIG_HEARTBEAT_PUB_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_hb_pub_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_HEARTBEAT_PUB_GET);
}

static void cmd_hb_sub_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_HEARTBEAT_SUB_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(parms[1])))
		add_group(parms[1]);

	/* Per Mesh Profile 4.3.2.65 */
	/* Source address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Destination address */
	put_le16(parms[1], msg + n);
	n += 2;
	/* Period log */
	msg[n++] = parms[2];

	if (!config_send(msg, n, OP_CONFIG_HEARTBEAT_SUB_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_hb_sub_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_HEARTBEAT_SUB_GET);
}

static void cmd_ttl_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_DEFAULT_TTL_GET);
}

static void cmd_network_transmit_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_NETWORK_TRANSMIT_GET);
}

static void cmd_network_transmit_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_NETWORK_TRANSMIT_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0] | (parms[1] << 3);

	if (!config_send(msg, n, OP_CONFIG_NETWORK_TRANSMIT_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_friend_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	uint32_t parm_cnt;

	n = mesh_opcode_set(OP_CONFIG_FRIEND_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];

	if (!config_send(msg, n, OP_CONFIG_FRIEND_SET))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_friend_get(int argc, char *argv[])
{
	cmd_default(OP_CONFIG_FRIEND_GET);
}

static void cmd_node_reset(int argc, char *argv[])
{
	uint16_t n, i;
	uint8_t msg[8];
	struct pending_req *req;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Cannot remet self */
	if (target == 0x0001) {
		bt_shell_printf("Resetting self not allowed\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_NODE_RESET, msg);

	req = l_new(struct pending_req, 1);
	req->addr = target;
	req->cmd = get_cmd(OP_NODE_RESET);

	/*
	 * As a courtesy to the remote node, send the reset command
	 * several times. Treat this as a single request with a longer
	 * response timeout.
	 */
	req->timer = l_timeout_create(rsp_timeout * 2,
				wait_rsp_timeout, req, NULL);

	l_queue_push_tail(requests, req);

	for (i = 0; i < 5; i++)
		send_msg(send_data, target, APP_IDX_DEV_REMOTE, msg, n);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_netkey_get(int argc, char *argv[])
{
	cmd_default(OP_NETKEY_GET);
}

static void print_group(void *a, void *b)
{
	struct mesh_group *grp = a;
	char buf[33];

	if (!IS_VIRTUAL(grp->addr)) {
		bt_shell_printf("\tGroup addr: %4.4x\n", grp->addr);
		return;
	}

	hex2str(grp->label, 16, buf, sizeof(buf));
	bt_shell_printf("\tVirtual addr: %4.4x, label: %s\n", grp->addr, buf);
}

static void cmd_add_virt(int argc, char *argv[])
{
	struct mesh_group *grp, *tmp;
	uint8_t max_tries = 3;

	grp = l_new(struct mesh_group, 1);

retry:
	l_getrandom(grp->label, 16);
	mesh_crypto_virtual_addr(grp->label, &grp->addr);

	/* For simplicity sake, avoid labels that map to the same hash */
	tmp = l_queue_find(groups, match_group_addr, L_UINT_TO_PTR(grp->addr));
	if (!tmp) {
		l_queue_insert(groups, grp, compare_group_addr, NULL);
		print_group(grp, NULL);
		mesh_db_add_group(grp);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	max_tries--;
	if (max_tries)
		goto retry;

	l_free(grp);
	bt_shell_printf("Failed to generate unique label. Try again.");
	bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_list_groups(int argc, char *argv[])
{
	l_queue_foreach(groups, print_group, NULL);
	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static bool tx_setup(model_send_msg_func_t send_func, void *user_data)
{
	if (!send_func)
		return false;

	send_msg = send_func;
	send_data = user_data;

	return true;
}

static const struct bt_shell_menu cfg_menu = {
	.name = "config",
	.desc = "Configuration Model Submenu",
	.entries = {
	{"target", "<unicast>", cmd_dst_set,
				"Set target node to configure"},
	{"timeout", "<seconds>", cmd_timeout_set,
				"Set response timeout (seconds)"},
	{"composition-get", "[page_num]", cmd_composition_get,
				"Get composition data"},
	{"netkey-add", "<net_idx>", cmd_netkey_add,
				"Add NetKey"},
	{"netkey-update", "<net_idx>", cmd_netkey_update,
				"Update NetKey"},
	{"netkey-del", "<net_idx>", cmd_netkey_del,
				"Delete NetKey"},
	{"netkey-get", NULL, cmd_netkey_get,
				"List NetKeys known to the node"},
	{"kr-phase-get", "<net_idx>", cmd_kr_phase_get,
				"Get Key Refresh phase of a NetKey"},
	{"kr-phase-set", "<net_idx> <phase>", cmd_kr_phase_set,
				"Set Key Refresh phase transition of a NetKey"},
	{"appkey-add", "<app_idx>", cmd_appkey_add,
				"Add AppKey"},
	{"appkey-update", "<app_idx>", cmd_appkey_update,
				"Add AppKey"},
	{"appkey-del", "<app_idx>", cmd_appkey_del,
				"Delete AppKey"},
	{"appkey-get", "<net_idx>", cmd_appkey_get,
				"List AppKeys bound to the NetKey"},
	{"bind", "<ele_addr> <app_idx> <model_id> [vendor_id]", cmd_add_binding,
				"Bind AppKey to a model"},
	{"unbind", "<ele_addr> <app_idx> <model_id> [vendor_id]",
				cmd_del_binding,
				"Remove AppKey from a model"},
	{"mod-appidx-get", "<ele_addr> <model_id> [vendor_id]",
				cmd_mod_appidx_get,
				"Get model app_idx"},
	{"ttl-set", "<ttl>", cmd_ttl_set,
				"Set default TTL"},
	{"ttl-get", NULL, cmd_ttl_get,
				"Get default TTL"},
	{"pub-set", "<ele_addr> <pub_addr> <app_idx> <per (step|res)> "
			"<re-xmt (cnt|per)> <model_id> [vendor_id]",
				cmd_pub_set,
				"Set publication"},
	{"pub-get", "<ele_addr> <model_id> [vendor_id]", cmd_pub_get,
				"Get publication"},
	{"proxy-set", "<proxy>", cmd_proxy_set,
				"Set proxy state"},
	{"proxy-get", NULL, cmd_proxy_get,
				"Get proxy state"},
	{"ident-set", "<net_idx> <state>", cmd_ident_set,
				"Set node identity state"},
	{"ident-get", "<net_idx>", cmd_ident_get,
				"Get node identity state"},
	{"beacon-set", "<state>", cmd_beacon_set,
				"Set node identity state"},
	{"beacon-get", NULL, cmd_beacon_get,
				"Get node beacon state"},
	{"relay-set", "<relay> <rexmt count> <rexmt steps>", cmd_relay_set,
				"Set relay"},
	{"relay-get", NULL, cmd_relay_get,
				"Get relay"},
	{"friend-set", "<state>", cmd_friend_set,
				"Set friend state"},
	{"friend-get", NULL, cmd_friend_get,
				"Get friend state"},
	{"network-transmit-get", NULL, cmd_network_transmit_get,
				"Get network transmit state"},
	{"network-transmit-set", "<count> <steps>", cmd_network_transmit_set,
				"Set network transmit state"},
	{"hb-pub-set", "<pub_addr> <count> <period> <ttl> <features> <net_idx>",
				cmd_hb_pub_set,
				"Set heartbeat publish"},
	{"hb-pub-get", NULL, cmd_hb_pub_get,
				"Get heartbeat publish"},
	{"hb-sub-set", "<src_addr> <dst_addr> <period>", cmd_hb_sub_set,
				"Set heartbeat subscribe"},
	{"hb-sub-get", NULL, cmd_hb_sub_get,
				"Get heartbeat subscribe"},
	{"virt-add", NULL, cmd_add_virt, "Generate and add a virtual label"},
	{"group-list", NULL, cmd_list_groups,
			"Display existing group addresses and virtual labels"},
	{"sub-add", "<ele_addr> <sub_addr> <model_id> [vendor]",
				cmd_sub_add, "Add subscription"},
	{"sub-del", "<ele_addr> <sub_addr> <model_id> [vendor]",
				cmd_sub_del, "Delete subscription"},
	{"sub-wrt", "<ele_addr> <sub_addr> <model_id> [vendor]",
				cmd_sub_ovwrt, "Overwrite subscription"},
	{"sub-del-all", "<ele_addr> <model_id> [vendor]", cmd_sub_del_all,
				"Delete subscription"},
	{"sub-get", "<ele_addr> <model_id> [vendor]", cmd_sub_get,
				"Get subscription"},
	{"node-reset", NULL, cmd_node_reset,
				"Reset a node and remove it from network"},
	{} },
};

static struct model_info cli_info = {
	.ops = {
		.set_send_func = tx_setup,
		.set_pub_func = NULL,
		.recv = msg_recvd,
		.bind = NULL,
		.pub = NULL
	},
	.mod_id = CONFIG_CLIENT_MODEL_ID,
	.vendor_id = VENDOR_ID_INVALID
};

struct model_info *cfgcli_init(key_send_func_t key_send,
				delete_remote_func_t del_node, void *user_data)
{
	if (!key_send)
		return NULL;

	send_key_msg = key_send;
	key_data = user_data;
	mgr_del_remote = del_node;
	requests = l_queue_new();
	groups = mesh_db_load_groups();
	bt_shell_add_submenu(&cfg_menu);

	return &cli_info;
}

void cfgcli_cleanup(void)
{
	l_queue_destroy(requests, free_request);
	l_queue_destroy(groups, l_free);
}
