/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

struct mesh_group {
	uint16_t addr;
	uint8_t label[16];
};

typedef bool (*key_send_func_t) (void *user_data, uint16_t dst,
				 uint16_t idx, bool is_appkey, bool update);
typedef void (*delete_remote_func_t) (uint16_t primary, uint8_t ele_cnt);

struct model_info *cfgcli_init(key_send_func_t key_func,
				delete_remote_func_t del_node, void *user_data);
void cfgcli_cleanup(void);
