// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/node.h"
#include "mesh/manager.h"
#include "mesh/mesh.h"
#include "mesh/error.h"
#include "mesh/dbus.h"

static struct l_dbus *dbus;

struct error_entry {
	const char *dbus_err;
	const char *default_desc;
};

struct send_info {
	struct l_dbus *dbus;
	struct l_timeout *timeout;
	l_dbus_message_func_t cb;
	l_dbus_destroy_func_t destroy;
	void *user_data;
	uint32_t serial;
};

/*
 * Important: The entries in this table follow the order of
 * enumerated values in mesh_error (file error.h)
 */
static struct error_entry const error_table[] =
{
	{ NULL, NULL },
	{ ERROR_INTERFACE ".Failed", "Operation failed" },
	{ ERROR_INTERFACE ".NotAuthorized", "Permission denied"},
	{ ERROR_INTERFACE ".NotFound", "Object not found"},
	{ ERROR_INTERFACE ".InvalidArgs", "Invalid arguments"},
	{ ERROR_INTERFACE ".InProgress", "Operation already in progress"},
	{ ERROR_INTERFACE ".Busy", "Busy"},
	{ ERROR_INTERFACE ".AlreadyExists", "Already exists"},
	{ ERROR_INTERFACE ".DoesNotExist", "Does not exist"},
	{ ERROR_INTERFACE ".Canceled", "Operation canceled"},
	{ ERROR_INTERFACE ".NotImplemented", "Not implemented"},
};

struct l_dbus_message *dbus_error(struct l_dbus_message *msg, int err,
							const char *description)
{
	int array_len = L_ARRAY_SIZE(error_table);

	/* Default to ".Failed" */
	if (!err || err >= array_len)
		err = MESH_ERROR_FAILED;

	if (description)
		return l_dbus_message_new_error(msg,
				error_table[err].dbus_err,
				"%s", description);
	else
		return l_dbus_message_new_error(msg,
				error_table[err].dbus_err,
				"%s", error_table[err].default_desc);
}

struct l_dbus *dbus_get_bus(void)
{
	return dbus;
}

bool dbus_init(struct l_dbus *bus)
{
	/* Network interface */
	if (!mesh_dbus_init(bus))
		return false;

	/* Node interface */
	if (!node_dbus_init(bus))
		return false;

	/* Management interface */
	if (!manager_dbus_init(bus))
		return false;

	dbus = bus;

	return true;
}

bool dbus_match_interface(struct l_dbus_message_iter *interfaces,
							const char *match)
{
	const char *interface;
	struct l_dbus_message_iter properties;

	while (l_dbus_message_iter_next_entry(interfaces, &interface,
								&properties)) {
		if (!strcmp(match, interface))
			return true;
	}

	return false;
}

void dbus_append_byte_array(struct l_dbus_message_builder *builder,
						const uint8_t *data, int len)
{
	int i;

	if (!builder)
		return;

	l_dbus_message_builder_enter_array(builder, "y");

	for (i = 0; i < len; i++)
		l_dbus_message_builder_append_basic(builder, 'y', data + i);

	l_dbus_message_builder_leave_array(builder);
}

void dbus_append_dict_entry_basic(struct l_dbus_message_builder *builder,
					const char *key, const char *signature,
					const void *data)
{
	if (!builder)
		return;

	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_append_basic(builder, 's', key);
	l_dbus_message_builder_enter_variant(builder, signature);
	l_dbus_message_builder_append_basic(builder, signature[0], data);
	l_dbus_message_builder_leave_variant(builder);
	l_dbus_message_builder_leave_dict(builder);
}

static void send_reply(struct l_dbus_message *message, void *user_data)
{
	struct send_info *info = user_data;

	l_timeout_remove(info->timeout);
	info->cb(message, info->user_data);

	if (info->destroy)
		info->destroy(info->user_data);

	l_free(info);
}

static void send_timeout(struct l_timeout *timeout, void *user_data)
{
	struct send_info *info = user_data;

	l_dbus_cancel(info->dbus, info->serial);
	send_reply(NULL, info);
}

void dbus_send_with_timeout(struct l_dbus *dbus, struct l_dbus_message *msg,
						l_dbus_message_func_t cb,
						void *user_data,
						l_dbus_destroy_func_t destroy,
						unsigned int seconds)
{
	struct send_info *info = l_new(struct send_info, 1);

	info->dbus = dbus;
	info->cb = cb;
	info->user_data = user_data;
	info->destroy = destroy;
	info->serial = l_dbus_send_with_reply(dbus, msg, send_reply,
								info, NULL);
	info->timeout = l_timeout_create(seconds, send_timeout, info, NULL);
}
