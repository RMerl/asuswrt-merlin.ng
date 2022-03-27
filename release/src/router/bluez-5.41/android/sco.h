/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
