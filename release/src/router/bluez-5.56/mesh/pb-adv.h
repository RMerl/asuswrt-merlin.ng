/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

bool pb_adv_reg(bool initiator, mesh_prov_open_func_t open_cb,
		mesh_prov_close_func_t close_cb,
		mesh_prov_receive_func_t rx_cb, mesh_prov_ack_func_t ack_cb,
		uint8_t uuid[16], void *user_data);
void pb_adv_unreg(void *user_data);
