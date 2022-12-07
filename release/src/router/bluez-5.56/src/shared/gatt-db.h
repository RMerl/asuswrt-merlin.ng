/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct gatt_db;
struct gatt_db_attribute;

struct gatt_db *gatt_db_new(void);

struct gatt_db *gatt_db_ref(struct gatt_db *db);
void gatt_db_unref(struct gatt_db *db);

bool gatt_db_isempty(struct gatt_db *db);

struct gatt_db_attribute *gatt_db_add_service(struct gatt_db *db,
						const bt_uuid_t *uuid,
						bool primary,
						uint16_t num_handles);

bool gatt_db_remove_service(struct gatt_db *db,
					struct gatt_db_attribute *attrib);
bool gatt_db_clear(struct gatt_db *db);
bool gatt_db_clear_range(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle);
bool gatt_db_hash_support(struct gatt_db *db);
uint8_t *gatt_db_get_hash(struct gatt_db *db);

struct gatt_db_attribute *gatt_db_insert_service(struct gatt_db *db,
							uint16_t handle,
							const bt_uuid_t *uuid,
							bool primary,
							uint16_t num_handles);

typedef void (*gatt_db_read_t) (struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

typedef void (*gatt_db_write_t) (struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

struct gatt_db_attribute *
gatt_db_service_add_characteristic(struct gatt_db_attribute *attrib,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);
struct gatt_db_attribute *
gatt_db_service_insert_characteristic(struct gatt_db_attribute *attrib,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);

struct gatt_db_attribute *
gatt_db_insert_characteristic(struct gatt_db *db,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);

struct gatt_db_attribute *
gatt_db_insert_descriptor(struct gatt_db *db,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);

struct gatt_db_attribute *
gatt_db_service_add_descriptor(struct gatt_db_attribute *attrib,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);
struct gatt_db_attribute *
gatt_db_service_insert_descriptor(struct gatt_db_attribute *attrib,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data);

struct gatt_db_attribute *
gatt_db_insert_included(struct gatt_db *db, uint16_t handle,
			struct gatt_db_attribute *include);

struct gatt_db_attribute *
gatt_db_service_add_included(struct gatt_db_attribute *attrib,
					struct gatt_db_attribute *include);
struct gatt_db_attribute *
gatt_db_service_insert_included(struct gatt_db_attribute *attrib,
				uint16_t handle,
				struct gatt_db_attribute *include);

bool gatt_db_service_set_active(struct gatt_db_attribute *attrib, bool active);
bool gatt_db_service_get_active(struct gatt_db_attribute *attrib);

bool gatt_db_service_set_claimed(struct gatt_db_attribute *attrib,
								bool claimed);
bool gatt_db_service_get_claimed(struct gatt_db_attribute *attrib);

typedef void (*gatt_db_attribute_cb_t)(struct gatt_db_attribute *attrib,
							void *user_data);

void gatt_db_read_by_group_type(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle,
							const bt_uuid_t type,
							struct queue *queue);

unsigned int gatt_db_find_by_type(struct gatt_db *db, uint16_t start_handle,
						uint16_t end_handle,
						const bt_uuid_t *type,
						gatt_db_attribute_cb_t func,
						void *user_data);

unsigned int gatt_db_find_by_type_value(struct gatt_db *db,
						uint16_t start_handle,
						uint16_t end_handle,
						const bt_uuid_t *type,
						const void *value,
						size_t value_len,
						gatt_db_attribute_cb_t func,
						void *user_data);

void gatt_db_read_by_type(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle,
							const bt_uuid_t type,
							struct queue *queue);

void gatt_db_find_information(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle,
							struct queue *queue);


void gatt_db_foreach_service(struct gatt_db *db, const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data);
void gatt_db_foreach_in_range(struct gatt_db *db, const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data,
						uint16_t start_handle,
						uint16_t end_handle);

void gatt_db_foreach_service_in_range(struct gatt_db *db,
						const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data,
						uint16_t start_handle,
						uint16_t end_handle);

void gatt_db_service_foreach(struct gatt_db_attribute *attrib,
						const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data);
void gatt_db_service_foreach_char(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data);
void gatt_db_service_foreach_desc(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data);
void gatt_db_service_foreach_incl(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data);

typedef void (*gatt_db_destroy_func_t)(void *user_data);

unsigned int gatt_db_register(struct gatt_db *db,
					gatt_db_attribute_cb_t service_added,
					gatt_db_attribute_cb_t service_removed,
					void *user_data,
					gatt_db_destroy_func_t destroy);
bool gatt_db_unregister(struct gatt_db *db, unsigned int id);

typedef uint8_t (*gatt_db_authorize_cb_t)(struct gatt_db_attribute *attrib,
					uint8_t opcode, struct bt_att *att,
					void *user_data);
bool gatt_db_set_authorize(struct gatt_db *db, gatt_db_authorize_cb_t cb,
					void *user_data);

struct gatt_db_attribute *gatt_db_get_service(struct gatt_db *db,
							uint16_t handle);

struct gatt_db_attribute *gatt_db_get_attribute(struct gatt_db *db,
							uint16_t handle);

struct gatt_db_attribute *gatt_db_get_service_with_uuid(struct gatt_db *db,
							const bt_uuid_t *uuid);

const bt_uuid_t *gatt_db_attribute_get_type(
					const struct gatt_db_attribute *attrib);

uint16_t gatt_db_attribute_get_handle(const struct gatt_db_attribute *attrib);

bool gatt_db_attribute_get_service_uuid(const struct gatt_db_attribute *attrib,
							bt_uuid_t *uuid);

bool gatt_db_attribute_get_service_handles(
					const struct gatt_db_attribute *attrib,
					uint16_t *start_handle,
					uint16_t *end_handle);

bool gatt_db_attribute_get_service_data(const struct gatt_db_attribute *attrib,
							uint16_t *start_handle,
							uint16_t *end_handle,
							bool *primary,
							bt_uuid_t *uuid);

bool gatt_db_attribute_get_char_data(const struct gatt_db_attribute *attrib,
							uint16_t *handle,
							uint16_t *value_handle,
							uint8_t *properties,
							uint16_t *ext_prop,
							bt_uuid_t *uuid);

bool gatt_db_attribute_get_incl_data(const struct gatt_db_attribute *attrib,
							uint16_t *handle,
							uint16_t *start_handle,
							uint16_t *end_handle);

uint32_t
gatt_db_attribute_get_permissions(const struct gatt_db_attribute *attrib);

bool gatt_db_attribute_set_fixed_length(struct gatt_db_attribute *attrib,
						uint16_t len);

typedef void (*gatt_db_attribute_read_t) (struct gatt_db_attribute *attrib,
						int err, const uint8_t *value,
						size_t length, void *user_data);

bool gatt_db_attribute_read(struct gatt_db_attribute *attrib, uint16_t offset,
				uint8_t opcode, struct bt_att *att,
				gatt_db_attribute_read_t func, void *user_data);

bool gatt_db_attribute_read_result(struct gatt_db_attribute *attrib,
					unsigned int id, int err,
					const uint8_t *value, size_t length);

typedef void (*gatt_db_attribute_write_t) (struct gatt_db_attribute *attrib,
						int err, void *user_data);

bool gatt_db_attribute_write(struct gatt_db_attribute *attrib, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					gatt_db_attribute_write_t func,
					void *user_data);

bool gatt_db_attribute_write_result(struct gatt_db_attribute *attrib,
						unsigned int id, int err);

bool gatt_db_attribute_reset(struct gatt_db_attribute *attrib);

void *gatt_db_attribute_get_user_data(struct gatt_db_attribute *attrib);

unsigned int gatt_db_attribute_register(struct gatt_db_attribute *attrib,
					gatt_db_attribute_cb_t removed,
					void *user_data,
					gatt_db_destroy_func_t destroy);

bool gatt_db_attribute_unregister(struct gatt_db_attribute *attrib,
						unsigned int id);
