/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
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

#include "src/shared/ad.h"

#include "src/eir.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"

#define MAX_ADV_DATA_LEN 31

struct bt_ad {
	int ref_count;
	struct queue *service_uuids;
	struct queue *manufacturer_data;
	struct queue *solicit_uuids;
	struct queue *service_data;
};

struct bt_ad *bt_ad_new(void)
{
	struct bt_ad *ad;

	ad = new0(struct bt_ad, 1);
	ad->service_uuids = queue_new();
	ad->manufacturer_data = queue_new();
	ad->solicit_uuids = queue_new();
	ad->service_data = queue_new();

	return bt_ad_ref(ad);
}

struct bt_ad *bt_ad_ref(struct bt_ad *ad)
{
	if (!ad)
		return NULL;

	ad->ref_count++;
	return ad;
}

static void uuid_destroy(void *data)
{
	struct bt_ad_service_data *uuid_data = data;

	free(uuid_data->data);
	free(uuid_data);
}

static bool uuid_data_match(const void *data, const void *elem)
{
	const struct bt_ad_service_data *uuid_data = elem;
	const bt_uuid_t *uuid = data;

	return !bt_uuid_cmp(&uuid_data->uuid, uuid);
}

static void manuf_destroy(void *data)
{
	struct bt_ad_manufacturer_data *manuf = data;

	free(manuf->data);
	free(manuf);
}

static bool manuf_match(const void *data, const void *elem)
{
	const struct bt_ad_manufacturer_data *manuf = elem;
	uint16_t manuf_id = PTR_TO_UINT(elem);

	return manuf->manufacturer_id == manuf_id;
}

void bt_ad_unref(struct bt_ad *ad)
{
	if (!ad)
		return;

	if (__sync_sub_and_fetch(&ad->ref_count, 1))
		return;

	queue_destroy(ad->service_uuids, free);

	queue_destroy(ad->manufacturer_data, manuf_destroy);

	queue_destroy(ad->solicit_uuids, free);

	queue_destroy(ad->service_data, uuid_destroy);

	free(ad);
}

static size_t uuid_list_length(struct queue *uuid_queue)
{
	bool uuid16_included = false;
	bool uuid32_included = false;
	bool uuid128_included = false;
	size_t length = 0;
	const struct queue_entry *entry;

	entry = queue_get_entries(uuid_queue);

	while (entry) {
		bt_uuid_t *uuid = entry->data;

		length += bt_uuid_len(uuid);

		if (uuid->type == BT_UUID16)
			uuid16_included = true;
		else if (uuid->type == BT_UUID32)
			uuid32_included = true;
		else
			uuid128_included = true;

		entry = entry->next;
	}

	if (uuid16_included)
		length += 2;

	if (uuid32_included)
		length += 2;

	if (uuid128_included)
		length += 2;

	return length;
}

static size_t mfg_data_length(struct queue *manuf_data)
{
	size_t length = 0;
	const struct queue_entry *entry;

	entry = queue_get_entries(manuf_data);

	while (entry) {
		struct bt_ad_manufacturer_data *data = entry->data;

		length += 2 + sizeof(uint16_t) + data->len;

		entry = entry->next;
	}

	return length;
}

static size_t uuid_data_length(struct queue *uuid_data)
{
	size_t length = 0;
	const struct queue_entry *entry;

	entry = queue_get_entries(uuid_data);

	while (entry) {
		struct bt_ad_service_data *data = entry->data;

		length += 2 + bt_uuid_len(&data->uuid) + data->len;

		entry = entry->next;
	}

	return length;
}

static size_t calculate_length(struct bt_ad *ad)
{
	size_t length = 0;

	length += uuid_list_length(ad->service_uuids);

	length += uuid_list_length(ad->solicit_uuids);

	length += mfg_data_length(ad->manufacturer_data);

	length += uuid_data_length(ad->service_data);

	return length;
}

static void serialize_uuids(struct queue *uuids, uint8_t uuid_type,
						uint8_t ad_type, uint8_t *buf,
						uint8_t *pos)
{
	const struct queue_entry *entry = queue_get_entries(uuids);
	bool added = false;
	uint8_t length_pos = 0;

	while (entry) {
		bt_uuid_t *uuid = entry->data;

		if (uuid->type == uuid_type) {
			if (!added) {
				length_pos = (*pos)++;
				buf[(*pos)++] = ad_type;
				added = true;
			}

			if (uuid_type != BT_UUID32)
				bt_uuid_to_le(uuid, buf + *pos);
			else
				bt_put_le32(uuid->value.u32, buf + *pos);

			*pos += bt_uuid_len(uuid);
		}

		entry = entry->next;
	}

	if (added)
		buf[length_pos] = *pos - length_pos - 1;
}

static void serialize_service_uuids(struct queue *uuids, uint8_t *buf,
								uint8_t *pos)
{
	serialize_uuids(uuids, BT_UUID16, EIR_UUID16_ALL, buf, pos);

	serialize_uuids(uuids, BT_UUID32, EIR_UUID32_ALL, buf, pos);

	serialize_uuids(uuids, BT_UUID128, EIR_UUID128_ALL, buf, pos);
}

static void serialize_solicit_uuids(struct queue *uuids, uint8_t *buf,
								uint8_t *pos)
{
	serialize_uuids(uuids, BT_UUID16, EIR_SOLICIT16, buf, pos);

	serialize_uuids(uuids, BT_UUID32, EIR_SOLICIT32, buf, pos);

	serialize_uuids(uuids, BT_UUID128, EIR_SOLICIT128, buf, pos);
}

static void serialize_manuf_data(struct queue *manuf_data, uint8_t *buf,
								uint8_t *pos)
{
	const struct queue_entry *entry = queue_get_entries(manuf_data);

	while (entry) {
		struct bt_ad_manufacturer_data *data = entry->data;

		buf[(*pos)++] = data->len + 2 + 1;

		buf[(*pos)++] = EIR_MANUFACTURER_DATA;

		bt_put_le16(data->manufacturer_id, buf + (*pos));

		*pos += 2;

		memcpy(buf + *pos, data->data, data->len);

		*pos += data->len;

		entry = entry->next;
	}
}

static void serialize_service_data(struct queue *service_data, uint8_t *buf,
								uint8_t *pos)
{
	const struct queue_entry *entry = queue_get_entries(service_data);

	while (entry) {
		struct bt_ad_service_data *data = entry->data;
		int uuid_len = bt_uuid_len(&data->uuid);

		buf[(*pos)++] =  uuid_len + data->len + 1;

		switch (uuid_len) {
		case 2:
			buf[(*pos)++] = EIR_SVC_DATA16;
			break;
		case 4:
			buf[(*pos)++] = EIR_SVC_DATA32;
			break;
		case 16:
			buf[(*pos)++] = EIR_SVC_DATA128;
			break;
		}

		if (uuid_len != 4)
			bt_uuid_to_le(&data->uuid, buf + *pos);
		else
			bt_put_le32(data->uuid.value.u32, buf + *pos);

		*pos += uuid_len;

		memcpy(buf + *pos, data->data, data->len);

		*pos += data->len;

		entry = entry->next;
	}
}

uint8_t *bt_ad_generate(struct bt_ad *ad, size_t *length)
{
	uint8_t *adv_data;
	uint8_t pos = 0;

	if (!ad)
		return NULL;

	*length = calculate_length(ad);

	if (*length > MAX_ADV_DATA_LEN)
		return NULL;

	adv_data = malloc0(*length);
	if (!adv_data)
		return NULL;

	serialize_service_uuids(ad->service_uuids, adv_data, &pos);

	serialize_solicit_uuids(ad->solicit_uuids, adv_data, &pos);

	serialize_manuf_data(ad->manufacturer_data, adv_data, &pos);

	serialize_service_data(ad->service_data, adv_data, &pos);

	return adv_data;
}

static bool queue_add_uuid(struct queue *queue, const bt_uuid_t *uuid)
{
	bt_uuid_t *new_uuid;

	if (!queue)
		return false;

	new_uuid = new0(bt_uuid_t, 1);

	*new_uuid = *uuid;

	if (queue_push_tail(queue, new_uuid))
		return true;

	free(new_uuid);

	return false;
}

static bool uuid_match(const void *data, const void *elem)
{
	const bt_uuid_t *match_uuid = data;
	const bt_uuid_t *uuid = elem;

	return bt_uuid_cmp(match_uuid, uuid);
}

static bool queue_remove_uuid(struct queue *queue, bt_uuid_t *uuid)
{
	bt_uuid_t *removed;

	if (!queue || !uuid)
		return false;

	removed = queue_remove_if(queue, uuid_match, uuid);

	if (removed) {
		free(removed);
		return true;
	}

	return false;
}

bool bt_ad_add_service_uuid(struct bt_ad *ad, const bt_uuid_t *uuid)
{
	if (!ad)
		return false;

	return queue_add_uuid(ad->service_uuids, uuid);
}

bool bt_ad_remove_service_uuid(struct bt_ad *ad, bt_uuid_t *uuid)
{
	if (!ad)
		return false;

	return queue_remove_uuid(ad->service_uuids, uuid);
}

void bt_ad_clear_service_uuid(struct bt_ad *ad)
{
	if (!ad)
		return;

	queue_remove_all(ad->service_uuids, NULL, NULL, free);
}

static bool manufacturer_id_data_match(const void *data, const void *user_data)
{
	const struct bt_ad_manufacturer_data *m = data;
	uint16_t id = PTR_TO_UINT(user_data);

	return m->manufacturer_id == id;
}

bool bt_ad_add_manufacturer_data(struct bt_ad *ad, uint16_t manufacturer_id,
							void *data, size_t len)
{
	struct bt_ad_manufacturer_data *new_data;

	if (!ad)
		return false;

	if (len > (MAX_ADV_DATA_LEN - 2 - sizeof(uint16_t)))
		return false;

	new_data = queue_find(ad->manufacturer_data, manufacturer_id_data_match,
						UINT_TO_PTR(manufacturer_id));
	if (new_data) {
		if (new_data->len == len && !memcmp(new_data->data, data, len))
			return false;
		new_data->data = realloc(new_data->data, len);
		memcpy(new_data->data, data, len);
		new_data->len = len;
		return true;
	}

	new_data = new0(struct bt_ad_manufacturer_data, 1);
	new_data->manufacturer_id = manufacturer_id;

	new_data->data = malloc(len);
	if (!new_data->data) {
		free(new_data);
		return false;
	}

	memcpy(new_data->data, data, len);

	new_data->len = len;

	if (queue_push_tail(ad->manufacturer_data, new_data))
		return true;

	manuf_destroy(new_data);

	return false;
}

static bool manufacturer_data_match(const void *data, const void *user_data)
{
	const struct bt_ad_manufacturer_data *m1 = data;
	const struct bt_ad_manufacturer_data *m2 = user_data;

	if (m1->manufacturer_id != m2->manufacturer_id)
		return false;

	if (m1->len != m2->len)
		return false;

	return !memcmp(m1->data, m2->data, m1->len);
}

bool bt_ad_has_manufacturer_data(struct bt_ad *ad,
				const struct bt_ad_manufacturer_data *data)
{
	if (!ad)
		return false;

	if (!data)
		return !queue_isempty(ad->manufacturer_data);

	return queue_find(ad->manufacturer_data, manufacturer_data_match, data);
}

void bt_ad_foreach_manufacturer_data(struct bt_ad *ad, bt_ad_func_t func,
							void *user_data)
{
	if (!ad)
		return;

	queue_foreach(ad->manufacturer_data, func, user_data);
}

bool bt_ad_remove_manufacturer_data(struct bt_ad *ad, uint16_t manufacturer_id)
{
	struct bt_ad_manufacturer_data *data;

	if (!ad)
		return false;

	data = queue_remove_if(ad->manufacturer_data, manuf_match,
						UINT_TO_PTR(manufacturer_id));

	if (!data)
		return false;

	manuf_destroy(data);

	return true;
}

void bt_ad_clear_manufacturer_data(struct bt_ad *ad)
{
	if (!ad)
		return;

	queue_remove_all(ad->manufacturer_data, NULL, NULL, manuf_destroy);
}

bool bt_ad_add_solicit_uuid(struct bt_ad *ad, const bt_uuid_t *uuid)
{
	if (!ad)
		return false;

	return queue_add_uuid(ad->solicit_uuids, uuid);
}

bool bt_ad_remove_solicit_uuid(struct bt_ad *ad, bt_uuid_t *uuid)
{
	if (!ad)
		return false;

	return queue_remove_uuid(ad->solicit_uuids, uuid);
}

void bt_ad_clear_solicit_uuid(struct bt_ad *ad)
{
	if (!ad)
		return;

	queue_remove_all(ad->solicit_uuids, NULL, NULL, free);
}


static bool service_uuid_match(const void *data, const void *user_data)
{
	const struct bt_ad_service_data *s = data;
	const bt_uuid_t *uuid = user_data;

	return !bt_uuid_cmp(&s->uuid, uuid);
}

bool bt_ad_add_service_data(struct bt_ad *ad, const bt_uuid_t *uuid, void *data,
								size_t len)
{
	struct bt_ad_service_data *new_data;

	if (!ad)
		return false;

	if (len > (MAX_ADV_DATA_LEN - 2 - (size_t)bt_uuid_len(uuid)))
		return false;

	new_data = queue_find(ad->service_data, service_uuid_match, uuid);
	if (new_data) {
		if (new_data->len == len && !memcmp(new_data->data, data, len))
			return false;
		new_data->data = realloc(new_data->data, len);
		memcpy(new_data->data, data, len);
		new_data->len = len;
		return true;
	}

	new_data = new0(struct bt_ad_service_data, 1);

	new_data->uuid = *uuid;

	new_data->data = malloc(len);
	if (!new_data->data) {
		free(new_data);
		return false;
	}

	memcpy(new_data->data, data, len);

	new_data->len = len;

	if (queue_push_tail(ad->service_data, new_data))
		return true;

	uuid_destroy(new_data);

	return false;
}

static bool service_data_match(const void *data, const void *user_data)
{
	const struct bt_ad_service_data *s1 = data;
	const struct bt_ad_service_data *s2 = user_data;

	if (bt_uuid_cmp(&s1->uuid, &s2->uuid))
		return false;

	if (s1->len != s2->len)
		return false;

	return !memcmp(s1->data, s2->data, s1->len);
}

bool bt_ad_has_service_data(struct bt_ad *ad,
					const struct bt_ad_service_data *data)
{
	if (!ad)
		return false;

	if (!data)
		return !queue_isempty(ad->service_data);

	return queue_find(ad->service_data, service_data_match, data);
}

void bt_ad_foreach_service_data(struct bt_ad *ad, bt_ad_func_t func,
							void *user_data)
{
	if (!ad)
		return;

	queue_foreach(ad->service_data, func, user_data);
}

bool bt_ad_remove_service_data(struct bt_ad *ad, bt_uuid_t *uuid)
{
	struct bt_ad_service_data *data;

	if (!ad)
		return false;

	data = queue_remove_if(ad->service_data, uuid_data_match, uuid);

	if (!data)
		return false;

	uuid_destroy(data);

	return true;
}

void bt_ad_clear_service_data(struct bt_ad *ad)
{
	if (!ad)
		return;

	queue_remove_all(ad->service_data, NULL, NULL, uuid_destroy);
}
