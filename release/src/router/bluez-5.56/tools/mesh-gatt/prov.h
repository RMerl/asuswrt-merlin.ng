/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

struct prov;

typedef void (*provision_done_cb)(void *user_data, int status);

bool prov_open(struct mesh_node *node, GDBusProxy *prov_in, uint16_t net_idx,
		provision_done_cb cb, void *user_data);
bool prov_data_ready(struct mesh_node *node, uint8_t *buf, uint8_t len);
bool prov_complete(struct mesh_node *node, uint8_t status);
bool prov_set_sec_level(uint8_t level);
uint8_t prov_get_sec_level(void);
