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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/timeout.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define MAX_CHAR_DECL_VALUE_LEN 19
#define MAX_INCLUDED_VALUE_LEN 6
#define ATTRIBUTE_TIMEOUT 5000

static const bt_uuid_t primary_service_uuid = { .type = BT_UUID16,
					.value.u16 = GATT_PRIM_SVC_UUID };
static const bt_uuid_t secondary_service_uuid = { .type = BT_UUID16,
					.value.u16 = GATT_SND_SVC_UUID };
static const bt_uuid_t characteristic_uuid = { .type = BT_UUID16,
					.value.u16 = GATT_CHARAC_UUID };
static const bt_uuid_t included_service_uuid = { .type = BT_UUID16,
					.value.u16 = GATT_INCLUDE_UUID };
static const bt_uuid_t ext_desc_uuid = { .type = BT_UUID16,
				.value.u16 = GATT_CHARAC_EXT_PROPER_UUID };

struct gatt_db {
	int ref_count;
	uint16_t next_handle;
	struct queue *services;

	struct queue *notify_list;
	unsigned int next_notify_id;
};

struct notify {
	unsigned int id;
	gatt_db_attribute_cb_t service_added;
	gatt_db_attribute_cb_t service_removed;
	gatt_db_destroy_func_t destroy;
	void *user_data;
};

struct pending_read {
	struct gatt_db_attribute *attrib;
	unsigned int id;
	unsigned int timeout_id;
	gatt_db_attribute_read_t func;
	void *user_data;
};

struct pending_write {
	struct gatt_db_attribute *attrib;
	unsigned int id;
	unsigned int timeout_id;
	gatt_db_attribute_write_t func;
	void *user_data;
};

struct gatt_db_attribute {
	struct gatt_db_service *service;
	uint16_t handle;
	bt_uuid_t uuid;
	uint32_t permissions;
	uint16_t value_len;
	uint8_t *value;

	gatt_db_read_t read_func;
	gatt_db_write_t write_func;
	void *user_data;

	unsigned int read_id;
	struct queue *pending_reads;

	unsigned int write_id;
	struct queue *pending_writes;
};

struct gatt_db_service {
	struct gatt_db *db;
	bool active;
	bool claimed;
	uint16_t num_handles;
	struct gatt_db_attribute **attributes;
};

static void pending_read_result(struct pending_read *p, int err,
					const uint8_t *data, size_t length)
{
	if (p->timeout_id > 0)
		timeout_remove(p->timeout_id);

	p->func(p->attrib, err, data, length, p->user_data);

	free(p);
}

static void pending_read_free(void *data)
{
	struct pending_read *p = data;

	pending_read_result(p, -ECANCELED, NULL, 0);
}

static void pending_write_result(struct pending_write *p, int err)
{
	if (p->timeout_id > 0)
		timeout_remove(p->timeout_id);

	p->func(p->attrib, err, p->user_data);

	free(p);
}

static void pending_write_free(void *data)
{
	struct pending_write *p = data;

	pending_write_result(p, -ECANCELED);
}

static void attribute_destroy(struct gatt_db_attribute *attribute)
{
	/* Attribute was not initialized by user */
	if (!attribute)
		return;

	queue_destroy(attribute->pending_reads, pending_read_free);
	queue_destroy(attribute->pending_writes, pending_write_free);

	free(attribute->value);
	free(attribute);
}

static struct gatt_db_attribute *new_attribute(struct gatt_db_service *service,
							uint16_t handle,
							const bt_uuid_t *type,
							const uint8_t *val,
							uint16_t len)
{
	struct gatt_db_attribute *attribute;

	attribute = new0(struct gatt_db_attribute, 1);

	attribute->service = service;
	attribute->handle = handle;
	attribute->uuid = *type;
	attribute->value_len = len;
	if (len) {
		attribute->value = malloc0(len);
		if (!attribute->value)
			goto failed;

		memcpy(attribute->value, val, len);
	}

	attribute->pending_reads = queue_new();
	attribute->pending_writes = queue_new();

	return attribute;

failed:
	attribute_destroy(attribute);
	return NULL;
}

struct gatt_db *gatt_db_ref(struct gatt_db *db)
{
	if (!db)
		return NULL;

	__sync_fetch_and_add(&db->ref_count, 1);

	return db;
}

struct gatt_db *gatt_db_new(void)
{
	struct gatt_db *db;

	db = new0(struct gatt_db, 1);
	db->services = queue_new();
	db->notify_list = queue_new();
	db->next_handle = 0x0001;

	return gatt_db_ref(db);
}

static void notify_destroy(void *data)
{
	struct notify *notify = data;

	if (notify->destroy)
		notify->destroy(notify->user_data);

	free(notify);
}

static bool match_notify_id(const void *a, const void *b)
{
	const struct notify *notify = a;
	unsigned int id = PTR_TO_UINT(b);

	return notify->id == id;
}

struct notify_data {
	struct gatt_db_attribute *attr;
	bool added;
};

static void handle_notify(void *data, void *user_data)
{
	struct notify *notify = data;
	struct notify_data *notify_data = user_data;

	if (notify_data->added)
		notify->service_added(notify_data->attr, notify->user_data);
	else
		notify->service_removed(notify_data->attr, notify->user_data);
}

static void notify_service_changed(struct gatt_db *db,
						struct gatt_db_service *service,
						bool added)
{
	struct notify_data data;

	if (queue_isempty(db->notify_list))
		return;

	data.attr = service->attributes[0];
	data.added = added;

	gatt_db_ref(db);

	queue_foreach(db->notify_list, handle_notify, &data);

	gatt_db_unref(db);
}

static void gatt_db_service_destroy(void *data)
{
	struct gatt_db_service *service = data;
	int i;

	if (service->active)
		notify_service_changed(service->db, service, false);

	for (i = 0; i < service->num_handles; i++)
		attribute_destroy(service->attributes[i]);

	free(service->attributes);
	free(service);
}

static void gatt_db_destroy(struct gatt_db *db)
{
	if (!db)
		return;

	/*
	 * Clear the notify list before clearing the services to prevent the
	 * latter from sending service_removed events.
	 */
	queue_destroy(db->notify_list, notify_destroy);
	db->notify_list = NULL;

	queue_destroy(db->services, gatt_db_service_destroy);
	free(db);
}

void gatt_db_unref(struct gatt_db *db)
{
	if (!db)
		return;

	if (__sync_sub_and_fetch(&db->ref_count, 1))
		return;

	gatt_db_destroy(db);
}

bool gatt_db_isempty(struct gatt_db *db)
{
	if (!db)
		return true;

	return queue_isempty(db->services);
}

static int uuid_to_le(const bt_uuid_t *uuid, uint8_t *dst)
{
	bt_uuid_t uuid128;

	if (uuid->type == BT_UUID16) {
		put_le16(uuid->value.u16, dst);
		return bt_uuid_len(uuid);
	}

	bt_uuid_to_uuid128(uuid, &uuid128);
	bswap_128(&uuid128.value.u128, dst);
	return bt_uuid_len(&uuid128);
}

static bool le_to_uuid(const uint8_t *src, size_t len, bt_uuid_t *uuid)
{
	uint128_t u128;

	if (len == 2) {
		bt_uuid16_create(uuid, get_le16(src));
		return true;
	}

	if (len == 4) {
		bt_uuid32_create(uuid, get_le32(src));
		return true;
	}

	if (len != 16)
		return false;

	bswap_128(src, &u128);
	bt_uuid128_create(uuid, u128);

	return true;
}

static struct gatt_db_service *gatt_db_service_create(const bt_uuid_t *uuid,
							uint16_t handle,
							bool primary,
							uint16_t num_handles)
{
	struct gatt_db_service *service;
	const bt_uuid_t *type;
	uint8_t value[16];
	uint16_t len;

	if (num_handles < 1)
		return NULL;

	service = new0(struct gatt_db_service, 1);
	service->attributes = new0(struct gatt_db_attribute *, num_handles);

	if (primary)
		type = &primary_service_uuid;
	else
		type = &secondary_service_uuid;

	len = uuid_to_le(uuid, value);

	service->attributes[0] = new_attribute(service, handle, type, value,
									len);
	if (!service->attributes[0]) {
		gatt_db_service_destroy(service);
		return NULL;
	}

	return service;
}


bool gatt_db_remove_service(struct gatt_db *db,
					struct gatt_db_attribute *attrib)
{
	struct gatt_db_service *service;

	if (!db || !attrib)
		return false;

	service = attrib->service;

	queue_remove(db->services, service);

	gatt_db_service_destroy(service);

	return true;
}

bool gatt_db_clear(struct gatt_db *db)
{
	if (!db)
		return false;

	queue_remove_all(db->services, NULL, NULL, gatt_db_service_destroy);

	db->next_handle = 0;

	return true;
}

static void gatt_db_service_get_handles(const struct gatt_db_service *service,
							uint16_t *start_handle,
							uint16_t *end_handle)
{
	if (start_handle)
		*start_handle = service->attributes[0]->handle;

	if (end_handle)
		*end_handle = service->attributes[0]->handle +
						service->num_handles - 1;
}

struct clear_range {
	uint16_t start, end;
};

static bool match_range(const void *a, const void *b)
{
	const struct gatt_db_service *service = a;
	const struct clear_range *range = b;
	uint16_t svc_start, svc_end;

	gatt_db_service_get_handles(service, &svc_start, &svc_end);

	return svc_start <= range->end && svc_end >= range->start;
}

bool gatt_db_clear_range(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle)
{
	struct clear_range range;

	if (!db || start_handle > end_handle)
		return false;

	range.start = start_handle;
	range.end = end_handle;

	queue_remove_all(db->services, match_range, &range,
						gatt_db_service_destroy);

	return true;
}

static struct gatt_db_service *find_insert_loc(struct gatt_db *db,
						uint16_t start, uint16_t end,
						struct gatt_db_service **after)
{
	const struct queue_entry *services_entry;
	struct gatt_db_service *service;
	uint16_t cur_start, cur_end;

	*after = NULL;

	services_entry = queue_get_entries(db->services);

	while (services_entry) {
		service = services_entry->data;

		gatt_db_service_get_handles(service, &cur_start, &cur_end);

		if (start >= cur_start && start <= cur_end)
			return service;

		if (end >= cur_start && end <= cur_end)
			return service;

		if (end < cur_start)
			return NULL;

		*after = service;
		services_entry = services_entry->next;
	}

	return NULL;
}

struct gatt_db_attribute *gatt_db_insert_service(struct gatt_db *db,
							uint16_t handle,
							const bt_uuid_t *uuid,
							bool primary,
							uint16_t num_handles)
{
	struct gatt_db_service *service, *after;

	after = NULL;

	if (!db || handle < 1)
		return NULL;

	if (num_handles < 1 || (handle + num_handles - 1) > UINT16_MAX)
		return NULL;

	service = find_insert_loc(db, handle, handle + num_handles - 1, &after);
	if (service) {
		const bt_uuid_t *type;
		bt_uuid_t value;

		if (primary)
			type = &primary_service_uuid;
		else
			type = &secondary_service_uuid;

		gatt_db_attribute_get_service_uuid(service->attributes[0],
									&value);

		/* Check if service match */
		if (!bt_uuid_cmp(&service->attributes[0]->uuid, type) &&
				!bt_uuid_cmp(&value, uuid) &&
				service->num_handles == num_handles &&
				service->attributes[0]->handle == handle)
			return service->attributes[0];

		return NULL;
	}

	service = gatt_db_service_create(uuid, handle, primary, num_handles);

	if (!service)
		return NULL;

	if (after) {
		if (!queue_push_after(db->services, after, service))
			goto fail;
	} else if (!queue_push_head(db->services, service)) {
		goto fail;
	}

	service->db = db;
	service->attributes[0]->handle = handle;
	service->num_handles = num_handles;

	/* Fast-forward next_handle if the new service was added to the end */
	db->next_handle = MAX(handle + num_handles, db->next_handle);

	return service->attributes[0];

fail:
	gatt_db_service_destroy(service);
	return NULL;
}

struct gatt_db_attribute *gatt_db_add_service(struct gatt_db *db,
						const bt_uuid_t *uuid,
						bool primary,
						uint16_t num_handles)
{
	return gatt_db_insert_service(db, db->next_handle, uuid, primary,
								num_handles);
}

unsigned int gatt_db_register(struct gatt_db *db,
					gatt_db_attribute_cb_t service_added,
					gatt_db_attribute_cb_t service_removed,
					void *user_data,
					gatt_db_destroy_func_t destroy)
{
	struct notify *notify;

	if (!db || !(service_added || service_removed))
		return 0;

	notify = new0(struct notify, 1);
	notify->service_added = service_added;
	notify->service_removed = service_removed;
	notify->destroy = destroy;
	notify->user_data = user_data;

	if (db->next_notify_id < 1)
		db->next_notify_id = 1;

	notify->id = db->next_notify_id++;

	if (!queue_push_tail(db->notify_list, notify)) {
		free(notify);
		return 0;
	}

	return notify->id;
}

bool gatt_db_unregister(struct gatt_db *db, unsigned int id)
{
	struct notify *notify;

	if (!db || !id)
		return false;

	notify = queue_find(db->notify_list, match_notify_id, UINT_TO_PTR(id));
	if (!notify)
		return false;

	queue_remove(db->notify_list, notify);
	notify_destroy(notify);

	return true;
}

static uint16_t get_attribute_index(struct gatt_db_service *service,
							int end_offset)
{
	int i = 0;

	/* Here we look for first free attribute index with given offset */
	while (i < (service->num_handles - end_offset) &&
						service->attributes[i])
		i++;

	return i == (service->num_handles - end_offset) ? 0 : i;
}

static uint16_t get_handle_at_index(struct gatt_db_service *service,
								int index)
{
	return service->attributes[index]->handle;
}

static struct gatt_db_attribute *
attribute_update(struct gatt_db_service *service, int index)
{
	uint16_t previous_handle;

	/* We call this function with index > 0, because index 0 is reserved
	 * for service declaration, and is set in add_service()
	 */
	previous_handle = service->attributes[index - 1]->handle;
	service->attributes[index]->handle = previous_handle + 1;

	return service->attributes[index];
}

static void set_attribute_data(struct gatt_db_attribute *attribute,
						gatt_db_read_t read_func,
						gatt_db_write_t write_func,
						uint32_t permissions,
						void *user_data)
{
	attribute->permissions = permissions;
	attribute->read_func = read_func;
	attribute->write_func = write_func;
	attribute->user_data = user_data;
}

static struct gatt_db_attribute *
service_insert_characteristic(struct gatt_db_service *service,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	uint8_t value[MAX_CHAR_DECL_VALUE_LEN];
	uint16_t len = 0;
	int i;

	/* Check if handle is in within service range */
	if (handle && handle <= service->attributes[0]->handle)
		return NULL;

	/*
	 * It is not possible to allocate last handle for a Characteristic
	 * since it would not have space for its value:
	 * 3.3.2 Characteristic Value Declaration
	 * The Characteristic Value declaration contains the value of the
	 * characteristic. It is the first Attribute after the characteristic
	 * declaration. All characteristic definitions shall have a
	 * Characteristic Value declaration.
	 */
	if (handle == UINT16_MAX)
		return NULL;

	i = get_attribute_index(service, 1);
	if (!i)
		return NULL;

	if (!handle)
		handle = get_handle_at_index(service, i - 1) + 2;

	value[0] = properties;
	len += sizeof(properties);

	/* We set handle of characteristic value, which will be added next */
	put_le16(handle, &value[1]);
	len += sizeof(uint16_t);
	len += uuid_to_le(uuid, &value[3]);

	service->attributes[i] = new_attribute(service, handle - 1,
							&characteristic_uuid,
							value, len);
	if (!service->attributes[i])
		return NULL;

	i++;

	service->attributes[i] = new_attribute(service, handle, uuid, NULL, 0);
	if (!service->attributes[i]) {
		free(service->attributes[i - 1]);
		return NULL;
	}

	set_attribute_data(service->attributes[i], read_func, write_func,
							permissions, user_data);

	return service->attributes[i];
}

struct gatt_db_attribute *
gatt_db_service_insert_characteristic(struct gatt_db_attribute *attrib,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	if (!attrib || !handle)
		return NULL;

	return service_insert_characteristic(attrib->service, handle, uuid,
						permissions, properties,
						read_func, write_func,
						user_data);
}

struct gatt_db_attribute *
gatt_db_service_add_characteristic(struct gatt_db_attribute *attrib,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					uint8_t properties,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	if (!attrib)
		return NULL;

	return service_insert_characteristic(attrib->service, 0, uuid,
						permissions, properties,
						read_func, write_func,
						user_data);
}

static struct gatt_db_attribute *
service_insert_descriptor(struct gatt_db_service *service,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	int i;

	i = get_attribute_index(service, 0);
	if (!i)
		return NULL;

	/* Check if handle is in within service range */
	if (handle && handle <= service->attributes[0]->handle)
		return NULL;

	if (!handle)
		handle = get_handle_at_index(service, i - 1) + 1;

	service->attributes[i] = new_attribute(service, handle, uuid, NULL, 0);
	if (!service->attributes[i])
		return NULL;

	set_attribute_data(service->attributes[i], read_func, write_func,
							permissions, user_data);

	return service->attributes[i];
}

struct gatt_db_attribute *
gatt_db_service_insert_descriptor(struct gatt_db_attribute *attrib,
					uint16_t handle,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	if (!attrib || !handle)
		return NULL;

	return service_insert_descriptor(attrib->service, handle, uuid,
					permissions, read_func, write_func,
					user_data);
}

struct gatt_db_attribute *
gatt_db_service_add_descriptor(struct gatt_db_attribute *attrib,
					const bt_uuid_t *uuid,
					uint32_t permissions,
					gatt_db_read_t read_func,
					gatt_db_write_t write_func,
					void *user_data)
{
	if (!attrib)
		return NULL;

	return service_insert_descriptor(attrib->service, 0, uuid,
					permissions, read_func, write_func,
					user_data);
}

struct gatt_db_attribute *
gatt_db_service_add_included(struct gatt_db_attribute *attrib,
					struct gatt_db_attribute *include)
{
	struct gatt_db_service *service, *included;
	uint8_t value[MAX_INCLUDED_VALUE_LEN];
	uint16_t included_handle, len = 0;
	int index;

	if (!attrib || !include)
		return NULL;

	service = attrib->service;
	included = include->service;

	/* Adjust include to point to the first attribute */
	if (include != included->attributes[0])
		include = included->attributes[0];

	included_handle = include->handle;

	put_le16(included_handle, &value[len]);
	len += sizeof(uint16_t);

	put_le16(included_handle + included->num_handles - 1, &value[len]);
	len += sizeof(uint16_t);

	/* The Service UUID shall only be present when the UUID is a 16-bit
	 * Bluetooth UUID. Vol 2. Part G. 3.2
	 */
	if (include->value_len == sizeof(uint16_t)) {
		memcpy(&value[len], include->value, include->value_len);
		len += include->value_len;
	}

	index = get_attribute_index(service, 0);
	if (!index)
		return NULL;

	service->attributes[index] = new_attribute(service, 0,
							&included_service_uuid,
							value, len);
	if (!service->attributes[index])
		return NULL;

	/* The Attribute Permissions shall be read only and not require
	 * authentication or authorization. Vol 2. Part G. 3.2
	 *
	 * TODO handle permissions
	 */
	set_attribute_data(service->attributes[index], NULL, NULL, 0, NULL);

	return attribute_update(service, index);
}

bool gatt_db_service_set_active(struct gatt_db_attribute *attrib, bool active)
{
	struct gatt_db_service *service;

	if (!attrib)
		return false;

	service = attrib->service;

	if (service->active == active)
		return true;

	service->active = active;

	notify_service_changed(service->db, service, active);

	return true;
}

bool gatt_db_service_get_active(struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return false;

	return attrib->service->active;
}

bool gatt_db_service_set_claimed(struct gatt_db_attribute *attrib,
								bool claimed)
{
	if (!attrib)
		return false;

	attrib->service->claimed = claimed;

	return true;
}

bool gatt_db_service_get_claimed(struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return false;

	return attrib->service->claimed;
}

void gatt_db_read_by_group_type(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle,
							const bt_uuid_t type,
							struct queue *queue)
{
	const struct queue_entry *services_entry;
	struct gatt_db_service *service;
	uint16_t grp_start, grp_end, uuid_size;

	uuid_size = 0;

	services_entry = queue_get_entries(db->services);

	while (services_entry) {
		service = services_entry->data;

		if (!service->active)
			goto next_service;

		if (bt_uuid_cmp(&type, &service->attributes[0]->uuid))
			goto next_service;

		grp_start = service->attributes[0]->handle;
		grp_end = grp_start + service->num_handles - 1;

		if (grp_end < start_handle || grp_start > end_handle)
			goto next_service;

		if (grp_start < start_handle || grp_start > end_handle)
			goto next_service;

		if (!uuid_size)
			uuid_size = service->attributes[0]->value_len;
		else if (uuid_size != service->attributes[0]->value_len)
			return;

		queue_push_tail(queue, service->attributes[0]);

next_service:
		services_entry = services_entry->next;
	}
}

struct find_by_type_value_data {
	bt_uuid_t uuid;
	uint16_t start_handle;
	uint16_t end_handle;
	gatt_db_attribute_cb_t func;
	void *user_data;
	const void *value;
	size_t value_len;
	unsigned int num_of_res;
};

static void find_by_type(void *data, void *user_data)
{
	struct find_by_type_value_data *search_data = user_data;
	struct gatt_db_service *service = data;
	struct gatt_db_attribute *attribute;
	int i;

	if (!service->active)
		return;

	for (i = 0; i < service->num_handles; i++) {
		attribute = service->attributes[i];

		if (!attribute)
			continue;

		if ((attribute->handle < search_data->start_handle) ||
				(attribute->handle > search_data->end_handle))
			continue;

		if (bt_uuid_cmp(&search_data->uuid, &attribute->uuid))
			continue;

		/* TODO: fix for read-callback based attributes */
		if (search_data->value && memcmp(attribute->value,
							search_data->value,
							search_data->value_len))
			continue;

		search_data->num_of_res++;
		search_data->func(attribute, search_data->user_data);
	}
}

unsigned int gatt_db_find_by_type(struct gatt_db *db, uint16_t start_handle,
						uint16_t end_handle,
						const bt_uuid_t *type,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	struct find_by_type_value_data data;

	memset(&data, 0, sizeof(data));

	data.uuid = *type;
	data.start_handle = start_handle;
	data.end_handle = end_handle;
	data.func = func;
	data.user_data = user_data;

	queue_foreach(db->services, find_by_type, &data);

	return data.num_of_res;
}

unsigned int gatt_db_find_by_type_value(struct gatt_db *db,
						uint16_t start_handle,
						uint16_t end_handle,
						const bt_uuid_t *type,
						const void *value,
						size_t value_len,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	struct find_by_type_value_data data;

	data.uuid = *type;
	data.start_handle = start_handle;
	data.end_handle = end_handle;
	data.func = func;
	data.user_data = user_data;
	data.value = value;
	data.value_len = value_len;

	queue_foreach(db->services, find_by_type, &data);

	return data.num_of_res;
}

struct read_by_type_data {
	struct queue *queue;
	bt_uuid_t uuid;
	uint16_t start_handle;
	uint16_t end_handle;
};

static void read_by_type(void *data, void *user_data)
{
	struct read_by_type_data *search_data = user_data;
	struct gatt_db_service *service = data;
	struct gatt_db_attribute *attribute;
	int i;

	if (!service->active)
		return;

	for (i = 0; i < service->num_handles; i++) {
		attribute = service->attributes[i];
		if (!attribute)
			continue;

		if (attribute->handle < search_data->start_handle)
			continue;

		if (attribute->handle > search_data->end_handle)
			return;

		if (bt_uuid_cmp(&search_data->uuid, &attribute->uuid))
			continue;

		queue_push_tail(search_data->queue, attribute);
	}
}

void gatt_db_read_by_type(struct gatt_db *db, uint16_t start_handle,
						uint16_t end_handle,
						const bt_uuid_t type,
						struct queue *queue)
{
	struct read_by_type_data data;
	data.uuid = type;
	data.start_handle = start_handle;
	data.end_handle = end_handle;
	data.queue = queue;

	queue_foreach(db->services, read_by_type, &data);
}


struct find_information_data {
	struct queue *queue;
	uint16_t start_handle;
	uint16_t end_handle;
};

static void find_information(void *data, void *user_data)
{
	struct find_information_data *search_data = user_data;
	struct gatt_db_service *service = data;
	struct gatt_db_attribute *attribute;
	int i;

	if (!service->active)
		return;

	/* Check if service is in range */
	if ((service->attributes[0]->handle + service->num_handles - 1) <
						search_data->start_handle)
		return;

	for (i = 0; i < service->num_handles; i++) {
		attribute = service->attributes[i];
		if (!attribute)
			continue;

		if (attribute->handle < search_data->start_handle)
			continue;

		if (attribute->handle > search_data->end_handle)
			return;

		queue_push_tail(search_data->queue, attribute);
	}
}

void gatt_db_find_information(struct gatt_db *db, uint16_t start_handle,
							uint16_t end_handle,
							struct queue *queue)
{
	struct find_information_data data;

	data.start_handle = start_handle;
	data.end_handle = end_handle;
	data.queue = queue;

	queue_foreach(db->services, find_information, &data);
}

void gatt_db_foreach_service(struct gatt_db *db, const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	gatt_db_foreach_service_in_range(db, uuid, func, user_data, 0x0001,
									0xffff);
}

struct foreach_data {
	gatt_db_attribute_cb_t func;
	const bt_uuid_t *uuid;
	void *user_data;
	uint16_t start, end;
};

static void foreach_service_in_range(void *data, void *user_data)
{
	struct gatt_db_service *service = data;
	struct foreach_data *foreach_data = user_data;
	uint16_t svc_start;
	bt_uuid_t uuid;

	svc_start = get_handle_at_index(service, 0);

	if (svc_start > foreach_data->end || svc_start < foreach_data->start)
		return;

	if (foreach_data->uuid) {
		gatt_db_attribute_get_service_uuid(service->attributes[0],
									&uuid);
		if (bt_uuid_cmp(&uuid, foreach_data->uuid))
			return;
	}

	foreach_data->func(service->attributes[0], foreach_data->user_data);
}

void gatt_db_foreach_service_in_range(struct gatt_db *db,
						const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data,
						uint16_t start_handle,
						uint16_t end_handle)
{
	struct foreach_data data;

	if (!db || !func || start_handle > end_handle)
		return;

	data.func = func;
	data.uuid = uuid;
	data.user_data = user_data;
	data.start = start_handle;
	data.end = end_handle;

	queue_foreach(db->services, foreach_service_in_range, &data);
}

void gatt_db_service_foreach(struct gatt_db_attribute *attrib,
						const bt_uuid_t *uuid,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	struct gatt_db_service *service;
	struct gatt_db_attribute *attr;
	uint16_t i;

	if (!attrib || !func)
		return;

	service = attrib->service;

	for (i = 0; i < service->num_handles; i++) {
		attr = service->attributes[i];
		if (!attr)
			continue;

		if (uuid && bt_uuid_cmp(uuid, &attr->uuid))
			continue;

		func(attr, user_data);
	}
}

void gatt_db_service_foreach_char(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	gatt_db_service_foreach(attrib, &characteristic_uuid, func, user_data);
}

void gatt_db_service_foreach_desc(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	struct gatt_db_service *service;
	struct gatt_db_attribute *attr;
	uint16_t i;

	if (!attrib || !func)
		return;

	/* Return if this attribute is not a characteristic declaration */
	if (bt_uuid_cmp(&characteristic_uuid, &attrib->uuid))
		return;

	service = attrib->service;

	/* Start from the attribute following the value handle */
	for (i = 0; i < service->num_handles; i++) {
		if (service->attributes[i] == attrib) {
			i += 2;
			break;
		}
	}

	for (; i < service->num_handles; i++) {
		attr = service->attributes[i];
		if (!attr)
			continue;

		/* Return if we reached the end of this characteristic */
		if (!bt_uuid_cmp(&characteristic_uuid, &attr->uuid) ||
			!bt_uuid_cmp(&included_service_uuid, &attr->uuid))
			return;

		func(attr, user_data);
	}
}

void gatt_db_service_foreach_incl(struct gatt_db_attribute *attrib,
						gatt_db_attribute_cb_t func,
						void *user_data)
{
	gatt_db_service_foreach(attrib, &included_service_uuid, func,
								user_data);
}

static bool find_service_for_handle(const void *data, const void *user_data)
{
	const struct gatt_db_service *service = data;
	uint16_t handle = PTR_TO_UINT(user_data);
	uint16_t start, end;

	gatt_db_service_get_handles(service, &start, &end);

	return (start <= handle) && (handle <= end);
}

struct gatt_db_attribute *gatt_db_get_attribute(struct gatt_db *db,
							uint16_t handle)
{
	struct gatt_db_service *service;
	int i;

	if (!db || !handle)
		return NULL;

	service = queue_find(db->services, find_service_for_handle,
							UINT_TO_PTR(handle));
	if (!service)
		return NULL;

	for (i = 0; i < service->num_handles; i++) {
		if (!service->attributes[i])
			continue;

		if (service->attributes[i]->handle == handle)
			return service->attributes[i];
	}

	return NULL;
}

static bool find_service_with_uuid(const void *data, const void *user_data)
{
	const struct gatt_db_service *service = data;
	const bt_uuid_t *uuid = user_data;
	bt_uuid_t svc_uuid;

	gatt_db_attribute_get_service_uuid(service->attributes[0], &svc_uuid);

	return bt_uuid_cmp(uuid, &svc_uuid) == 0;
}

struct gatt_db_attribute *gatt_db_get_service_with_uuid(struct gatt_db *db,
							const bt_uuid_t *uuid)
{
	struct gatt_db_service *service;

	if (!db || !uuid)
		return NULL;

	service = queue_find(db->services, find_service_with_uuid, uuid);
	if (!service)
		return NULL;

	return service->attributes[0];
}

const bt_uuid_t *gatt_db_attribute_get_type(
					const struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return NULL;

	return &attrib->uuid;
}

uint16_t gatt_db_attribute_get_handle(const struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return 0;

	return attrib->handle;
}

bool gatt_db_attribute_get_service_uuid(const struct gatt_db_attribute *attrib,
							bt_uuid_t *uuid)
{
	struct gatt_db_service *service;

	if (!attrib || !uuid)
		return false;

	service = attrib->service;

	if (service->attributes[0]->value_len == sizeof(uint16_t)) {
		uint16_t value;

		value = get_le16(service->attributes[0]->value);
		bt_uuid16_create(uuid, value);

		return true;
	}

	if (service->attributes[0]->value_len == sizeof(uint128_t)) {
		uint128_t value;

		bswap_128(service->attributes[0]->value, &value);
		bt_uuid128_create(uuid, value);

		return true;
	}

	return false;
}

bool gatt_db_attribute_get_service_handles(
					const struct gatt_db_attribute *attrib,
					uint16_t *start_handle,
					uint16_t *end_handle)
{
	struct gatt_db_service *service;

	if (!attrib)
		return false;

	service = attrib->service;

	gatt_db_service_get_handles(service, start_handle, end_handle);

	return true;
}

bool gatt_db_attribute_get_service_data(const struct gatt_db_attribute *attrib,
							uint16_t *start_handle,
							uint16_t *end_handle,
							bool *primary,
							bt_uuid_t *uuid)
{
	struct gatt_db_service *service;
	struct gatt_db_attribute *decl;

	if (!attrib)
		return false;

	service = attrib->service;
	decl = service->attributes[0];

	gatt_db_service_get_handles(service, start_handle, end_handle);

	if (primary)
		*primary = bt_uuid_cmp(&decl->uuid, &secondary_service_uuid);

	if (!uuid)
		return true;

	/*
	 * The service declaration attribute value is the 16 or 128 bit service
	 * UUID.
	 */
	return le_to_uuid(decl->value, decl->value_len, uuid);
}

static void read_ext_prop_value(struct gatt_db_attribute *attrib,
						int err, const uint8_t *value,
						size_t length, void *user_data)
{
	uint16_t *ext_prop = user_data;

	if (err || (length != sizeof(uint16_t)))
		return;

	*ext_prop = (uint16_t) value[0];
}

static void read_ext_prop(struct gatt_db_attribute *attrib,
							void *user_data)
{
	uint16_t *ext_prop = user_data;

	/*
	 * If ext_prop is set that means extended properties descriptor
	 * has been already found
	 */
	if (*ext_prop != 0)
		return;

	if (bt_uuid_cmp(&ext_desc_uuid, &attrib->uuid))
		return;

	gatt_db_attribute_read(attrib, 0, BT_ATT_OP_READ_REQ, NULL,
						read_ext_prop_value, ext_prop);
}

static uint8_t get_char_extended_prop(const struct gatt_db_attribute *attrib)
{
	uint16_t ext_prop;

	if (!attrib)
		return 0;

	if (bt_uuid_cmp(&characteristic_uuid, &attrib->uuid))
		return 0;

	/* Check properties first */
	if (!(attrib->value[0] & BT_GATT_CHRC_PROP_EXT_PROP))
		return 0;

	ext_prop = 0;

	/*
	 * Cast needed for foreach function. We do not change attrib during
	 * this call
	 */
	gatt_db_service_foreach_desc((struct gatt_db_attribute *) attrib,
						read_ext_prop, &ext_prop);

	return ext_prop;
}

bool gatt_db_attribute_get_char_data(const struct gatt_db_attribute *attrib,
							uint16_t *handle,
							uint16_t *value_handle,
							uint8_t *properties,
							uint16_t *ext_prop,
							bt_uuid_t *uuid)
{
	if (!attrib)
		return false;

	if (bt_uuid_cmp(&characteristic_uuid, &attrib->uuid))
		return false;

	/*
	 * Characteristic declaration value:
	 * 1 octet: Characteristic properties
	 * 2 octets: Characteristic value handle
	 * 2 or 16 octets: characteristic UUID
	 */
	if (!attrib->value || (attrib->value_len != 5 &&
						attrib->value_len != 19))
		return false;

	if (handle)
		*handle = attrib->handle;

	if (properties)
		*properties = attrib->value[0];

	if (ext_prop)
		*ext_prop = get_char_extended_prop(attrib);

	if (value_handle)
		*value_handle = get_le16(attrib->value + 1);

	if (!uuid)
		return true;

	return le_to_uuid(attrib->value + 3, attrib->value_len - 3, uuid);
}

bool gatt_db_attribute_get_incl_data(const struct gatt_db_attribute *attrib,
							uint16_t *handle,
							uint16_t *start_handle,
							uint16_t *end_handle)
{
	if (!attrib)
		return false;

	if (bt_uuid_cmp(&included_service_uuid, &attrib->uuid))
		return false;

	/*
	 * Include definition value:
	 * 2 octets: start handle of included service
	 * 2 octets: end handle of included service
	 * optional 2 octets: 16-bit Bluetooth UUID
	 */
	if (!attrib->value || attrib->value_len < 4 || attrib->value_len > 6)
		return false;

	/*
	 * We only return the handles since the UUID can be easily obtained
	 * from the corresponding attribute.
	 */
	if (handle)
		*handle = attrib->handle;

	if (start_handle)
		*start_handle = get_le16(attrib->value);

	if (end_handle)
		*end_handle = get_le16(attrib->value + 2);

	return true;
}

uint32_t
gatt_db_attribute_get_permissions(const struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return 0;

	return attrib->permissions;
}

static bool read_timeout(void *user_data)
{
	struct pending_read *p = user_data;

	p->timeout_id = 0;

	queue_remove(p->attrib->pending_reads, p);

	pending_read_result(p, -ETIMEDOUT, NULL, 0);

	return false;
}

bool gatt_db_attribute_read(struct gatt_db_attribute *attrib, uint16_t offset,
				uint8_t opcode, struct bt_att *att,
				gatt_db_attribute_read_t func, void *user_data)
{
	uint8_t *value;

	if (!attrib || !func)
		return false;

	if (attrib->read_func) {
		struct pending_read *p;

		p = new0(struct pending_read, 1);
		p->attrib = attrib;
		p->id = ++attrib->read_id;
		p->timeout_id = timeout_add(ATTRIBUTE_TIMEOUT, read_timeout,
								p, NULL);
		p->func = func;
		p->user_data = user_data;

		queue_push_tail(attrib->pending_reads, p);

		attrib->read_func(attrib, p->id, offset, opcode, att,
							attrib->user_data);
		return true;
	}

	/* Check boundary if value is stored in the db */
	if (offset > attrib->value_len) {
		func(attrib, BT_ATT_ERROR_INVALID_OFFSET, NULL, 0, user_data);
		return true;
	}

	/* Guard against invalid access if offset equals to value length */
	value = offset == attrib->value_len ? NULL : &attrib->value[offset];

	func(attrib, 0, value, attrib->value_len - offset, user_data);

	return true;
}

static bool find_pending(const void *a, const void *b)
{
	const struct pending_read *p = a;
	unsigned int id = PTR_TO_UINT(b);

	return p->id == id;
}

bool gatt_db_attribute_read_result(struct gatt_db_attribute *attrib,
					unsigned int id, int err,
					const uint8_t *value, size_t length)
{
	struct pending_read *p;

	if (!attrib || !id)
		return false;

	p = queue_remove_if(attrib->pending_reads, find_pending,
							UINT_TO_PTR(id));
	if (!p)
		return false;

	pending_read_result(p, err, value, length);

	return true;
}

static bool write_timeout(void *user_data)
{
	struct pending_write *p = user_data;

	p->timeout_id = 0;

	queue_remove(p->attrib->pending_writes, p);

	pending_write_result(p, -ETIMEDOUT);

	return false;
}

bool gatt_db_attribute_write(struct gatt_db_attribute *attrib, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					gatt_db_attribute_write_t func,
					void *user_data)
{
	if (!attrib || !func)
		return false;

	if (attrib->write_func) {
		struct pending_write *p;

		p = new0(struct pending_write, 1);
		p->attrib = attrib;
		p->id = ++attrib->write_id;
		p->timeout_id = timeout_add(ATTRIBUTE_TIMEOUT, write_timeout,
								p, NULL);
		p->func = func;
		p->user_data = user_data;

		queue_push_tail(attrib->pending_writes, p);

		attrib->write_func(attrib, p->id, offset, value, len, opcode,
							att, attrib->user_data);
		return true;
	}

	/* Nothing to write just skip */
	if (len == 0)
		goto done;

	/* For values stored in db allocate on demand */
	if (!attrib->value || offset >= attrib->value_len ||
				len > (unsigned) (attrib->value_len - offset)) {
		void *buf;

		buf = realloc(attrib->value, len + offset);
		if (!buf)
			return false;

		attrib->value = buf;

		/* Init data in the first allocation */
		if (!attrib->value_len)
			memset(attrib->value, 0, offset);

		attrib->value_len = len + offset;
	}

	memcpy(&attrib->value[offset], value, len);

done:
	func(attrib, 0, user_data);

	return true;
}

bool gatt_db_attribute_write_result(struct gatt_db_attribute *attrib,
						unsigned int id, int err)
{
	struct pending_write *p;

	if (!attrib || !id)
		return false;

	p = queue_remove_if(attrib->pending_writes, find_pending,
							UINT_TO_PTR(id));
	if (!p)
		return false;

	pending_write_result(p, err);

	return true;
}

bool gatt_db_attribute_reset(struct gatt_db_attribute *attrib)
{
	if (!attrib)
		return false;

	if (!attrib->value || !attrib->value_len)
		return true;

	free(attrib->value);
	attrib->value = NULL;
	attrib->value_len = 0;

	return true;
}
