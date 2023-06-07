/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
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

#ifndef __ELL_SERVICE_H
#define __ELL_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_dbus;
struct l_dbus_interface;
struct l_dbus_message;

enum l_dbus_method_flag {
	L_DBUS_METHOD_FLAG_DEPRECATED =	1,
	L_DBUS_METHOD_FLAG_NOREPLY =	2,
	L_DBUS_METHOD_FLAG_ASYNC =	4,
};

enum l_dbus_signal_flag {
	L_DBUS_SIGNAL_FLAG_DEPRECATED =	1,
};

enum l_dbus_property_flag {
	L_DBUS_PROPERTY_FLAG_DEPRECATED = 1,
	L_DBUS_PROPERTY_FLAG_AUTO_EMIT	= 2,
};

typedef struct l_dbus_message *(*l_dbus_interface_method_cb_t) (struct l_dbus *,
						struct l_dbus_message *message,
						void *user_data);

typedef void (*l_dbus_property_complete_cb_t) (struct l_dbus *,
						struct l_dbus_message *,
						struct l_dbus_message *error);

typedef struct l_dbus_message *(*l_dbus_property_set_cb_t) (struct l_dbus *,
					struct l_dbus_message *message,
					struct l_dbus_message_iter *new_value,
					l_dbus_property_complete_cb_t complete,
					void *user_data);

typedef bool (*l_dbus_property_get_cb_t) (struct l_dbus *,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data);

bool l_dbus_interface_method(struct l_dbus_interface *interface,
				const char *name, uint32_t flags,
				l_dbus_interface_method_cb_t cb,
				const char *return_sig, const char *param_sig,
				...);

bool l_dbus_interface_signal(struct l_dbus_interface *interface,
				const char *name, uint32_t flags,
				const char *signature, ...);

bool l_dbus_interface_property(struct l_dbus_interface *interface,
				const char *name, uint32_t flags,
				const char *signature,
				l_dbus_property_get_cb_t getter,
				l_dbus_property_set_cb_t setter);

bool l_dbus_property_changed(struct l_dbus *dbus, const char *path,
				const char *interface, const char *property);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_DBUS_SERVICE_H */
