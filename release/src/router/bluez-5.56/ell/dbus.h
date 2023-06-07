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

#ifndef __ELL_DBUS_H
#define __ELL_DBUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_DBUS_INTERFACE_DBUS		"org.freedesktop.DBus"
#define L_DBUS_INTERFACE_INTROSPECTABLE	"org.freedesktop.DBus.Introspectable"
#define L_DBUS_INTERFACE_PROPERTIES	"org.freedesktop.DBus.Properties"
#define L_DBUS_INTERFACE_OBJECT_MANAGER	"org.freedesktop.DBus.ObjectManager"

enum l_dbus_bus {
	L_DBUS_SYSTEM_BUS,
	L_DBUS_SESSION_BUS,
};

enum l_dbus_match_type {
	L_DBUS_MATCH_NONE = 0,
	L_DBUS_MATCH_TYPE,
	L_DBUS_MATCH_SENDER,
	L_DBUS_MATCH_PATH,
	L_DBUS_MATCH_INTERFACE,
	L_DBUS_MATCH_MEMBER,
	L_DBUS_MATCH_ARG0,
};

#define L_DBUS_MATCH_ARGUMENT(i)	(L_DBUS_MATCH_ARG0 + (i))

struct l_dbus;
struct l_dbus_interface;
struct l_dbus_message_builder;

typedef void (*l_dbus_ready_func_t) (void *user_data);
typedef void (*l_dbus_disconnect_func_t) (void *user_data);

typedef void (*l_dbus_debug_func_t) (const char *str, void *user_data);
typedef void (*l_dbus_destroy_func_t) (void *user_data);
typedef void (*l_dbus_interface_setup_func_t) (struct l_dbus_interface *);

typedef void (*l_dbus_watch_func_t) (struct l_dbus *dbus, void *user_data);

typedef void (*l_dbus_name_acquire_func_t) (struct l_dbus *dbus, bool success,
						bool queued, void *user_data);

struct l_dbus *l_dbus_new(const char *address);
struct l_dbus *l_dbus_new_default(enum l_dbus_bus bus);
void l_dbus_destroy(struct l_dbus *dbus);

bool l_dbus_set_ready_handler(struct l_dbus *dbus, l_dbus_ready_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);
bool l_dbus_set_disconnect_handler(struct l_dbus *dbus,
				l_dbus_disconnect_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);

bool l_dbus_set_debug(struct l_dbus *dbus, l_dbus_debug_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);

struct l_dbus_message;

struct l_dbus_message_iter {
	struct l_dbus_message *message;
	const char *sig_start;
	uint8_t sig_len;
	uint8_t sig_pos;
	const void *data;
	size_t len;
	size_t pos;
	char container_type;
	const void *offsets;
};

struct l_dbus_message *l_dbus_message_new_method_call(struct l_dbus *dbus,
							const char *destination,
							const char *path,
							const char *interface,
							const char *method);

struct l_dbus_message *l_dbus_message_new_signal(struct l_dbus *dbus,
							const char *path,
							const char *interface,
							const char *name);

struct l_dbus_message *l_dbus_message_new_method_return(
					struct l_dbus_message *method_call);

struct l_dbus_message *l_dbus_message_new_error_valist(
					struct l_dbus_message *method_call,
					const char *name,
					const char *format, va_list args)
					__attribute__((format(printf, 3, 0)));
struct l_dbus_message *l_dbus_message_new_error(
					struct l_dbus_message *method_call,
					const char *name,
					const char *format, ...)
					__attribute__((format(printf, 3, 4)));

struct l_dbus_message *l_dbus_message_ref(struct l_dbus_message *message);
void l_dbus_message_unref(struct l_dbus_message *message);

const char *l_dbus_message_get_path(struct l_dbus_message *message);
const char *l_dbus_message_get_interface(struct l_dbus_message *message);
const char *l_dbus_message_get_member(struct l_dbus_message *message);
const char *l_dbus_message_get_destination(struct l_dbus_message *message);
const char *l_dbus_message_get_sender(struct l_dbus_message *message);
const char *l_dbus_message_get_signature(struct l_dbus_message *message);

bool l_dbus_message_set_no_reply(struct l_dbus_message *message, bool on);
bool l_dbus_message_get_no_reply(struct l_dbus_message *message);

bool l_dbus_message_set_no_autostart(struct l_dbus_message *message, bool on);
bool l_dbus_message_get_no_autostart(struct l_dbus_message *message);

typedef void (*l_dbus_message_func_t) (struct l_dbus_message *message,
							void *user_data);

uint32_t l_dbus_send_with_reply(struct l_dbus *dbus,
				struct l_dbus_message *message,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);
uint32_t l_dbus_send(struct l_dbus *dbus,
				struct l_dbus_message *message);
bool l_dbus_cancel(struct l_dbus *dbus, uint32_t serial);

unsigned int l_dbus_register(struct l_dbus *dbus,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);
bool l_dbus_unregister(struct l_dbus *dbus, unsigned int id);

uint32_t l_dbus_method_call(struct l_dbus *dbus,
				const char *destination, const char *path,
				const char *interface, const char *method,
				l_dbus_message_func_t setup,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy);

bool l_dbus_message_is_error(struct l_dbus_message *message);
bool l_dbus_message_get_error(struct l_dbus_message *message,
					const char **name, const char **text);
bool l_dbus_message_get_arguments(struct l_dbus_message *message,
						const char *signature, ...);
bool l_dbus_message_get_arguments_valist(struct l_dbus_message *message,
					const char *signature, va_list args);

bool l_dbus_message_iter_next_entry(struct l_dbus_message_iter *iter, ...);
bool l_dbus_message_iter_get_variant(struct l_dbus_message_iter *iter,
						const char *signature, ...);
bool l_dbus_message_iter_get_fixed_array(struct l_dbus_message_iter *iter,
						void *out, uint32_t *n_elem);

bool l_dbus_message_set_arguments(struct l_dbus_message *message,
						const char *signature, ...);
bool l_dbus_message_set_arguments_valist(struct l_dbus_message *message,
					 const char *signature, va_list args);

struct l_dbus_message_builder *l_dbus_message_builder_new(
						struct l_dbus_message *message);
void l_dbus_message_builder_destroy(struct l_dbus_message_builder *builder);

bool l_dbus_message_builder_append_basic(struct l_dbus_message_builder *builder,
					char type, const void *value);

bool l_dbus_message_builder_enter_container(
					struct l_dbus_message_builder *builder,
					char container_type,
					const char *signature);
bool l_dbus_message_builder_leave_container(
					struct l_dbus_message_builder *builder,
					char container_type);

bool l_dbus_message_builder_enter_struct(struct l_dbus_message_builder *builder,
						const char *signature);
bool l_dbus_message_builder_leave_struct(
					struct l_dbus_message_builder *builder);

bool l_dbus_message_builder_enter_dict(struct l_dbus_message_builder *builder,
					const char *signature);
bool l_dbus_message_builder_leave_dict(struct l_dbus_message_builder *builder);

bool l_dbus_message_builder_enter_array(struct l_dbus_message_builder *builder,
					const char *signature);
bool l_dbus_message_builder_leave_array(struct l_dbus_message_builder *builder);

bool l_dbus_message_builder_enter_variant(
					struct l_dbus_message_builder *builder,
					const char *signature);
bool l_dbus_message_builder_leave_variant(
					struct l_dbus_message_builder *builder);

bool l_dbus_message_builder_append_from_iter(
					struct l_dbus_message_builder *builder,
					struct l_dbus_message_iter *from);

bool l_dbus_message_builder_append_from_valist(
					struct l_dbus_message_builder *builder,
					const char *signature, va_list args);

struct l_dbus_message *l_dbus_message_builder_finalize(
					struct l_dbus_message_builder *builder);

bool l_dbus_register_interface(struct l_dbus *dbus, const char *interface,
				l_dbus_interface_setup_func_t setup_func,
				l_dbus_destroy_func_t destroy,
				bool handle_old_style_properties);
bool l_dbus_unregister_interface(struct l_dbus *dbus, const char *interface);

bool l_dbus_register_object(struct l_dbus *dbus, const char *path,
				void *user_data, l_dbus_destroy_func_t destroy,
				...);
bool l_dbus_unregister_object(struct l_dbus *dbus, const char *object);

bool l_dbus_object_add_interface(struct l_dbus *dbus, const char *object,
					const char *interface, void *user_data);
bool l_dbus_object_remove_interface(struct l_dbus *dbus, const char *object,
					const char *interface);
void *l_dbus_object_get_data(struct l_dbus *dbus, const char *object,
				const char *interface);

bool l_dbus_object_manager_enable(struct l_dbus *dbus, const char *root);

unsigned int l_dbus_add_service_watch(struct l_dbus *dbus,
					const char *name,
					l_dbus_watch_func_t connect_func,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy);

unsigned int l_dbus_add_disconnect_watch(struct l_dbus *dbus,
					const char *name,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy);
bool l_dbus_remove_watch(struct l_dbus *dbus, unsigned int id);

unsigned int l_dbus_add_signal_watch(struct l_dbus *dbus,
					const char *sender,
					const char *path,
					const char *interface,
					const char *member, ...);
bool l_dbus_remove_signal_watch(struct l_dbus *dbus, unsigned int id);

uint32_t l_dbus_name_acquire(struct l_dbus *dbus, const char *name,
				bool allow_replacement, bool replace_existing,
				bool queue, l_dbus_name_acquire_func_t callback,
				void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_DBUS_H */
