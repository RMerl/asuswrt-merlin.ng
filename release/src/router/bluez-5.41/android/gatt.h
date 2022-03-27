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

bool bt_gatt_register(struct ipc *ipc, const bdaddr_t *addr);
void bt_gatt_unregister(void);


typedef enum {
	GATT_CLIENT,
	GATT_SERVER,
} gatt_type_t;

typedef void (*gatt_conn_cb_t)(const bdaddr_t *addr, int err, void *attrib);

unsigned int bt_gatt_register_app(const char *uuid, gatt_type_t type,
							gatt_conn_cb_t func);
bool bt_gatt_unregister_app(unsigned int id);

bool bt_gatt_connect_app(unsigned int id, const bdaddr_t *addr);
bool bt_gatt_disconnect_app(unsigned int id, const bdaddr_t *addr);
bool bt_gatt_set_security(const bdaddr_t *bdaddr, int sec_level);
bool bt_gatt_add_autoconnect(unsigned int id, const bdaddr_t *addr);
void bt_gatt_remove_autoconnect(unsigned int id, const bdaddr_t *addr);
