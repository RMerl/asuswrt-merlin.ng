/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
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
