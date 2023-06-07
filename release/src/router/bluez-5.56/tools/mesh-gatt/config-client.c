// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wordexp.h>

#include <glib.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"

#include "tools/mesh/config-model.h"

#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/keys.h"
#include "tools/mesh-gatt/net.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/prov-db.h"
#include "tools/mesh-gatt/util.h"

#define MIN_COMPOSITION_LEN 16

static uint32_t print_mod_id(uint8_t *data, bool vid)
{
	uint32_t mod_id;

	if (!vid) {
		mod_id = get_le16(data);
		bt_shell_printf("Model Id\t%4.4x\n", mod_id);
		mod_id = 0xffff0000 | mod_id;
	} else {
		mod_id = get_le16(data + 2);
		bt_shell_printf("Model Id\t%4.4x %4.4x\n",
				get_le16(data), mod_id);
		mod_id = get_le16(data) << 16 | mod_id;
	}
	return mod_id;
}

static bool client_msg_recvd(uint16_t src, uint8_t *data,
				uint16_t len, void *user_data)
{
	uint32_t opcode;
	struct mesh_node *node;
	uint16_t app_idx, net_idx, addr;
	uint32_t mod_id;
	uint16_t primary;
	uint16_t ele_addr;
	uint8_t ele_idx;
	struct mesh_publication pub;
	int n;
	uint16_t i;

	if (mesh_opcode_get(data, len, &opcode, &n)) {
		len -= n;
		data += n;
	} else
		return false;

	if (IS_UNICAST(src)) {
		node = node_find_by_addr(src);
	} else
		node = NULL;

	if (!node)
		return false;

	primary = node_get_primary(node);
	if (primary != src)
		return false;

	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_STATUS:
		if (len < MIN_COMPOSITION_LEN || !node)
			break;
		if (node_parse_composition(node, data, len)) {
			if (!prov_db_add_node_composition(node, data, len))
				break;
		}

		if (node_get_composition(node))
			prov_db_print_node_composition(node);
		break;

	case OP_APPKEY_STATUS:
		if (len != 4)
			break;

		bt_shell_printf("Node %4.4x AppKey status %s\n", src,
						mesh_status_str(data[0]));
		net_idx = get_le16(data + 1) & 0xfff;
		app_idx = get_le16(data + 2) >> 4;

		bt_shell_printf("NetKey\t%3.3x\n", net_idx);
		bt_shell_printf("AppKey\t%3.3x\n", app_idx);

		if (data[0] != MESH_STATUS_SUCCESS &&
				data[0] != MESH_STATUS_IDX_ALREADY_STORED &&
				node_app_key_delete(node, net_idx, app_idx))
			prov_db_node_keys(node, node_get_app_keys(node),
								"appKeys");
		break;

	case OP_NETKEY_STATUS:
		if (len != 3)
			break;

		bt_shell_printf("Node %4.4x NetKey status %s\n", src,
						mesh_status_str(data[0]));
		net_idx = get_le16(data + 1) & 0xfff;

		bt_shell_printf("\tNetKey %3.3x\n", net_idx);

		if (data[0] != MESH_STATUS_SUCCESS &&
				data[0] != MESH_STATUS_IDX_ALREADY_STORED &&
					node_net_key_delete(node, net_idx))
			prov_db_node_keys(node, node_get_net_keys(node),
								"netKeys");
		break;

	case OP_MODEL_APP_STATUS:
		if (len != 7 && len != 9)
			break;

		bt_shell_printf("Node %4.4x Model App status %s\n", src,
						mesh_status_str(data[0]));
		addr = get_le16(data + 1);
		app_idx = get_le16(data + 3);

		bt_shell_printf("Element Addr\t%4.4x\n", addr);

		mod_id = print_mod_id(data + 5, (len == 9) ? true : false);

		bt_shell_printf("AppIdx\t\t%3.3x\n ", app_idx);

		if (data[0] == MESH_STATUS_SUCCESS &&
			node_add_binding(node, addr - src, mod_id, app_idx))
			prov_db_add_binding(node, addr - src, mod_id, app_idx);
		break;

	case OP_NODE_IDENTITY_STATUS:
		if (len != 4)
			return true;
		bt_shell_printf("Network index 0x%04x "
				"Node Identity state 0x%02x status %s\n",
				get_le16(data + 1), data[3],
				mesh_status_str(data[0]));
		break;

	case OP_CONFIG_BEACON_STATUS:
		if (len != 1)
			return true;
		bt_shell_printf("Node %4.4x Config Beacon Status 0x%02x\n",
				src, data[0]);
		break;

	case OP_CONFIG_RELAY_STATUS:
		if (len != 2)
			return true;
		bt_shell_printf("Node %4.4x Relay state 0x%02x"
				" count %d steps %d\n",
				src, data[0], data[1]>>5, data[1] & 0x1f);
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
		if (node_set_default_ttl (node, data[0]))
			prov_db_node_set_ttl(node, data[0]);
		break;

	case OP_CONFIG_MODEL_PUB_STATUS:
		if (len != 12 && len != 14)
			return true;

		bt_shell_printf("\nNode %4.4x Publication status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		ele_addr = get_le16(data + 1);

		bt_shell_printf("Element Addr\t%04x\n", ele_addr);

		mod_id = print_mod_id(data + 10, (len == 14) ? true : false);

		pub.u.addr16 = get_le16(data + 3);
		pub.app_idx = get_le16(data + 5);
		pub.ttl = data[7];
		pub.period = data[8];
		n = (data[8] & 0x3f);
		bt_shell_printf("Pub Addr\t%04x\n", pub.u.addr16);
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

		pub.retransmit = data[9];
		bt_shell_printf("Rexmit count\t%d\n", data[9] >> 5);
		bt_shell_printf("Rexmit steps\t%d\n", data[9] & 0x1f);

		ele_idx = ele_addr - node_get_primary(node);

		/* Local configuration is saved by server */
		if (node == node_get_local_node())
			break;

		if (node_model_pub_set(node, ele_idx, mod_id, &pub))
			prov_db_node_set_model_pub(node, ele_idx, mod_id,
				     node_model_pub_get(node, ele_idx, mod_id));
		break;

	/* Per Mesh Profile 4.3.2.19 */
	case OP_CONFIG_MODEL_SUB_STATUS:
		bt_shell_printf("\nNode %4.4x Subscription status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		ele_addr = get_le16(data + 1);
		addr = get_le16(data + 3);
		ele_idx = ele_addr - node_get_primary(node);

		bt_shell_printf("Element Addr\t%4.4x\n", ele_addr);

		mod_id = print_mod_id(data + 5, (len == 9) ? true : false);

		bt_shell_printf("Subscr Addr\t%4.4x\n", addr);

		/* Save subscriptions in node and database */
		if (node_add_subscription(node, ele_idx, mod_id, addr))
			prov_db_add_subscription(node, ele_idx, mod_id, addr);
		break;

	/* Per Mesh Profile 4.3.2.27 */
	case OP_CONFIG_MODEL_SUB_LIST:

		bt_shell_printf("\nNode %4.4x Subscription List status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		bt_shell_printf("Element Addr\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Model ID\t%4.4x\n", get_le16(data + 3));

		for (i = 5; i < len; i += 2)
			bt_shell_printf("Subscr Addr\t%4.4x\n",
					get_le16(data + i));
		break;

	/* Per Mesh Profile 4.3.2.50 */
	case OP_MODEL_APP_LIST:
		bt_shell_printf("\nNode %4.4x Model AppIdx "
				"status %s\n", src,
				mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		bt_shell_printf("Element Addr\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Model ID\t%4.4x\n", get_le16(data + 3));

		for (i = 5; i < len; i += 2)
			bt_shell_printf("Model AppIdx\t%4.4x\n",
					get_le16(data + i));
		break;

	/* Per Mesh Profile 4.3.2.63 */
	case OP_CONFIG_HEARTBEAT_PUB_STATUS:
		bt_shell_printf("\nNode %4.4x Heartbeat publish status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		bt_shell_printf("Destination\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Count\t\t%2.2x\n", data[3]);
		bt_shell_printf("Period\t\t%2.2x\n", data[4]);
		bt_shell_printf("TTL\t\t%2.2x\n", data[5]);
		bt_shell_printf("Features\t%4.4x\n", get_le16(data + 6));
		bt_shell_printf("Net_Idx\t%4.4x\n", get_le16(data + 8));
		break;

	/* Per Mesh Profile 4.3.2.66 */
	case OP_CONFIG_HEARTBEAT_SUB_STATUS:
		bt_shell_printf("\nNode %4.4x Heartbeat subscribe status %s\n",
				src, mesh_status_str(data[0]));

		if (data[0] != MESH_STATUS_SUCCESS)
			return true;

		bt_shell_printf("Source\t\t%4.4x\n", get_le16(data + 1));
		bt_shell_printf("Destination\t%4.4x\n", get_le16(data + 3));
		bt_shell_printf("Period\t\t%2.2x\n", data[5]);
		bt_shell_printf("Count\t\t%2.2x\n", data[6]);
		bt_shell_printf("Min Hops\t%2.2x\n", data[7]);
		bt_shell_printf("Max Hops\t%2.2x\n", data[8]);
		break;

	/* Per Mesh Profile 4.3.2.54 */
	case OP_NODE_RESET_STATUS:
		bt_shell_printf("Node %4.4x reset status %s\n",
				src, mesh_status_str(data[0]));

		net_release_address(node_get_primary(node),
				(node_get_num_elements(node)));
		/* TODO: Remove node info from database */
		node_free(node);
		break;
	}

	return true;
}

static uint32_t target;
static uint32_t parms[8];

static uint32_t read_input_parameters(int argc, char *argv[])
{
	uint32_t i;

	if (!argc)
		return 0;

	--argc;
	++argv;

	if (!argc || argv[0][0] == '\0')
		return 0;

	memset(parms, 0xff, sizeof(parms));

	for (i = 0; i < sizeof(parms)/sizeof(parms[0]) && i < (unsigned) argc;
									i++) {
		sscanf(argv[i], "%x", &parms[i]);
		if (parms[i] == 0xffffffff)
			break;
	}

	return i;
}

static void cmd_node_set(int argc, char *argv[])
{
	uint32_t dst;
	char *end;

	dst = strtol(argv[1], &end, 16);
	if (end != (argv[1] + 4)) {
		bt_shell_printf("Bad unicast address %s: "
				"expected format 4 digit hex\n", argv[1]);
		target = UNASSIGNED_ADDRESS;
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else {
		bt_shell_printf("Configuring node %4.4x\n", dst);
		target = dst;
		set_menu_prompt("config", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}
}

static bool config_send(uint8_t *buf, uint16_t len)
{
	struct mesh_node *node = node_get_local_node();
	uint16_t primary;

	if(!node)
		return false;

	primary = node_get_primary(node);
	if (target != primary)
		return net_access_layer_send(DEFAULT_TTL, primary,
						target, APP_IDX_DEV, buf, len);

	node_local_data_handler(primary, target, node_get_iv_index(node),
				node_get_sequence_number(node), APP_IDX_DEV,
				buf, len);
	return true;

}

static void cmd_default(uint32_t opcode)
{
	uint16_t n;
	uint8_t msg[32];

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(opcode, msg);

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send command (opcode 0x%x)\n",
								opcode);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_composition_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	struct mesh_node *node;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	node = node_find_by_addr(target);

	if (!node)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_DEV_COMP_GET, msg);

	/* By default, use page 0 */
	msg[n++] = (read_input_parameters(argc, argv) == 1) ? parms[0] : 0;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"GET NODE COMPOSITION\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_net_key(int argc, char *argv[], uint32_t opcode)
{
	uint16_t n;
	uint8_t msg[32];
	uint16_t net_idx;
	uint8_t *key;
	struct mesh_node *node;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(opcode, msg);

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	node = node_find_by_addr(target);
	if (!node) {
		bt_shell_printf("Node %4.4x\n not found", target);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	net_idx = parms[0];

	if (opcode != OP_NETKEY_DELETE) {

		key = keys_net_key_get(net_idx, true);
		if (!key) {
			bt_shell_printf("NetKey with index %4.4x not found\n",
								net_idx);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		put_le16(net_idx, &msg[n]);
		n += 2;

		memcpy(msg + n, key, 16);
		n += 16;
	}

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"%s NET KEY\"\n",
				opcode == OP_NETKEY_ADD ? "ADD" : "DEL");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (opcode != OP_NETKEY_DELETE) {
		if (node_net_key_add(node, net_idx))
			prov_db_node_keys(node, node_get_net_keys(node),
								"netKeys");
	} else {
		if (node_net_key_delete(node, net_idx))
			prov_db_node_keys(node, node_get_net_keys(node),
								"netKeys");
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_netkey_add(int argc, char *argv[])
{
	cmd_net_key(argc, argv, OP_NETKEY_ADD);
}

static void cmd_netkey_del(int argc, char *argv[])
{
	cmd_net_key(argc, argv, OP_NETKEY_DELETE);
}

static void cmd_app_key(int argc, char *argv[], uint32_t opcode)
{
	uint16_t n;
	uint8_t msg[32];
	uint16_t net_idx;
	uint16_t app_idx;
	uint8_t *key;
	struct mesh_node *node;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (read_input_parameters(argc, argv) != 1) {
		bt_shell_printf("Bad arguments %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	node = node_find_by_addr(target);
	if (!node) {
		bt_shell_printf("Node %4.4x\n not found", target);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(opcode, msg);

	app_idx = parms[0];
	net_idx = keys_app_key_get_bound(app_idx);
	if (net_idx == NET_IDX_INVALID) {
		bt_shell_printf("AppKey with index %4.4x not found\n", app_idx);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = net_idx & 0xff;
	msg[n++] = ((net_idx >> 8) & 0xf) |
		((app_idx << 4) & 0xf0);
	msg[n++] = app_idx >> 4;

	if (opcode != OP_APPKEY_DELETE) {
		key = keys_app_key_get(app_idx, true);
		if (!key) {
			bt_shell_printf("AppKey %4.4x not found\n", net_idx);
			return;
		}

		memcpy(msg + n, key, 16);
		n += 16;
	}

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"ADD %s KEY\"\n",
				opcode == OP_APPKEY_ADD ? "ADD" : "DEL");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (opcode != OP_APPKEY_DELETE) {
		if (node_app_key_add(node, app_idx))
			prov_db_node_keys(node, node_get_app_keys(node),
								"appKeys");
	} else {
		if (node_app_key_delete(node, net_idx, app_idx))
			prov_db_node_keys(node, node_get_app_keys(node),
								"appKeys");
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_appkey_add(int argc, char *argv[])
{
	cmd_app_key(argc, argv, OP_APPKEY_ADD);
}

static void cmd_appkey_del(int argc, char *argv[])
{
	cmd_app_key(argc, argv, OP_APPKEY_DELETE);
}

static bool verify_config_target(uint32_t dst)
{
	struct mesh_node *node;

	if (IS_UNASSIGNED(dst)) {
		bt_shell_printf("Destination not set\n");
		return false;
	}

	node = node_find_by_addr(dst);
	if (!node) {
		bt_shell_printf("Node with unicast address %4.4x unknown\n",
				dst);
		return false;
	}

	if (!node_get_composition(node)) {
		bt_shell_printf("Node composition for %4.4x unknown\n", dst);
		return false;
	}

	return true;
}

static void cmd_bind(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3 && parm_cnt != 4) {
		bt_shell_printf("Bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_MODEL_APP_BIND, msg);

	put_le16(target + parms[0], msg + n);
	n += 2;
	put_le16(parms[1], msg + n);
	n += 2;
	if (parm_cnt == 4) {
		put_le16(parms[3], msg + n);
		put_le16(parms[2], msg + n + 2);
		n += 4;
	} else {
		put_le16(parms[2], msg + n);
		n += 2;
	}

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"MODEL APP BIND\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_beacon_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_CONFIG_BEACON_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET BEACON\"\n");
		return;
	}

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
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_NODE_IDENTITY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;
	msg[n++] = parms[1];

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET IDENTITY\"\n");
		return;
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ident_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 2 + 4];
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_NODE_IDENTITY_GET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"GET IDENTITY\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_proxy_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[2 + 1];
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_CONFIG_PROXY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 1) {
		bt_shell_printf("bad arguments");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET PROXY\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

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
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_CONFIG_RELAY_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3) {
		bt_shell_printf("bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	msg[n++] = parms[0];
	msg[n++] = (parms[1] << 5) | parms[2];

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET RELAY\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

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
	int parm_cnt;
	uint8_t ttl;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_DEFAULT_TTL_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt) {
		ttl = parms[0] & TTL_MASK;
	} else
		ttl = node_get_default_ttl(node_get_local_node());

	msg[n++] = ttl;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET_DEFAULT TTL\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_pub_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (!verify_config_target(target))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	n = mesh_opcode_set(OP_CONFIG_MODEL_PUB_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 6 && parm_cnt != 7) {
		bt_shell_printf("Bad arguments\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	put_le16(parms[0], msg + n);
	n += 2;
	/* Publish address */
	put_le16(parms[1], msg + n);
	n += 2;
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
	if (parm_cnt == 7) {
		put_le16(parms[6], msg + n);
		put_le16(parms[5], msg + n + 2);
		n += 4;
	} else {
		put_le16(parms[5], msg + n);
		n += 2;
	}

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET MODEL PUBLICATION\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_pub_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

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
	if (parm_cnt == 3) {
		put_le16(parms[2], msg + n);
		put_le16(parms[1], msg + n + 2);
		n += 4;
	} else {
		put_le16(parms[1], msg + n);
		n += 2;
	}

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"GET MODEL PUBLICATION\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_sub_add(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_MODEL_SUB_ADD, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Per Mesh Profile 4.3.2.19 */
	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Subscription Address */
	put_le16(parms[1], msg + n);
	n += 2;
	/* SIG Model ID */
	put_le16(parms[2], msg + n);
	n += 2;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"ADD SUBSCRIPTION\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_sub_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_MODEL_SUB_GET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Per Mesh Profile 4.3.2.27 */
	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Model ID */
	put_le16(parms[1], msg + n);
	n += 2;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"GET SUB GET\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_mod_appidx_get(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_MODEL_APP_GET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 2) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Per Mesh Profile 4.3.2.49 */
	/* Element Address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Model ID */
	put_le16(parms[1], msg + n);
	n += 2;

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"GET APP GET\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_hb_pub_set(int argc, char *argv[])
{
	uint16_t n;
	uint8_t msg[32];
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_HEARTBEAT_PUB_SET, msg);

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

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET HEARTBEAT PUBLISH\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

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
	int parm_cnt;

	if (IS_UNASSIGNED(target)) {
		bt_shell_printf("Destination not set\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	n = mesh_opcode_set(OP_CONFIG_HEARTBEAT_SUB_SET, msg);

	parm_cnt = read_input_parameters(argc, argv);
	if (parm_cnt != 3) {
		bt_shell_printf("Bad arguments: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* Per Mesh Profile 4.3.2.65 */
	/* Source address */
	put_le16(parms[0], msg + n);
	n += 2;
	/* Destination address */
	put_le16(parms[1], msg + n);
	n += 2;
	/* Period log */
	msg[n++] = parms[2];

	if (!config_send(msg, n)) {
		bt_shell_printf("Failed to send \"SET HEARTBEAT SUBSCRIBE\"\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

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

static void cmd_node_reset(int argc, char *argv[])
{
	cmd_default(OP_NODE_RESET);
}

static const struct bt_shell_menu cfg_menu = {
	.name = "config",
	.desc = "Configuration Model Submenu",
	.entries = {
	{"target",		"<unicast>",		cmd_node_set,
						"Set target node to configure"},
	{"composition-get",	"[page_num]",		cmd_composition_get,
						"Get composition data"},
	{"netkey-add",		"<net_idx>",		cmd_netkey_add,
						"Add network key"},
	{"netkey-del",		"<net_idx>",		cmd_netkey_del,
						"Delete network key"},
	{"appkey-add",		"<app_idx>",		cmd_appkey_add,
						"Add application key"},
	{"appkey-del",		"<app_idx>",		cmd_appkey_del,
						"Delete application key"},
	{"bind",		"<ele_idx> <app_idx> <mod_id> [cid]",
				cmd_bind,	"Bind app key to a model"},
	{"mod-appidx-get", "<ele_addr> <model id>",
				cmd_mod_appidx_get,    "Get model app_idx"},
	{"ttl-set",		"<ttl>",		cmd_ttl_set,
						"Set default TTL"},
	{"ttl-get",		NULL,			cmd_ttl_get,
						"Get default TTL"},
	{"pub-set", "<ele_addr> <pub_addr> <app_idx> "
			"<per (step|res)> <re-xmt (cnt|per)> <mod id> "
			"[cid]",
			cmd_pub_set,	"\n\t\t\t\t\t\t  Set publication"},
	{"pub-get", "<ele_addr> <model>",               cmd_pub_get,
						"Get publication"},
	{"proxy-set",           "<proxy>",              cmd_proxy_set,
						"Set proxy state"},
	{"proxy-get",           NULL,                   cmd_proxy_get,
						"Get proxy state"},
	{"ident-set",           "<net_idx> <state>",    cmd_ident_set,
						"Set node identity state"},
	{"ident-get",           "<net_idx>",            cmd_ident_get,
						"Get node identity state"},
	{"beacon-set",           "<state>",             cmd_beacon_set,
						"Set node identity state"},
	{"beacon-get",           NULL,                  cmd_beacon_get,
						"Get node beacon state"},
	{"relay-set",           "<relay> <rexmt count> <rexmt steps>",
						cmd_relay_set,
						"Set relay"},
	{"relay-get",           NULL,                   cmd_relay_get,
						"Get relay"},
	{"hb-pub-set", "<pub_addr> <count> <period> <ttl> <features> <net_idx>",
				cmd_hb_pub_set,	"Set heartbeat publish"},
	{"hb-pub-get",           NULL,                   cmd_hb_pub_get,
						"Get heartbeat publish"},
	{"hb-sub-set", "<src_addr> <dst_addr> <period>",
				cmd_hb_sub_set,     "Set heartbeat subscribe"},
	{"hb-sub-get",           NULL,                   cmd_hb_sub_get,
						"Get heartbeat subscribe"},
	{"sub-add", "<ele_addr> <sub_addr> <model id>",
				cmd_sub_add,    "Add subscription"},
	{"sub-get", "<ele_addr> <model id>",
				cmd_sub_get,    "Get subscription"},
	{"node-reset",		NULL,                    cmd_node_reset,
				"Reset a node and remove it from network"},
	{} },
};

void config_client_get_composition(uint32_t dst)
{
	uint32_t tmp = target;

	target = dst;
	cmd_composition_get(0, NULL);
	target = tmp;
}

static struct mesh_model_ops client_cbs = {
	client_msg_recvd,
		NULL,
		NULL,
		NULL
};

bool config_client_init(void)
{
	if (!node_local_model_register(PRIMARY_ELEMENT_IDX,
						CONFIG_CLIENT_MODEL_ID,
						&client_cbs, NULL))
		return false;

	bt_shell_add_submenu(&cfg_menu);

	return true;
}
