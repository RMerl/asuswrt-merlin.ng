/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *  Copyright (C) 2017  Codecoup. All rights reserved.
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

#ifndef __ELL_DBUS_CLIENT_H
#define __ELL_DBUS_CLIENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_dbus;
struct l_dbus_message;
struct l_dbus_client;
struct l_dbus_proxy;

typedef void (*l_dbus_client_ready_func_t)(struct l_dbus_client *client,
							void *user_data);
typedef void (*l_dbus_client_proxy_func_t) (struct l_dbus_proxy *proxy,
							void *user_data);
typedef void (*l_dbus_client_proxy_result_func_t) (struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data);
typedef void (*l_dbus_client_property_function_t) (struct l_dbus_proxy *proxy,
						const char *name,
						struct l_dbus_message *msg,
						void *user_data);

struct l_dbus_client *l_dbus_client_new(struct l_dbus *dbus,
					const char *service, const char *path);
void l_dbus_client_destroy(struct l_dbus_client *client);

bool l_dbus_client_set_connect_handler(struct l_dbus_client *client,
						l_dbus_watch_func_t function,
						void *user_data,
						l_dbus_destroy_func_t destroy);

bool l_dbus_client_set_disconnect_handler(struct l_dbus_client *client,
						l_dbus_watch_func_t function,
						void *user_data,
						l_dbus_destroy_func_t destroy);

bool l_dbus_client_set_ready_handler(struct l_dbus_client *client,
					l_dbus_client_ready_func_t function,
					void *user_data,
					l_dbus_destroy_func_t destroy);

bool l_dbus_client_set_proxy_handlers(struct l_dbus_client *client,
			l_dbus_client_proxy_func_t proxy_added,
			l_dbus_client_proxy_func_t proxy_removed,
			l_dbus_client_property_function_t property_changed,
			void *user_data, l_dbus_destroy_func_t destroy);

const char *l_dbus_proxy_get_path(struct l_dbus_proxy *proxy);

const char *l_dbus_proxy_get_interface(struct l_dbus_proxy *proxy);

bool l_dbus_proxy_get_property(struct l_dbus_proxy *proxy, const char *name,
						const char *signature, ...);

bool l_dbus_proxy_set_property(struct l_dbus_proxy *proxy,
				l_dbus_client_proxy_result_func_t result,
				void *user_data, l_dbus_destroy_func_t destroy,
				const char *name, const char *signature, ...);

uint32_t l_dbus_proxy_method_call(struct l_dbus_proxy *proxy,
					const char *method,
					l_dbus_message_func_t setup,
					l_dbus_client_proxy_result_func_t reply,
					void *user_data,
					l_dbus_destroy_func_t destroy);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_DBUS_CLIENT_H */
