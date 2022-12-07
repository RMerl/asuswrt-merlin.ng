/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020  Intel Corporation. All rights reserved.
 *
 *
 */

struct mesh_rpl {
	uint32_t iv_index;
	uint32_t seq;
	uint16_t src;
};

bool rpl_put_entry(struct mesh_node *node, uint16_t src, uint32_t iv_index,
								uint32_t seq);
void rpl_del_entry(struct mesh_node *node, uint16_t src);
bool rpl_get_list(struct mesh_node *node, struct l_queue *rpl_list);
void rpl_update(struct mesh_node *node, uint32_t iv_index);
bool rpl_init(const char *node_path);
