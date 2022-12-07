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

#include <endian.h>

enum dbus_message_type {
	DBUS_MESSAGE_TYPE_METHOD_CALL	= 1,
	DBUS_MESSAGE_TYPE_METHOD_RETURN	= 2,
	DBUS_MESSAGE_TYPE_ERROR		= 3,
	DBUS_MESSAGE_TYPE_SIGNAL	= 4,
};

enum dbus_container_type {
	DBUS_CONTAINER_TYPE_ARRAY	= 'a',
	DBUS_CONTAINER_TYPE_STRUCT	= 'r',
	DBUS_CONTAINER_TYPE_VARIANT	= 'v',
	DBUS_CONTAINER_TYPE_DICT_ENTRY	= 'e',
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define DBUS_NATIVE_ENDIAN 'l'
#elif __BYTE_ORDER == __BIG_ENDIAN
#define DBUS_NATIVE_ENDIAN 'B'
#else
#error "Unknown byte order"
#endif

struct dbus_header {
	uint8_t  endian;
	uint8_t  message_type;
	uint8_t  flags;
	uint8_t  version;

	union {
		struct {
			uint32_t body_length;
			uint32_t serial;
			uint32_t field_length;
		} __attribute__ ((packed)) dbus1;
	};
} __attribute__ ((packed));
#define DBUS_HEADER_SIZE 16

struct dbus_builder;
struct l_string;
struct l_dbus_interface;
struct _dbus_method;
struct _dbus_signal;
struct _dbus_property;
struct l_dbus_message_iter;
struct l_dbus_message;
struct l_dbus;
struct _dbus_filter;
struct _dbus_filter_condition;
struct _dbus_filter_ops;

void _dbus1_iter_init(struct l_dbus_message_iter *iter,
			struct l_dbus_message *message,
			const char *sig_start, const char *sig_end,
			const void *data, size_t len);
bool _dbus1_iter_next_entry_basic(struct l_dbus_message_iter *iter, char type,
					void *out);
bool _dbus1_iter_get_fixed_array(struct l_dbus_message_iter *iter,
					void *out, uint32_t *n_elem);
bool _dbus1_iter_enter_struct(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *structure);
bool _dbus1_iter_enter_variant(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *variant);
bool _dbus1_iter_enter_array(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *array);
bool _dbus1_iter_skip_entry(struct l_dbus_message_iter *iter);

struct dbus_builder *_dbus1_builder_new(void *body, size_t body_size);
void _dbus1_builder_free(struct dbus_builder *builder);
bool _dbus1_builder_append_basic(struct dbus_builder *builder,
					char type, const void *value);
bool _dbus1_builder_enter_struct(struct dbus_builder *builder,
					const char *signature);
bool _dbus1_builder_leave_struct(struct dbus_builder *builder);
bool _dbus1_builder_enter_dict(struct dbus_builder *builder,
					const char *signature);
bool _dbus1_builder_leave_dict(struct dbus_builder *builder);
bool _dbus1_builder_enter_variant(struct dbus_builder *builder,
					const char *signature);
bool _dbus1_builder_leave_variant(struct dbus_builder *builder);
bool _dbus1_builder_enter_array(struct dbus_builder *builder,
					const char *signature);
bool _dbus1_builder_leave_array(struct dbus_builder *builder);
char *_dbus1_builder_finish(struct dbus_builder *builder,
				void **body, size_t *body_size);
bool _dbus1_builder_mark(struct dbus_builder *builder);
bool _dbus1_builder_rewind(struct dbus_builder *builder);

void *_dbus_message_get_body(struct l_dbus_message *msg, size_t *out_size);
void *_dbus_message_get_header(struct l_dbus_message *msg, size_t *out_size);
void *_dbus_message_get_footer(struct l_dbus_message *msg, size_t *out_size);
int *_dbus_message_get_fds(struct l_dbus_message *msg, uint32_t *num_fds);
void _dbus_message_set_serial(struct l_dbus_message *msg, uint32_t serial);
uint32_t _dbus_message_get_serial(struct l_dbus_message *msg);
uint32_t _dbus_message_get_reply_serial(struct l_dbus_message *message);

void _dbus_message_set_sender(struct l_dbus_message *message,
					const char *sender);
void _dbus_message_set_destination(struct l_dbus_message *message,
					const char *destination);

enum dbus_message_type _dbus_message_get_type(struct l_dbus_message *message);
const char * _dbus_message_get_type_as_string(struct l_dbus_message *message);
uint8_t _dbus_message_get_version(struct l_dbus_message *message);
uint8_t _dbus_message_get_endian(struct l_dbus_message *message);
const char *_dbus_message_get_nth_string_argument(
					struct l_dbus_message *message, int n);

struct l_dbus_message *_dbus_message_new_method_call(uint8_t version,
							const char *destination,
							const char *path,
							const char *interface,
							const char *method);
struct l_dbus_message *_dbus_message_new_signal(uint8_t version,
						const char *path,
						const char *interface,
						const char *name);
struct l_dbus_message *_dbus_message_new_error(uint8_t version,
						uint32_t reply_serial,
						const char *destination,
						const char *name,
						const char *error);

struct l_dbus_message *dbus_message_from_blob(const void *data, size_t size,
						int fds[], uint32_t num_fds);
struct l_dbus_message *dbus_message_build(void *header, size_t header_size,
						void *body, size_t body_size,
						int fds[], uint32_t num_fds);
bool dbus_message_compare(struct l_dbus_message *message,
					const void *data, size_t size);

bool _dbus_message_builder_mark(struct l_dbus_message_builder *builder);
bool _dbus_message_builder_rewind(struct l_dbus_message_builder *builder);

unsigned int _dbus_message_unix_fds_from_header(const void *data, size_t size);

const char *_dbus_signature_end(const char *signature);

bool _dbus_valid_object_path(const char *path);
bool _dbus_valid_signature(const char *sig);
int _dbus_num_children(const char *sig);
bool _dbus_valid_interface(const char *interface);
bool _dbus_valid_method(const char *method);
bool _dbus_parse_unique_name(const char *name, uint64_t *out_id);
bool _dbus_valid_bus_name(const char *bus_name);

bool _dbus1_header_is_valid(void *data, size_t size);

void _dbus_method_introspection(struct _dbus_method *info,
					struct l_string *buf);
void _dbus_signal_introspection(struct _dbus_signal *info,
					struct l_string *buf);
void _dbus_property_introspection(struct _dbus_property *info,
						struct l_string *buf);
void _dbus_interface_introspection(struct l_dbus_interface *interface,
						struct l_string *buf);

struct l_dbus_interface *_dbus_interface_new(const char *name);
void _dbus_interface_free(struct l_dbus_interface *interface);

struct _dbus_method *_dbus_interface_find_method(struct l_dbus_interface *i,
							const char *method);
struct _dbus_signal *_dbus_interface_find_signal(struct l_dbus_interface *i,
							const char *signal);
struct _dbus_property *_dbus_interface_find_property(struct l_dbus_interface *i,
						const char *property);

struct _dbus_object_tree *_dbus_object_tree_new(void);
void _dbus_object_tree_free(struct _dbus_object_tree *tree);

struct object_node *_dbus_object_tree_makepath(struct _dbus_object_tree *tree,
						const char *path);
struct object_node *_dbus_object_tree_lookup(struct _dbus_object_tree *tree,
						const char *path);
void _dbus_object_tree_prune_node(struct object_node *node);

struct object_node *_dbus_object_tree_new_object(struct _dbus_object_tree *tree,
						const char *path,
						void *user_data,
						void (*destroy) (void *));
bool _dbus_object_tree_object_destroy(struct _dbus_object_tree *tree,
						const char *path);

bool _dbus_object_tree_register_interface(struct _dbus_object_tree *tree,
				const char *interface,
				void (*setup_func)(struct l_dbus_interface *),
				void (*destroy) (void *),
				bool old_style_properties);
bool _dbus_object_tree_unregister_interface(struct _dbus_object_tree *tree,
						const char *interface);

bool _dbus_object_tree_add_interface(struct _dbus_object_tree *tree,
					const char *path, const char *interface,
					void *user_data);
bool _dbus_object_tree_remove_interface(struct _dbus_object_tree *tree,
					const char *path,
					const char *interface);
void *_dbus_object_tree_get_interface_data(struct _dbus_object_tree *tree,
						const char *path,
						const char *interface);

void _dbus_object_tree_introspect(struct _dbus_object_tree *tree,
					const char *path, struct l_string *buf);
bool _dbus_object_tree_dispatch(struct _dbus_object_tree *tree,
					struct l_dbus *dbus,
					struct l_dbus_message *message);
struct l_dbus_message *_dbus_object_tree_get_objects(
					struct _dbus_object_tree *tree,
					struct l_dbus *dbus,
					const char *path,
					struct l_dbus_message *message);

bool _dbus_object_tree_property_changed(struct l_dbus *dbus,
					const char *path,
					const char *interface_name,
					const char *property_name);

void _dbus_object_tree_signals_flush(struct l_dbus *dbus, const char *path);

typedef void (*_dbus_name_owner_change_func_t)(const char *name,
						uint64_t old_owner,
						uint64_t new_owner,
						void *user_data);

uint8_t _dbus_get_version(struct l_dbus *dbus);
int _dbus_get_fd(struct l_dbus *dbus);
struct _dbus_object_tree *_dbus_get_tree(struct l_dbus *dbus);

struct _dbus_name_ops {
	bool (*get_name_owner)(struct l_dbus *bus, const char *name);
};

struct _dbus_name_cache;

struct _dbus_name_cache *_dbus_name_cache_new(struct l_dbus *bus,
					const struct _dbus_name_ops *driver);
void _dbus_name_cache_free(struct _dbus_name_cache *cache);

bool _dbus_name_cache_add(struct _dbus_name_cache *cache, const char *name);
bool _dbus_name_cache_remove(struct _dbus_name_cache *cache, const char *name);
const char *_dbus_name_cache_lookup(struct _dbus_name_cache *cache,
					const char *name);

void _dbus_name_cache_notify(struct _dbus_name_cache *cache,
				const char *name, const char *owner);

unsigned int _dbus_name_cache_add_watch(struct _dbus_name_cache *cache,
					const char *name,
					l_dbus_watch_func_t connect_func,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy);
bool _dbus_name_cache_remove_watch(struct _dbus_name_cache *cache,
					unsigned int id);

struct _dbus_filter_condition {
	enum l_dbus_match_type type;
	const char *value;
};

struct _dbus_filter_ops {
	bool skip_register;
	bool (*add_match)(struct l_dbus *bus, unsigned int id,
				const struct _dbus_filter_condition *rule,
				int rule_len);
	bool (*remove_match)(struct l_dbus *bus, unsigned int id);
};

struct _dbus_filter *_dbus_filter_new(struct l_dbus *dbus,
					const struct _dbus_filter_ops *driver,
					struct _dbus_name_cache *name_cache);
void _dbus_filter_free(struct _dbus_filter *filter);

unsigned int _dbus_filter_add_rule(struct _dbus_filter *filter,
				const struct _dbus_filter_condition *rule,
				int rule_len,
				l_dbus_message_func_t signal_func,
				void *user_data);
bool _dbus_filter_remove_rule(struct _dbus_filter *filter, unsigned int id);

char *_dbus_filter_rule_to_str(const struct _dbus_filter_condition *rule,
				int rule_len);

void _dbus_filter_dispatch(struct l_dbus_message *message, void *user_data);
