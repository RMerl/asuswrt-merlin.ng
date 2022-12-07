/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#define BLUEZ_MESH_PATH "/org/bluez/mesh"
#define BLUEZ_MESH_SERVICE "org.bluez.mesh"

#define DEFAULT_DBUS_TIMEOUT	30

bool dbus_init(struct l_dbus *dbus);
struct l_dbus *dbus_get_bus(void);
void dbus_append_byte_array(struct l_dbus_message_builder *builder,
						const uint8_t *data, int len);
void dbus_append_dict_entry_basic(struct l_dbus_message_builder *builder,
					const char *key, const char *signature,
					const void *data);
bool dbus_match_interface(struct l_dbus_message_iter *interfaces,
							const char *match);
struct l_dbus_message *dbus_error(struct l_dbus_message *msg, int err,
						const char *description);
void dbus_send_with_timeout(struct l_dbus *dbus, struct l_dbus_message *msg,
						l_dbus_message_func_t cb,
						void *user_data,
						l_dbus_destroy_func_t destroy,
						unsigned int seconds);
