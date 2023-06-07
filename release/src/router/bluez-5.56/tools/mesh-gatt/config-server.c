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
#include "tools/mesh/config-model.h"

#include "src/shared/util.h"
#include "src/shared/shell.h"
#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/keys.h"
#include "tools/mesh-gatt/net.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/prov-db.h"
#include "tools/mesh-gatt/util.h"

static bool server_msg_recvd(uint16_t src, uint8_t *data,
				uint16_t len, void *user_data)
{
	uint32_t opcode;
	uint8_t msg[32];
	struct mesh_node *node;
	uint16_t primary;
	uint32_t mod_id;
	uint16_t ele_addr;
	uint8_t ele_idx;
	struct mesh_publication pub;
	int m, n;

	if (mesh_opcode_get(data, len, &opcode, &n)) {
		len -= n;
		data += n;
	} else
		return false;

	node = node_get_local_node();

	if (!node)
		return true;

	n = 0;

	switch (opcode) {
	default:
		return false;

	case OP_CONFIG_DEFAULT_TTL_SET:
		if (len != 1 || data[0] > TTL_MASK || data[0] == 1)
			return true;

		if (data[0] <= TTL_MASK) {
			node_set_default_ttl(node, data[0]);
			prov_db_node_set_ttl(node, data[0]);
		}

		/* Fall Through */

	case OP_CONFIG_DEFAULT_TTL_GET:
		n = mesh_opcode_set(OP_CONFIG_DEFAULT_TTL_STATUS, msg);
		msg[n++] = node_get_default_ttl(node);
		break;

	case OP_CONFIG_MODEL_PUB_SET:

		if (len != 11 && len != 13)
			return true;

		bt_shell_printf("Set publication\n");

		ele_addr = get_le16(data);
		mod_id = get_le16(data + 9);
		if (len == 14)
			mod_id = (mod_id << 16)  | get_le16(data + 11);
		else
			mod_id |= 0xffff0000;

		pub.u.addr16 = get_le16(data + 2);
		pub.app_idx = get_le16(data + 4);
		pub.ttl = data[6];
		pub.period = data[7];
		m = (data[7] & 0x3f);
		switch (data[7] >> 6) {
		case 0:
			bt_shell_printf("Period: %d ms\n", m * 100);
			break;
		case 2:
			m *= 10;
			/* fall through */
		case 1:
			bt_shell_printf("Period: %d sec\n", m);
			break;
		case 3:
			bt_shell_printf("Period: %d min\n", m * 10);
			break;
		}

		pub.retransmit = data[8];
		bt_shell_printf("Retransmit count: %d\n", data[8] >> 5);
		bt_shell_printf("Retransmit Interval Steps: %d\n",
				data[8] & 0x1f);

		ele_idx = ele_addr - node_get_primary(node);

		if (node_model_pub_set(node, ele_idx, mod_id, &pub)) {
			prov_db_node_set_model_pub(node, ele_idx, mod_id,
				     node_model_pub_get(node, ele_idx, mod_id));
		}
		break;
	}

	if (!n)
		return true;

	primary = node_get_primary(node);
	if (src != primary)
		net_access_layer_send(node_get_default_ttl(node), primary,
					src, APP_IDX_DEV, msg, n);
	else
		node_local_data_handler(primary, src, node_get_iv_index(node),
					node_get_sequence_number(node),
					APP_IDX_DEV, msg, n);
	return true;
}


static struct mesh_model_ops server_cbs = {
	server_msg_recvd,
		NULL,
		NULL,
		NULL
};

bool config_server_init(void)
{
	if (!node_local_model_register(PRIMARY_ELEMENT_IDX,
						CONFIG_SERVER_MODEL_ID,
						&server_cbs, NULL))
		return false;

	return true;
}
