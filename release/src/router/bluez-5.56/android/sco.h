/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

enum sco_status {
	SCO_STATUS_OK,
	SCO_STATUS_ERROR,
};

struct bt_sco;

struct bt_sco *bt_sco_new(const bdaddr_t *local_bdaddr);

struct bt_sco *bt_sco_ref(struct bt_sco *sco);
void bt_sco_unref(struct bt_sco *sco);

bool bt_sco_connect(struct bt_sco *sco, const bdaddr_t *remote_addr,
						uint16_t voice_settings);
void bt_sco_disconnect(struct bt_sco *sco);
bool bt_sco_get_fd_and_mtu(struct bt_sco *sco, int *fd, uint16_t *mtu);

typedef bool (*bt_sco_confirm_func_t) (const bdaddr_t *remote_addr,
						uint16_t *voice_settings);
typedef void (*bt_sco_conn_func_t) (enum sco_status status,
							const bdaddr_t *addr);
typedef void (*bt_sco_disconn_func_t) (const bdaddr_t *addr);

void bt_sco_set_confirm_cb(struct bt_sco *sco,
					bt_sco_confirm_func_t func);
void bt_sco_set_connect_cb(struct bt_sco *sco, bt_sco_conn_func_t func);
void bt_sco_set_disconnect_cb(struct bt_sco *sco,
						bt_sco_disconn_func_t func);
