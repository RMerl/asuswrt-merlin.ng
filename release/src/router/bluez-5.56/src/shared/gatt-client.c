// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "src/shared/att.h"
#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/gatt-helpers.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"

#include <assert.h>
#include <limits.h>
#include <sys/uio.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define UUID_BYTES (BT_GATT_UUID_SIZE * sizeof(uint8_t))

#define GATT_SVC_UUID	0x1801
#define SVC_CHNGD_UUID	0x2a05

struct ready_cb {
	bt_gatt_client_callback_t callback;
	bt_gatt_client_destroy_func_t destroy;
	void *data;
};

struct bt_gatt_client {
	struct bt_att *att;
	int ref_count;
	uint8_t features;

	struct bt_gatt_client *parent;
	struct queue *clones;

	struct queue *ready_cbs;

	bt_gatt_client_service_changed_callback_t svc_chngd_callback;
	bt_gatt_client_destroy_func_t svc_chngd_destroy;
	void *svc_chngd_data;

	bt_gatt_client_debug_func_t debug_callback;
	bt_gatt_client_destroy_func_t debug_destroy;
	void *debug_data;

	struct gatt_db *db;
	bool in_init;
	bool ready;

	/*
	 * Queue of long write requests. An error during "prepare write"
	 * requests can result in a cancel through "execute write". To prevent
	 * cancelation of prepared writes to the wrong attribute and multiple
	 * requests to the same attribute that may result in a corrupted final
	 * value, we avoid interleaving prepared writes.
	 */
	struct queue *long_write_queue;
	bool in_long_write;

	unsigned int reliable_write_session_id;

	/* List of registered disconnect/notification/indication callbacks */
	struct queue *notify_list;
	struct queue *notify_chrcs;
	int next_reg_id;
	unsigned int disc_id, nfy_id, nfy_mult_id, ind_id;

	/*
	 * Handles of the GATT Service and the Service Changed characteristic
	 * value handle. These will have the value 0 if they are not present on
	 * the remote peripheral.
	 */
	unsigned int svc_chngd_ind_id;
	bool svc_chngd_registered;
	struct queue *svc_chngd_queue;  /* Queued service changed events */
	bool in_svc_chngd;

	/*
	 * List of pending read/write operations. For operations that span
	 * across multiple PDUs, this list provides a mapping from an operation
	 * id to an ATT request id.
	 */
	struct queue *pending_requests;
	unsigned int next_request_id;

	struct bt_gatt_request *discovery_req;
	unsigned int mtu_req_id;
};

struct request {
	struct bt_gatt_client *client;
	bool long_write;
	bool prep_write;
	bool removed;
	int ref_count;
	unsigned int id;
	unsigned int att_id;
	void *data;
	void (*destroy)(void *);
};

static struct request *request_ref(struct request *req)
{
	__sync_fetch_and_add(&req->ref_count, 1);

	return req;
}

static struct request *request_create(struct bt_gatt_client *client)
{
	struct request *req;

	if (!client->att)
		return NULL;

	req = new0(struct request, 1);

	if (client->next_request_id < 1)
		client->next_request_id = 1;

	queue_push_tail(client->pending_requests, req);
	req->client = client;
	req->id = client->next_request_id++;

	return request_ref(req);
}

static void request_unref(void *data)
{
	struct request *req = data;

	if (__sync_sub_and_fetch(&req->ref_count, 1))
		return;

	if (req->destroy)
		req->destroy(req->data);

	if (!req->removed)
		queue_remove(req->client->pending_requests, req);

	free(req);
}

struct notify_chrc {
	struct bt_gatt_client *client;
	struct gatt_db_attribute *attr;
	uint16_t value_handle;
	uint16_t ccc_handle;
	uint16_t properties;
	unsigned int notify_id;
	int notify_count;  /* Reference count of registered notify callbacks */

	/* Pending calls to register_notify are queued here so that they can be
	 * processed after a write that modifies the CCC descriptor.
	 */
	struct queue *reg_notify_queue;
	unsigned int ccc_write_id;
};

struct notify_data {
	struct bt_gatt_client *client;
	unsigned int id;
	unsigned int att_id;
	uint8_t att_ecode;
	int ref_count;
	struct notify_chrc *chrc;
	bt_gatt_client_register_callback_t callback;
	bt_gatt_client_notify_callback_t notify;
	void *user_data;
	bt_gatt_client_destroy_func_t destroy;
};

static struct notify_data *notify_data_ref(struct notify_data *notify_data)
{
	__sync_fetch_and_add(&notify_data->ref_count, 1);

	return notify_data;
}

static void notify_data_unref(void *data)
{
	struct notify_data *notify_data = data;

	if (__sync_sub_and_fetch(&notify_data->ref_count, 1))
		return;

	if (notify_data->destroy)
		notify_data->destroy(notify_data->user_data);

	free(notify_data);
}

static void find_ccc(struct gatt_db_attribute *attr, void *user_data)
{
	struct gatt_db_attribute **ccc_ptr = user_data;
	bt_uuid_t uuid;

	if (*ccc_ptr)
		return;

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);

	if (bt_uuid_cmp(&uuid, gatt_db_attribute_get_type(attr)))
		return;

	*ccc_ptr = attr;
}

static bool match_notify_chrc(const void *data, const void *user_data)
{
	const struct notify_data *notify_data = data;
	const struct notify_chrc *chrc = user_data;

	return notify_data->chrc == chrc;
}

static void notify_data_cleanup(void *data)
{
	struct notify_data *notify_data = data;

	if (notify_data->att_id)
		bt_att_cancel(notify_data->client->att, notify_data->att_id);

	notify_data_unref(notify_data);
}

static void notify_chrc_free(void *data)
{
	struct notify_chrc *chrc = data;

	if (chrc->notify_id)
		gatt_db_attribute_unregister(chrc->attr, chrc->notify_id);

	queue_destroy(chrc->reg_notify_queue, notify_data_unref);
	free(chrc);
}

static void chrc_removed(struct gatt_db_attribute *attr, void *user_data)
{
	struct notify_chrc *chrc = user_data;
	struct bt_gatt_client *client = chrc->client;
	struct notify_data *data;

	chrc->notify_id = 0;

	while ((data = queue_remove_if(client->notify_list, match_notify_chrc,
								chrc)))
		notify_data_cleanup(data);

	queue_remove(client->notify_chrcs, chrc);
	notify_chrc_free(chrc);
}

static struct notify_chrc *notify_chrc_create(struct bt_gatt_client *client,
							uint16_t value_handle)
{
	struct gatt_db_attribute *attr, *ccc;
	struct notify_chrc *chrc;
	bt_uuid_t uuid;
	uint8_t properties;

	/* Check that chrc_value_handle belongs to a known characteristic */
	attr = gatt_db_get_attribute(client->db, value_handle - 1);
	if (!attr)
		return NULL;

	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	if (bt_uuid_cmp(&uuid, gatt_db_attribute_get_type(attr)))
		return NULL;

	if (!gatt_db_attribute_get_char_data(attr, NULL, NULL, &properties,
								NULL, NULL))
		return NULL;

	chrc = new0(struct notify_chrc, 1);

	chrc->reg_notify_queue = queue_new();
	if (!chrc->reg_notify_queue) {
		free(chrc);
		return NULL;
	}

	/*
	 * Find the CCC characteristic. Some characteristics that allow
	 * notifications may not have a CCC descriptor. We treat these as
	 * automatically successful.
	 */
	ccc = NULL;
	gatt_db_service_foreach_desc(attr, find_ccc, &ccc);
	if (ccc)
		chrc->ccc_handle = gatt_db_attribute_get_handle(ccc);

	chrc->client = client;
	chrc->attr = attr;
	chrc->value_handle = value_handle;
	chrc->properties = properties;
	chrc->notify_id = gatt_db_attribute_register(attr, chrc_removed, chrc,
									NULL);

	queue_push_tail(client->notify_chrcs, chrc);

	return chrc;
}

static bool match_notify_data_id(const void *a, const void *b)
{
	const struct notify_data *notify_data = a;
	unsigned int id = PTR_TO_UINT(b);

	return notify_data->id == id;
}

struct handle_range {
	uint16_t start;
	uint16_t end;
};

struct discovery_op;

typedef void (*discovery_op_complete_func_t)(struct discovery_op *op,
							bool success,
							uint8_t att_ecode);
typedef void (*discovery_op_fail_func_t)(struct discovery_op *op);

struct discovery_op {
	struct bt_gatt_client *client;
	struct queue *discov_ranges;
	struct queue *pending_svcs;
	struct queue *pending_chrcs;
	struct queue *ext_prop_desc;
	struct gatt_db_attribute *cur_svc;
	struct gatt_db_attribute *hash;
	uint8_t server_feat;
	bool success;
	uint16_t start;
	uint16_t end;
	uint16_t last;
	uint16_t svc_first;
	uint16_t svc_last;
	unsigned int db_id;
	int ref_count;
	discovery_op_complete_func_t complete_func;
	discovery_op_fail_func_t failure_func;
};

static void discovery_op_free(struct discovery_op *op)
{
	if (op->db_id > 0)
		gatt_db_unregister(op->client->db, op->db_id);

	queue_destroy(op->discov_ranges, free);
	queue_destroy(op->pending_svcs, NULL);
	queue_destroy(op->pending_chrcs, free);
	queue_destroy(op->ext_prop_desc, NULL);
	free(op);
}

static bool read_db_hash(struct discovery_op *op);

static void discovery_op_complete(struct discovery_op *op, bool success,
								uint8_t err)
{
	const struct queue_entry *svc;

	op->success = success;

	/* Read database hash if discovery has been successful */
	if (success && read_db_hash(op))
		return;

	/*
	 * Unregister remove callback so it is not called when clearing unused
	 * range.
	 */
	gatt_db_unregister(op->client->db, op->db_id);
	op->db_id = 0;

	/* Remove services pending */
	for (svc = queue_get_entries(op->pending_svcs); svc; svc = svc->next) {
		struct gatt_db_attribute *attr = svc->data;
		uint16_t start, end;

		/* Leave active services if operation was aborted */
		if ((!success && err == 0) && gatt_db_service_get_active(attr))
			continue;

		gatt_db_attribute_get_service_data(attr, &start, &end,
							NULL, NULL);

		util_debug(op->client->debug_callback, op->client->debug_data,
				"service disappeared: start 0x%04x end 0x%04x",
				start, end);

		gatt_db_remove_service(op->client->db, attr);
	}

	/* Reset remaining range */
	if (op->last != UINT16_MAX)
		gatt_db_clear_range(op->client->db, op->last + 1, UINT16_MAX);

	op->complete_func(op, success, err);
}

static void discovery_load_services(struct gatt_db_attribute *attr,
							void *user_data)
{
	struct discovery_op *op = user_data;

	queue_push_tail(op->pending_svcs, attr);
}

static void discovery_service_changed(struct gatt_db_attribute *attr,
							void *user_data)
{
	struct discovery_op *op = user_data;

	queue_remove(op->pending_svcs, attr);
}

static struct discovery_op *discovery_op_create(struct bt_gatt_client *client,
				uint16_t start, uint16_t end,
				discovery_op_complete_func_t complete_func,
				discovery_op_fail_func_t failure_func)
{
	struct discovery_op *op;
	struct handle_range *range;

	op = new0(struct discovery_op, 1);
	op->discov_ranges = queue_new();
	op->pending_svcs = queue_new();
	op->pending_chrcs = queue_new();
	op->ext_prop_desc = queue_new();
	op->client = client;
	op->complete_func = complete_func;
	op->failure_func = failure_func;
	op->start = start;
	op->end = end;
	op->last = gatt_db_isempty(client->db) ? 0 : UINT16_MAX;
	op->svc_first = UINT16_MAX;
	op->svc_last = 0;

	/* Load existing services as pending */
	gatt_db_foreach_service_in_range(client->db, NULL,
					 discovery_load_services, op,
					 start, end);

	/*
	 * Services are only added when set active in which case they are no
	 * longer pending so it is safe to remove either way.
	 */
	op->db_id = gatt_db_register(client->db, discovery_service_changed,
						discovery_service_changed,
						op, NULL);

	range = new0(struct handle_range, 1);
	range->start = start;
	range->end = end;
	queue_push_tail(op->discov_ranges, range);

	return op;
}

static struct discovery_op *discovery_op_ref(struct discovery_op *op)
{
	__sync_fetch_and_add(&op->ref_count, 1);

	return op;
}

static void discovery_op_unref(void *data)
{
	struct discovery_op *op = data;

	if (__sync_sub_and_fetch(&op->ref_count, 1))
		return;

	if (!op->success && op->failure_func)
		op->failure_func(op);

	discovery_op_free(op);
}

static void discovery_req_clear(struct bt_gatt_client *client)
{
	if (!client->discovery_req)
		return;

	bt_gatt_request_unref(client->discovery_req);
	client->discovery_req = NULL;
}

static void discover_chrcs_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data);

static void discover_incl_cb(bool success, uint8_t att_ecode,
				struct bt_gatt_result *result, void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	struct bt_gatt_iter iter;
	struct gatt_db_attribute *attr;
	uint16_t handle, start, end;
	uint128_t u128;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];
	unsigned int includes_count, i;
	struct handle_range *range;

	discovery_req_clear(client);

	if (!success) {
		if (att_ecode == BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND)
			goto next;

		goto failed;
	}

	if (!result || !bt_gatt_iter_init(&iter, result))
		goto failed;

	includes_count = bt_gatt_result_included_count(result);
	if (includes_count == 0)
		goto failed;

	util_debug(client->debug_callback, client->debug_data,
						"Included services found: %u",
						includes_count);

	for (i = 0; i < includes_count; i++) {
		if (!bt_gatt_iter_next_included_service(&iter, &handle, &start,
							&end, u128.data))
			break;

		bt_uuid128_create(&uuid, u128);

		/* Log debug message */
		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		util_debug(client->debug_callback, client->debug_data,
				"handle: 0x%04x, start: 0x%04x, end: 0x%04x,"
				"uuid: %s", handle, start, end, uuid_str);

		attr = gatt_db_get_attribute(client->db, start);
		if (!attr) {
			util_debug(client->debug_callback, client->debug_data,
				"Unable to find attribute at 0x%04x", start);
			goto failed;
		}

		attr = gatt_db_insert_included(client->db, handle, attr);
		if (!attr) {
			util_debug(client->debug_callback, client->debug_data,
				"Unable to add include attribute at 0x%04x",
				handle);
			goto failed;
		}

		/*
		 * GATT requires that all include definitions precede
		 * characteristic declarations. Based on the order we're adding
		 * these entries, the correct handle must be assigned to the new
		 * attribute.
		 */
		if (gatt_db_attribute_get_handle(attr) != handle) {
			util_debug(client->debug_callback, client->debug_data,
				"Invalid attribute 0x%04x expect it at 0x%04x",
				gatt_db_attribute_get_handle(attr), handle);
			goto failed;
		}
	}

next:
	range = queue_pop_head(op->discov_ranges);
	if (!range)
		goto failed;

	client->discovery_req = bt_gatt_discover_characteristics(client->att,
							range->start,
							range->end,
							discover_chrcs_cb,
							discovery_op_ref(op),
							discovery_op_unref);
	free(range);
	if (client->discovery_req)
		return;

	util_debug(client->debug_callback, client->debug_data,
				"Failed to start characteristic discovery");
	discovery_op_unref(op);
failed:
	discovery_op_complete(op, false, att_ecode);
}

struct chrc {
	uint16_t start_handle;
	uint16_t end_handle;
	uint16_t value_handle;
	uint8_t properties;
	bt_uuid_t uuid;
};

static void discover_descs_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data);

static bool discover_descs(struct discovery_op *op, bool *discovering)
{
	struct bt_gatt_client *client = op->client;
	struct gatt_db_attribute *attr;
	struct chrc *chrc_data;
	uint16_t desc_start;

	*discovering = false;

	while ((chrc_data = queue_pop_head(op->pending_chrcs))) {
		struct gatt_db_attribute *svc;
		uint16_t start, end;

		/* Adjust current service */
		svc = gatt_db_get_service(client->db, chrc_data->value_handle);
		if (op->cur_svc != svc) {
			if (op->cur_svc) {
				queue_remove(op->pending_svcs, op->cur_svc);

				/* Done with the current service */
				gatt_db_service_set_active(op->cur_svc, true);
			}

			op->cur_svc = svc;
		}

		attr = gatt_db_insert_characteristic(client->db,
							chrc_data->value_handle,
							&chrc_data->uuid, 0,
							chrc_data->properties,
							NULL, NULL, NULL);

		if (!attr) {
			util_debug(client->debug_callback, client->debug_data,
				"Failed to insert characteristic at 0x%04x",
				chrc_data->value_handle);

			/* Some devices have been seen reporting orphaned
			 * characteristics.  In order to favor interoperability
			 * we skip over characteristics in error
			 */
			free(chrc_data);
			continue;
		}

		if (gatt_db_attribute_get_handle(attr) !=
							chrc_data->value_handle)
			goto failed;

		gatt_db_attribute_get_service_handles(svc, &start, &end);

		/*
		 * Adjust end_handle in case the next chrc is not within the
		 * same service.
		 */
		if (chrc_data->end_handle > end)
			chrc_data->end_handle = end;

		/*
		 * check for descriptors presence, before initializing the
		 * desc_handle and avoid integer overflow during desc_handle
		 * initialization.
		 */
		if (chrc_data->value_handle >= chrc_data->end_handle) {
			free(chrc_data);
			continue;
		}

		desc_start = chrc_data->value_handle + 1;

		if (desc_start == chrc_data->end_handle &&
			(chrc_data->properties & BT_GATT_CHRC_PROP_NOTIFY ||
			 chrc_data->properties & BT_GATT_CHRC_PROP_INDICATE)) {
			bt_uuid_t ccc_uuid;

			/* If there is only one descriptor that must be the CCC
			 * in case either notify or indicate are supported.
			 */
			bt_uuid16_create(&ccc_uuid,
					GATT_CLIENT_CHARAC_CFG_UUID);
			attr = gatt_db_insert_descriptor(client->db, desc_start,
							&ccc_uuid, 0, NULL,
							NULL, NULL);
			if (attr) {
				free(chrc_data);
				continue;
			}
		}

		/* Check if the start range is within characteristic range */
		if (desc_start > chrc_data->end_handle) {
			free(chrc_data);
			continue;
		}

		client->discovery_req = bt_gatt_discover_descriptors(
							client->att, desc_start,
							chrc_data->end_handle,
							discover_descs_cb,
							discovery_op_ref(op),
							discovery_op_unref);
		if (client->discovery_req) {
			*discovering = true;
			goto done;
		}

		util_debug(client->debug_callback, client->debug_data,
					"Failed to start descriptor discovery");
		discovery_op_unref(op);

		goto failed;
	}

done:
	free(chrc_data);
	return true;

failed:
	free(chrc_data);
	return false;
}

static void ext_prop_write_cb(struct gatt_db_attribute *attrib,
						int err, void *user_data)
{
	struct bt_gatt_client *client = user_data;

	util_debug(client->debug_callback, client->debug_data,
						"Value set status: %d", err);
}

static void ext_prop_read_cb(bool success, uint8_t att_ecode,
					const uint8_t *value, uint16_t length,
					void *user_data);

static bool read_ext_prop_desc(struct discovery_op *op)
{
	struct bt_gatt_client *client = op->client;
	uint16_t handle;
	struct gatt_db_attribute *attr;

	attr = queue_peek_head(op->ext_prop_desc);
	if (!attr)
		return false;

	handle = gatt_db_attribute_get_handle(attr);

	if (!bt_gatt_client_read_value(client, handle, ext_prop_read_cb,
							discovery_op_ref(op),
							discovery_op_unref))
		return false;

	return true;
}

static void ext_prop_read_cb(bool success, uint8_t att_ecode,
					const uint8_t *value, uint16_t length,
					void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	bool discovering;
	struct gatt_db_attribute *desc_attr = NULL;

	if (!success)
		goto done;

	util_debug(client->debug_callback, client->debug_data,
				"Ext. prop value: 0x%04x", (uint16_t)value[0]);

	desc_attr = queue_pop_head(op->ext_prop_desc);
	if (!desc_attr)
		goto failed;

	if (!gatt_db_attribute_write(desc_attr, 0, value, length, 0, NULL,
						ext_prop_write_cb, client))
		goto failed;

	/* Any other descriptor to read? */
	if (read_ext_prop_desc(op))
		return;

	if (!discover_descs(op, &discovering))
			goto failed;

	if (discovering)
		return;

	/* Done with the current service */
	gatt_db_service_set_active(op->cur_svc, true);

	goto done;

failed:
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void discover_descs_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	struct bt_gatt_iter iter;
	struct gatt_db_attribute *attr;
	uint16_t handle;
	uint128_t u128;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];
	unsigned int desc_count;
	bool discovering;
	bt_uuid_t ext_prop_uuid;

	discovery_req_clear(client);

	if (!success) {
		if (att_ecode == BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND) {
			success = true;
			goto next;
		}

		goto done;
	}

	if (!result || !bt_gatt_iter_init(&iter, result))
		goto failed;

	desc_count = bt_gatt_result_descriptor_count(result);
	if (desc_count == 0)
		goto failed;

	util_debug(client->debug_callback, client->debug_data,
					"Descriptors found: %u", desc_count);

	bt_uuid16_create(&ext_prop_uuid, GATT_CHARAC_EXT_PROPER_UUID);

	while (bt_gatt_iter_next_descriptor(&iter, &handle, u128.data)) {
		bt_uuid128_create(&uuid, u128);

		/* Log debug message */
		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		util_debug(client->debug_callback, client->debug_data,
						"handle: 0x%04x, uuid: %s",
						handle, uuid_str);

		attr = gatt_db_insert_descriptor(client->db, handle,
							&uuid, 0, NULL, NULL,
							NULL);
		if (!attr) {
			attr = gatt_db_get_attribute(client->db, handle);
			if (attr && !bt_uuid_cmp(&uuid,
					gatt_db_attribute_get_type(attr)))
				continue;

			util_debug(client->debug_callback, client->debug_data,
				"Failed to insert descriptor at 0x%04x",
				handle);
			goto failed;
		}

		if (gatt_db_attribute_get_handle(attr) != handle)
			goto failed;

		if (!bt_uuid_cmp(&ext_prop_uuid, &uuid))
			queue_push_tail(op->ext_prop_desc, attr);
	}

	/* If we got extended prop descriptor, lets read it right away */
	if (read_ext_prop_desc(op))
		return;

next:
	if (!discover_descs(op, &discovering))
		goto failed;

	if (discovering)
		return;

	/* Done with the current service */
	gatt_db_service_set_active(op->cur_svc, true);

	goto done;

failed:
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void discover_chrcs_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	struct bt_gatt_iter iter;
	struct chrc *chrc_data;
	uint16_t start, end, value;
	uint8_t properties;
	uint128_t u128;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];
	unsigned int chrc_count;
	bool discovering;

	discovery_req_clear(client);

	if (!success) {
		if (att_ecode == BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND) {
			success = true;
			goto next;
		}

		goto done;
	}

	if (!result || !bt_gatt_iter_init(&iter, result))
		goto failed;

	chrc_count = bt_gatt_result_characteristic_count(result);
	util_debug(client->debug_callback, client->debug_data,
				"Characteristics found: %u", chrc_count);

	if (chrc_count == 0)
		goto failed;

	while (bt_gatt_iter_next_characteristic(&iter, &start, &end, &value,
						&properties, u128.data)) {
		bt_uuid128_create(&uuid, u128);

		/* Log debug message */
		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		util_debug(client->debug_callback, client->debug_data,
				"start: 0x%04x, end: 0x%04x, value: 0x%04x, "
				"props: 0x%02x, uuid: %s",
				start, end, value, properties, uuid_str);

		chrc_data = new0(struct chrc, 1);

		chrc_data->start_handle = start;
		chrc_data->end_handle = end;
		chrc_data->value_handle = value;
		chrc_data->properties = properties;
		chrc_data->uuid = uuid;

		queue_push_tail(op->pending_chrcs, chrc_data);
	}

next:
	/*
	 * Before attempting to process discovered characteristics make sure we
	 * discovered all missing ranges.
	 */
	if (queue_length(op->discov_ranges)) {
		struct handle_range *range;

		range = queue_peek_head(op->discov_ranges);
		if (!range)
			goto failed;

		client->discovery_req =
			bt_gatt_discover_included_services(client->att,
							range->start,
							range->end,
							discover_incl_cb,
							discovery_op_ref(op),
							discovery_op_unref);
		if (client->discovery_req)
			return;

		util_debug(client->debug_callback, client->debug_data,
				"Failed to start included services discovery");

		discovery_op_unref(op);

		goto failed;
	}

	/*
	 * Sequentially discover descriptors for each characteristic and insert
	 * the characteristics into the database as we proceed.
	 */
	if (!discover_descs(op, &discovering))
		goto failed;

	if (discovering)
		return;

	/* Done with the current service */
	gatt_db_service_set_active(op->cur_svc, true);

	goto done;

failed:
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static bool match_handle_range(const void *data, const void *match_data)
{
	const struct handle_range *range = data;
	const struct handle_range *match_range = match_data;

	return (match_range->start >= range->start) &&
					(match_range->start <= range->end);
}

static void remove_discov_range(struct discovery_op *op, uint16_t start,
								uint16_t end)
{
	struct handle_range match_range;
	struct handle_range *range, *new_range;

	match_range.start = start;
	match_range.end = end;

	range = queue_find(op->discov_ranges, match_handle_range, &match_range);
	if (!range)
		return;

	if ((range->start == start) && (range->end == end)) {
		queue_remove(op->discov_ranges, range);
		free(range);
	} else if (range->start == start)
		range->start = end + 1;
	else if (range->end == end)
		range->end = start - 1;
	else {
		new_range = new0(struct handle_range, 1);
		new_range->start = end + 1;
		new_range->end = range->end;

		queue_push_after(op->discov_ranges, range, new_range);

		range->end = start - 1;
	}
}

static void discovery_found_service(struct discovery_op *op,
					struct gatt_db_attribute *attr,
					uint16_t start, uint16_t end)
{
	/* Skip if service already active */
	if (!gatt_db_service_get_active(attr)) {
		/* Skip if there are no attributes */
		if (end == start)
			gatt_db_service_set_active(attr, true);
		else
			queue_push_tail(op->pending_svcs, attr);

		if (start < op->svc_first)
			op->svc_first = start;
		if (end > op->svc_last)
			op->svc_last = end;
	} else {
		/* Remove from pending if active */
		queue_remove(op->pending_svcs, attr);

		remove_discov_range(op, start, end);
	}

	/* Update last handle */
	if (end > op->last)
		op->last = end;
}

static bool discovery_parse_services(struct discovery_op *op, bool primary,
						struct bt_gatt_iter *iter)
{
	struct bt_gatt_client *client = op->client;
	struct gatt_db_attribute *attr;
	uint16_t start, end;
	uint128_t u128;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];

	while (bt_gatt_iter_next_service(iter, &start, &end, u128.data)) {
		bt_uuid128_create(&uuid, u128);

		/* Log debug message */
		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		util_debug(client->debug_callback, client->debug_data,
				"start: 0x%04x, end: 0x%04x, uuid: %s",
				start, end, uuid_str);

		/* Store the service */
		attr = gatt_db_insert_service(client->db, start, &uuid, primary,
							end - start + 1);
		if (!attr) {
			gatt_db_clear_range(client->db, start, end);
			attr = gatt_db_insert_service(client->db, start, &uuid,
							false, end - start + 1);
			if (!attr) {
				util_debug(client->debug_callback,
						client->debug_data,
						"Failed to store service");
				return false;
			}
			/* Database has changed adjust last handle */
			op->last = end;
		}

		/* Update pending list */
		discovery_found_service(op, attr, start, end);
	}

	return true;
}

static void discover_secondary_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	struct bt_gatt_iter iter;
	struct handle_range *range;

	discovery_req_clear(client);

	if (!success) {
		switch (att_ecode) {
		case BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND:
		case BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE:
			success = true;
			att_ecode = 0;
			goto next;
		default:
			util_debug(client->debug_callback, client->debug_data,
					"Secondary service discovery failed."
					" ATT ECODE: 0x%02x", att_ecode);
			goto done;
		}
	}

	if (!result || !bt_gatt_iter_init(&iter, result)) {
		success = false;
		goto done;
	}

	util_debug(client->debug_callback, client->debug_data,
					"Secondary services found: %u",
					bt_gatt_result_service_count(result));

	if (!discovery_parse_services(op, false, &iter)) {
		success = false;
		goto done;
	}


next:
	if (queue_isempty(op->pending_svcs) || queue_isempty(op->discov_ranges))
		goto done;

	if (op->svc_first > 0x0001)
		remove_discov_range(op, 1, op->svc_first - 1);
	if (op->svc_last < 0xffff)
		remove_discov_range(op, op->svc_last + 1, 0xffff);

	range = queue_peek_head(op->discov_ranges);

	client->discovery_req = bt_gatt_discover_included_services(client->att,
							range->start,
							range->end,
							discover_incl_cb,
							discovery_op_ref(op),
							discovery_op_unref);
	if (client->discovery_req)
		return;

	util_debug(client->debug_callback, client->debug_data,
				"Failed to start included services discovery");
	discovery_op_unref(op);
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void discover_primary_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	struct bt_gatt_iter iter;

	discovery_req_clear(client);

	if (!success) {
		/* Reset error in case of not found */
		switch (att_ecode) {
		case BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND:
			success = true;
			att_ecode = 0;
			goto secondary;
		default:
			util_debug(client->debug_callback, client->debug_data,
					"Primary service discovery failed."
					" ATT ECODE: 0x%02x", att_ecode);
			goto done;
		}
	}

	if (!result || !bt_gatt_iter_init(&iter, result)) {
		success = false;
		goto done;
	}

	util_debug(client->debug_callback, client->debug_data,
					"Primary services found: %u",
					bt_gatt_result_service_count(result));

	if (!discovery_parse_services(op, true, &iter)) {
		success = false;
		goto done;
	}

secondary:
	/*
	 * Version 4.2 [Vol 1, Part A] page 101:
	 * A secondary service is a service that provides auxiliary
	 * functionality of a device and is referenced from at least one
	 * primary service on the device.
	 */
	if (queue_isempty(op->pending_svcs))
		goto done;

	/* Discover secondary services */
	client->discovery_req = bt_gatt_discover_secondary_services(client->att,
						NULL, op->start, op->end,
						discover_secondary_cb,
						discovery_op_ref(op),
						discovery_op_unref);
	if (client->discovery_req)
		return;

	util_debug(client->debug_callback, client->debug_data,
				"Failed to start secondary service discovery");
	discovery_op_unref(op);
	success = false;

done:
	discovery_op_complete(op, success, att_ecode);
}

static void ready_destroy(void *data)
{
	struct ready_cb *ready = data;

	if (ready->destroy)
		ready->destroy(ready->data);

	free(ready);
}

static void notify_client_ready(struct bt_gatt_client *client, bool success,
							uint8_t att_ecode)
{
	const struct queue_entry *entry;

	if (client->ready)
		return;

	bt_gatt_client_ref(client);
	client->ready = success;

	if (client->parent)
		client->features = client->parent->features;

	for (entry = queue_get_entries(client->ready_cbs); entry;
							entry = entry->next) {
		struct ready_cb *ready = entry->data;

		ready->callback(success, att_ecode, ready->data);
	}

	queue_remove_all(client->ready_cbs, NULL, NULL, ready_destroy);

	/* Notify clones */
	for (entry = queue_get_entries(client->clones); entry;
							entry = entry->next) {
		struct bt_gatt_client *clone = entry->data;

		notify_client_ready(clone, success, att_ecode);
	}

	bt_gatt_client_unref(client);
}

static void discover_all(struct discovery_op *op)
{
	struct bt_gatt_client *client = op->client;

	client->discovery_req = bt_gatt_discover_all_primary_services(
							client->att, NULL,
							discover_primary_cb,
							discovery_op_ref(op),
							discovery_op_unref);
	if (client->discovery_req)
		return;

	util_debug(client->debug_callback, client->debug_data,
			"Failed to initiate primary service discovery");

	client->in_init = false;
	notify_client_ready(client, false, BT_ATT_ERROR_UNLIKELY);

	discovery_op_unref(op);
}

static void db_hash_write_value_cb(struct gatt_db_attribute *attrib,
						int err, void *user_data)
{
	struct bt_gatt_client *client = user_data;

	util_debug(client->debug_callback, client->debug_data,
						"Value set status: %d", err);
}

static void db_hash_read_value_cb(struct gatt_db_attribute *attrib,
						int err, const uint8_t *value,
						size_t length, void *user_data)
{
	const uint8_t **hash = user_data;

	if (err || (length != 16))
		return;

	*hash = value;
}

static void db_hash_read_cb(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	const uint8_t *hash = NULL, *value;
	uint16_t len, handle;
	struct bt_gatt_iter iter;

	if (!success)
		goto discover;

	bt_gatt_iter_init(&iter, result);
	bt_gatt_iter_next_read_by_type(&iter, &handle, &len, &value);

	util_debug(client->debug_callback, client->debug_data,
				"DB Hash found: handle 0x%04x length 0x%04x",
				handle, len);

	if (len != 16)
		goto discover;

	/* Read stored value in the db */
	gatt_db_attribute_read(op->hash, 0, BT_ATT_OP_READ_REQ, NULL,
					db_hash_read_value_cb, &hash);

	/* Check if the has has changed since last time */
	if (hash && !memcmp(hash, value, len)) {
		util_debug(client->debug_callback, client->debug_data,
				"DB Hash match: skipping discovery");
		queue_remove_all(op->pending_svcs, NULL, NULL, NULL);
		discovery_op_complete(op, true, 0);
		return;
	}

	util_debug(client->debug_callback, client->debug_data,
						"DB Hash value:");
	util_hexdump(' ', value, len, client->debug_callback,
						client->debug_data);

	/* Store ithe new hash in the db */
	gatt_db_attribute_write(op->hash, 0, value, len, 0, NULL,
					db_hash_write_value_cb, client);

discover:
	if (!op->success) {
		discover_all(op);
		return;
	}

	discovery_op_complete(op, true, 0);
}

static void get_first_attribute(struct gatt_db_attribute *attrib,
								void *user_data)
{
	struct gatt_db_attribute **stored = user_data;

	if (*stored)
		return;

	*stored = attrib;
}

static bool read_db_hash(struct discovery_op *op)
{
	struct bt_gatt_client *client = op->client;
	bt_uuid_t uuid;

	/* Check if hash was already been read or there are more services to
	 * process.
	 */
	if (op->hash || !queue_isempty(client->svc_chngd_queue))
		return false;

	bt_uuid16_create(&uuid, GATT_CHARAC_DB_HASH);
	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &op->hash);
	if (!op->hash)
		return false;

	if (!bt_gatt_read_by_type(client->att, 0x0001, 0xffff, &uuid,
							db_hash_read_cb,
							discovery_op_ref(op),
							discovery_op_unref)) {
		discovery_op_unref(op);
		return false;
	}

	return true;
}

static void db_server_feat_read(bool success, uint8_t att_ecode,
				struct bt_gatt_result *result, void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;
	const uint8_t *value;
	uint16_t len, handle;
	struct bt_gatt_iter iter;

	if (!result)
		return;

	bt_gatt_iter_init(&iter, result);
	bt_gatt_iter_next_read_by_type(&iter, &handle, &len, &value);

	util_debug(client->debug_callback, client->debug_data,
				"Server Features found: handle 0x%04x "
				"length 0x%04x value 0x%02x", handle, len,
				value[0]);

	op->server_feat = value[0];
}

static void server_feat_read_value(struct gatt_db_attribute *attrib,
						int err, const uint8_t *value,
						size_t length, void *user_data)
{
	const uint8_t **feat = user_data;

	if (err)
		return;

	*feat = value;
}

static void read_server_feat(struct discovery_op *op)
{
	struct bt_gatt_client *client = op->client;
	struct gatt_db_attribute *attr = NULL;
	const uint8_t *feat = NULL;
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVER_FEAT);

	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &attr);
	if (attr) {
		/* Read stored value in the db */
		gatt_db_attribute_read(attr, 0, BT_ATT_OP_READ_REQ, NULL,
					server_feat_read_value, &feat);
		if (feat)
			return;
	}

	if (!bt_gatt_read_by_type(client->att, 0x0001, 0xffff, &uuid,
							db_server_feat_read,
							discovery_op_ref(op),
							discovery_op_unref))
		discovery_op_unref(op);
}

static void exchange_mtu_cb(bool success, uint8_t att_ecode, void *user_data)
{
	struct discovery_op *op = user_data;
	struct bt_gatt_client *client = op->client;

	op->success = success;
	client->mtu_req_id = 0;

	if (!success) {
		util_debug(client->debug_callback, client->debug_data,
				"MTU Exchange failed. ATT ECODE: 0x%02x",
				att_ecode);

		/*
		 * BLUETOOTH SPECIFICATION Version 4.2 [Vol 3, Part G] page 546
		 * If the Error Response is sent by the server with the Error
		 * Code set to RequestNot Supported , the Attribute Opcode is
		 * not supported and the default MTU shall be used.
		 */
		if (att_ecode == BT_ATT_ERROR_REQUEST_NOT_SUPPORTED)
			goto discover;

		client->in_init = false;
		notify_client_ready(client, success, att_ecode);

		return;
	}

	util_debug(client->debug_callback, client->debug_data,
					"MTU exchange complete, with MTU: %u",
					bt_att_get_mtu(client->att));

discover:
	read_server_feat(op);

	if (read_db_hash(op)) {
		op->success = false;
		return;
	}

	discover_all(op);
}

struct service_changed_op {
	struct bt_gatt_client *client;
	uint16_t start_handle;
	uint16_t end_handle;
};

static void process_service_changed(struct bt_gatt_client *client,
							uint16_t start_handle,
							uint16_t end_handle);
static void service_changed_cb(uint16_t value_handle, const uint8_t *value,
					uint16_t length, void *user_data);

static void complete_notify_request(void *data)
{
	struct notify_data *notify_data = data;

	notify_data->att_id = 0;

	if (notify_data->callback)
		notify_data->callback(notify_data->att_ecode,
						notify_data->user_data);
}

static bool notify_data_write_ccc(struct notify_data *notify_data, bool enable,
						bt_att_response_func_t callback)
{
	uint8_t pdu[4];
	unsigned int att_id;

	assert(notify_data->chrc->ccc_handle);
	memset(pdu, 0, sizeof(pdu));
	put_le16(notify_data->chrc->ccc_handle, pdu);

	if (enable) {
		/* Try to enable notifications and/or indications based on
		 * whatever the characteristic supports.
		 */
		if (notify_data->chrc->properties & BT_GATT_CHRC_PROP_NOTIFY)
			pdu[2] = 0x01;

		if (notify_data->chrc->properties & BT_GATT_CHRC_PROP_INDICATE)
			pdu[2] |= 0x02;

		if (!pdu[2])
			return false;
	}

	att_id = bt_att_send(notify_data->client->att, BT_ATT_OP_WRITE_REQ,
						pdu, sizeof(pdu), callback,
						notify_data_ref(notify_data),
						notify_data_unref);
	notify_data->chrc->ccc_write_id = notify_data->att_id = att_id;

	return !!att_id;
}

static uint8_t process_error(const void *pdu, uint16_t length)
{
	const struct bt_att_pdu_error_rsp *error_pdu;

	if (!pdu || length != sizeof(struct bt_att_pdu_error_rsp))
		return BT_ATT_ERROR_UNLIKELY;

	error_pdu = pdu;

	return error_pdu->ecode;
}

static bool notify_set_ecode(const void *data, const void *match_data)
{
	struct notify_data *notify_data = (void *)data;
	uint8_t ecode = PTR_TO_UINT(match_data);

	notify_data->att_ecode = ecode;

	return true;
}

static void enable_ccc_callback(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct notify_data *notify_data = user_data;

	assert(notify_data->chrc->ccc_write_id);

	notify_data->chrc->ccc_write_id = 0;

	bt_gatt_client_ref(notify_data->client);

	if (opcode == BT_ATT_OP_ERROR_RSP)
		notify_data->att_ecode = process_error(pdu, length);

	/* Notify for all remaining requests. */
	complete_notify_request(notify_data);
	queue_remove_all(notify_data->chrc->reg_notify_queue, notify_set_ecode,
				UINT_TO_PTR(notify_data->att_ecode),
				complete_notify_request);

	bt_gatt_client_unref(notify_data->client);
}

static bool match_notify_chrc_value_handle(const void *a, const void *b)
{
	const struct notify_chrc *chrc = a;
	uint16_t value_handle = PTR_TO_UINT(b);

	return chrc->value_handle == value_handle;
}

static unsigned int register_notify(struct bt_gatt_client *client,
				uint16_t handle,
				bt_gatt_client_register_callback_t callback,
				bt_gatt_client_notify_callback_t notify,
				void *user_data,
				bt_gatt_client_destroy_func_t destroy)
{
	struct notify_data *notify_data;
	struct notify_chrc *chrc = NULL;

	/* Check if a characteristic ref count has been started already */
	chrc = queue_find(client->notify_chrcs, match_notify_chrc_value_handle,
						UINT_TO_PTR(handle));

	if (!chrc) {
		/*
		 * Create an entry if the characteristic is known and has a CCC
		 * descriptor.
		 */
		chrc = notify_chrc_create(client, handle);
		if (!chrc)
			return 0;
	}

	/* Fail if we've hit the maximum allowed notify sessions */
	if (chrc->notify_count == INT_MAX)
		return 0;

	notify_data = new0(struct notify_data, 1);
	notify_data->client = client;
	notify_data->ref_count = 1;
	notify_data->chrc = chrc;
	notify_data->callback = callback;
	notify_data->notify = notify;
	notify_data->user_data = user_data;
	notify_data->destroy = destroy;

	/* Add the handler to the bt_gatt_client's general list */
	queue_push_tail(client->notify_list, notify_data);

	/* Assign an ID to the handler. */
	if (client->next_reg_id < 1)
		client->next_reg_id = 1;

	notify_data->id = client->next_reg_id++;

	/* Increment the per-characteristic ref count of notify handlers */
	__sync_fetch_and_add(&notify_data->chrc->notify_count, 1);

	/*
	 * If a write to the CCC descriptor is in progress, then queue this
	 * request.
	 */
	if (chrc->ccc_write_id) {
		queue_push_tail(chrc->reg_notify_queue, notify_data);
		return notify_data->id;
	}

	/*
	 * If the ref count > 1, then notifications are already enabled.
	 */
	if (chrc->notify_count > 1 || !chrc->ccc_handle) {
		complete_notify_request(notify_data);
		return notify_data->id;
	}

	/* Write to the CCC descriptor */
	if (!notify_data_write_ccc(notify_data, true, enable_ccc_callback)) {
		queue_remove(client->notify_list, notify_data);
		free(notify_data);
		return 0;
	}

	return notify_data->id;
}

static void service_changed_register_cb(uint16_t att_ecode, void *user_data)
{
	bool success;
	struct bt_gatt_client *client = user_data;

	if (att_ecode) {
		util_debug(client->debug_callback, client->debug_data,
			"Failed to register handler for \"Service Changed\"");
		success = false;
		client->svc_chngd_ind_id = 0;
		goto done;
	}

	client->svc_chngd_registered = true;
	success = true;
	util_debug(client->debug_callback, client->debug_data,
			"Registered handler for \"Service Changed\": %u",
			client->svc_chngd_ind_id);

done:
	notify_client_ready(client, success, att_ecode);
}

static bool register_service_changed(struct bt_gatt_client *client)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *attr = NULL;

	bt_uuid16_create(&uuid, SVC_CHNGD_UUID);

	if (client->svc_chngd_ind_id)
		return true;

	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &attr);
	if (!attr)
		return true;

	/*
	 * Register an indication handler for the "Service Changed"
	 * characteristic and report ready only if the handler is registered
	 * successfully.
	 */
	client->svc_chngd_ind_id = register_notify(client,
					gatt_db_attribute_get_handle(attr),
					service_changed_register_cb,
					service_changed_cb,
					client, NULL);

	return client->svc_chngd_ind_id ? true : false;
}

static void service_changed_complete(struct discovery_op *op, bool success,
							uint8_t att_ecode)
{
	struct bt_gatt_client *client = op->client;
	struct service_changed_op *next_sc_op;
	uint16_t start_handle = op->start;
	uint16_t end_handle = op->end;
	const struct queue_entry *entry;

	client->in_svc_chngd = false;

	if (!success && att_ecode != BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND) {
		util_debug(client->debug_callback, client->debug_data,
			"Failed to discover services within changed range - "
			"error: 0x%02x", att_ecode);

		gatt_db_clear_range(client->db, start_handle, end_handle);
	}

	/* Notify the upper layer of changed services */
	if (client->svc_chngd_callback)
		client->svc_chngd_callback(start_handle, end_handle,
							client->svc_chngd_data);

	/* Notify clones */
	for (entry = queue_get_entries(client->clones); entry;
							entry = entry->next) {
		struct bt_gatt_client *clone = entry->data;

		if (clone->svc_chngd_callback)
			clone->svc_chngd_callback(start_handle, end_handle,
							clone->svc_chngd_data);
	}

	/* Process any queued events */
	next_sc_op = queue_pop_head(client->svc_chngd_queue);
	if (next_sc_op) {
		process_service_changed(client, next_sc_op->start_handle,
							next_sc_op->end_handle);
		free(next_sc_op);
		return;
	}

	if (register_service_changed(client))
		return;

	util_debug(client->debug_callback, client->debug_data,
		"Failed to re-register handler for \"Service Changed\"");
}

static void service_changed_failure(struct discovery_op *op)
{
	struct bt_gatt_client *client = op->client;

	gatt_db_clear_range(client->db, op->start, op->end);
}

static void process_service_changed(struct bt_gatt_client *client,
							uint16_t start_handle,
							uint16_t end_handle)
{
	struct discovery_op *op;

	op = discovery_op_create(client, start_handle, end_handle,
						service_changed_complete,
						service_changed_failure);
	if (!op)
		goto fail;

	client->discovery_req = bt_gatt_discover_primary_services(client->att,
						NULL, start_handle, end_handle,
						discover_primary_cb,
						discovery_op_ref(op),
						discovery_op_unref);
	if (client->discovery_req) {
		client->in_svc_chngd = true;
		return;
	}

	discovery_op_free(op);

fail:
	util_debug(client->debug_callback, client->debug_data,
					"Failed to initiate service discovery"
					" after Service Changed");
}

static void service_changed_cb(uint16_t value_handle, const uint8_t *value,
					uint16_t length, void *user_data)
{
	struct bt_gatt_client *client = user_data;
	struct service_changed_op *op;
	uint16_t start, end;

	if (length != 4)
		return;

	start = get_le16(value);
	end = get_le16(value + 2);

	if (start > end) {
		util_debug(client->debug_callback, client->debug_data,
			"Service Changed received with invalid handles");
		return;
	}

	util_debug(client->debug_callback, client->debug_data,
			"Service Changed received - start: 0x%04x end: 0x%04x",
			start, end);

	if (!client->in_svc_chngd) {
		process_service_changed(client, start, end);
		return;
	}

	op = new0(struct service_changed_op, 1);

	op->start_handle = start;
	op->end_handle = end;

	queue_push_tail(client->svc_chngd_queue, op);
}

static void server_feat_write_value(struct gatt_db_attribute *attrib,
						int err, void *user_data)
{
	struct bt_gatt_client *client = user_data;

	util_debug(client->debug_callback, client->debug_data,
			"Server Features Value set status: %d", err);
}

static void write_server_features(struct bt_gatt_client *client, uint8_t feat)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *attr = NULL;

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVER_FEAT);

	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &attr);
	if (!attr)
		return;

	/* Store value in the DB */
	if (!gatt_db_attribute_write(attr, 0, &feat, sizeof(feat),
					0, NULL, server_feat_write_value,
					client))
		util_debug(client->debug_callback, client->debug_data,
					"Unable to store Server Features");
}

static void write_client_features(struct bt_gatt_client *client)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *attr = NULL;
	uint16_t handle;
	const uint8_t *feat = NULL;

	bt_uuid16_create(&uuid, GATT_CHARAC_CLI_FEAT);

	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &attr);
	if (!attr)
		return;

	handle = gatt_db_attribute_get_handle(attr);

	client->features = BT_GATT_CHRC_CLI_FEAT_ROBUST_CACHING;

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVER_FEAT);

	attr = NULL;
	gatt_db_find_by_type(client->db, 0x0001, 0xffff, &uuid,
						get_first_attribute, &attr);
	if (attr) {
		/* Read stored value in the db */
		gatt_db_attribute_read(attr, 0, BT_ATT_OP_READ_REQ,
						NULL, server_feat_read_value,
						&feat);
		if (feat && feat[0] & BT_GATT_CHRC_SERVER_FEAT_EATT)
			client->features |= BT_GATT_CHRC_CLI_FEAT_EATT;
	}

	client->features |= BT_GATT_CHRC_CLI_FEAT_NFY_MULTI;

	util_debug(client->debug_callback, client->debug_data,
			"Writing Client Features 0x%02x", client->features);

	bt_gatt_client_write_value(client, handle, &client->features,
				sizeof(client->features), NULL, NULL, NULL);
}

static void init_complete(struct discovery_op *op, bool success,
							uint8_t att_ecode)
{
	struct bt_gatt_client *client = op->client;

	client->in_init = false;

	if (!success)
		goto fail;

	if (op->server_feat)
		write_server_features(client, op->server_feat);

	write_client_features(client);

	if (register_service_changed(client))
		goto done;

	util_debug(client->debug_callback, client->debug_data,
			"Failed to register handler for \"Service Changed\"");
	success = false;

fail:
	util_debug(client->debug_callback, client->debug_data,
			"Failed to initialize gatt-client");

	op->success = false;

done:
	notify_client_ready(client, success, att_ecode);
}

static bool gatt_client_init(struct bt_gatt_client *client, uint16_t mtu)
{
	struct discovery_op *op;

	if (client->in_init || client->ready)
		return false;

	op = discovery_op_create(client, 0x0001, 0xffff, init_complete, NULL);
	if (!op)
		return false;

	/*
	 * BLUETOOTH SPECIFICATION Version 4.2 [Vol 3, Part G] page 546:
	 *
	 * 4.3.1 Exchange MTU
	 *
	 * This sub-procedure shall not be used on a BR/EDR physical link since
	 * the MTU size is negotiated using L2CAP channel configuration
	 * procedures.
	 */
	if (bt_att_get_link_type(client->att) == BT_ATT_BREDR)
		goto discover;

	/* Check if MTU needs to be send */
	mtu = MAX(BT_ATT_DEFAULT_LE_MTU, mtu);
	if (mtu == BT_ATT_DEFAULT_LE_MTU)
		goto discover;

	/* Configure the MTU */
	client->mtu_req_id = bt_gatt_exchange_mtu(client->att, mtu,
						exchange_mtu_cb,
						discovery_op_ref(op),
						discovery_op_unref);
	if (!client->mtu_req_id) {
		discovery_op_free(op);
		return false;
	}

	client->in_init = true;

	return true;

discover:
	read_server_feat(op);

	if (read_db_hash(op)) {
		op->success = false;
		goto done;
	}

	client->discovery_req = bt_gatt_discover_all_primary_services(
							client->att, NULL,
							discover_primary_cb,
							discovery_op_ref(op),
							discovery_op_unref);
	if (!client->discovery_req) {
		discovery_op_free(op);
		return false;
	}

done:
	client->in_init = true;
	return true;
}

struct value_data {
	uint16_t handle;
	uint16_t len;
	const void *data;
};

static void disable_ccc_callback(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct notify_data *notify_data = user_data;
	struct notify_data *next_data;

	assert(notify_data->chrc->ccc_write_id);

	notify_data->chrc->ccc_write_id = 0;

	/* This is a best effort procedure, so ignore errors and process any
	 * queued requests.
	 */
	while (1) {
		next_data = queue_pop_head(notify_data->chrc->reg_notify_queue);
		if (!next_data || notify_data_write_ccc(next_data, true,
							enable_ccc_callback))
			return;
	}
}

static void complete_unregister_notify(void *data)
{
	struct notify_data *notify_data = data;

	/*
	 * If a procedure to enable the CCC is still pending, then cancel it and
	 * return.
	 */
	if (notify_data->att_id) {
		bt_att_cancel(notify_data->client->att, notify_data->att_id);
		notify_data->att_id = 0;
		goto done;
	}

	if (__sync_sub_and_fetch(&notify_data->chrc->notify_count, 1) ||
						!notify_data->chrc->ccc_handle)
		goto done;

	notify_data_write_ccc(notify_data, false, disable_ccc_callback);

done:
	notify_data_unref(notify_data);
}

static void notify_handler(void *data, void *user_data)
{
	struct notify_data *notify_data = data;
	struct value_data *value_data = user_data;

	if (notify_data->chrc->value_handle != value_data->handle)
		return;

	/*
	 * Even if the notify data has a pending ATT request to write to the
	 * CCC, there is really no reason not to notify the handlers.
	 */
	if (notify_data->notify)
		notify_data->notify(value_data->handle, value_data->data,
				value_data->len, notify_data->user_data);
}

static void notify_cb(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t length,
					void *user_data)
{
	struct bt_gatt_client *client = user_data;
	struct value_data data;

	bt_gatt_client_ref(client);

	memset(&data, 0, sizeof(data));

	if (opcode == BT_ATT_OP_HANDLE_NFY_MULT) {
		while (length >= 4) {
			data.handle = get_le16(pdu);
			length -= 2;
			pdu += 2;

			data.len = get_le16(pdu);
			length -= 2;
			pdu += 2;

			data.data = pdu;

			queue_foreach(client->notify_list, notify_handler,
								&data);

			length -= data.len;
		}
	} else {
		data.handle = get_le16(pdu);
		length -= 2;
		pdu += 2;

		data.len = length;
		data.data = pdu;

		queue_foreach(client->notify_list, notify_handler, &data);
	}

	if (opcode == BT_ATT_OP_HANDLE_IND && !client->parent)
		bt_att_chan_send(chan, BT_ATT_OP_HANDLE_CONF, NULL, 0,
							NULL, NULL, NULL);

	bt_gatt_client_unref(client);
}

static void bt_gatt_client_free(struct bt_gatt_client *client)
{
	bt_gatt_client_cancel_all(client);

	queue_destroy(client->notify_chrcs, notify_chrc_free);
	queue_destroy(client->notify_list, notify_data_cleanup);

	queue_destroy(client->ready_cbs, ready_destroy);

	if (client->debug_destroy)
		client->debug_destroy(client->debug_data);

	if (client->att) {
		bt_att_unregister_disconnect(client->att, client->disc_id);
		bt_att_unregister(client->att, client->nfy_id);
		bt_att_unregister(client->att, client->nfy_mult_id);
		bt_att_unregister(client->att, client->ind_id);
		bt_att_unref(client->att);
	}

	gatt_db_unref(client->db);

	queue_destroy(client->clones, NULL);
	queue_destroy(client->svc_chngd_queue, free);
	queue_destroy(client->long_write_queue, request_unref);
	queue_destroy(client->pending_requests, request_unref);

	if (client->parent) {
		queue_remove(client->parent->clones, client);
		bt_gatt_client_unref(client->parent);
	}

	free(client);
}

static void att_disconnect_cb(int err, void *user_data)
{
	struct bt_gatt_client *client = user_data;
	bool in_init = client->in_init;

	client->disc_id = 0;

	bt_att_unref(client->att);
	client->att = NULL;

	client->in_init = false;
	client->ready = false;

	if (in_init)
		notify_client_ready(client, false, 0);
}

static struct bt_gatt_client *gatt_client_new(struct gatt_db *db,
							struct bt_att *att,
							uint8_t features)
{
	struct bt_gatt_client *client;

	client = new0(struct bt_gatt_client, 1);
	client->disc_id = bt_att_register_disconnect(att, att_disconnect_cb,
								client, NULL);
	if (!client->disc_id)
		goto fail;

	client->clones = queue_new();
	client->ready_cbs = queue_new();
	client->long_write_queue = queue_new();
	client->svc_chngd_queue = queue_new();
	client->notify_list = queue_new();
	client->notify_chrcs = queue_new();
	client->pending_requests = queue_new();

	client->nfy_id = bt_att_register(att, BT_ATT_OP_HANDLE_NFY,
						notify_cb, client, NULL);
	if (!client->nfy_id)
		goto fail;

	client->nfy_mult_id = bt_att_register(att, BT_ATT_OP_HANDLE_NFY_MULT,
						notify_cb, client, NULL);
	if (!client->nfy_mult_id)
		goto fail;

	client->ind_id = bt_att_register(att, BT_ATT_OP_HANDLE_IND,
						notify_cb, client, NULL);
	if (!client->ind_id)
		goto fail;

	client->att = bt_att_ref(att);
	client->db = gatt_db_ref(db);
	client->features = features;

	return client;

fail:
	bt_gatt_client_free(client);
	return NULL;

}

struct bt_gatt_client *bt_gatt_client_new(struct gatt_db *db,
							struct bt_att *att,
							uint16_t mtu,
							uint8_t features)
{
	struct bt_gatt_client *client;

	if (!att || !db)
		return NULL;

	client = gatt_client_new(db, att, features);
	if (!client)
		return NULL;

	if (!gatt_client_init(client, mtu)) {
		bt_gatt_client_free(client);
		return NULL;
	}

	return bt_gatt_client_ref(client);
}

struct bt_gatt_client *bt_gatt_client_clone(struct bt_gatt_client *client)
{
	struct bt_gatt_client *clone;

	if (!client)
		return NULL;

	clone = gatt_client_new(client->db, client->att, client->features);
	if (!clone)
		return NULL;

	queue_push_tail(client->clones, clone);

	/*
	 * Reference the parent since the clones depend on it to propagate
	 * service changed and ready callbacks.
	 */
	clone->parent = bt_gatt_client_ref(client);
	clone->ready = client->ready;

	return bt_gatt_client_ref(clone);
}

struct bt_gatt_client *bt_gatt_client_ref(struct bt_gatt_client *client)
{
	if (!client)
		return NULL;

	__sync_fetch_and_add(&client->ref_count, 1);

	return client;
}

void bt_gatt_client_unref(struct bt_gatt_client *client)
{
	if (!client)
		return;

	if (__sync_sub_and_fetch(&client->ref_count, 1))
		return;

	bt_gatt_client_free(client);
}

bool bt_gatt_client_is_ready(struct bt_gatt_client *client)
{
	return (client && client->ready);
}

unsigned int bt_gatt_client_ready_register(struct bt_gatt_client *client,
					bt_gatt_client_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	struct ready_cb *ready;

	if (!client)
		return 0;

	ready = new0(struct ready_cb, 1);
	ready->callback = callback;
	ready->destroy = destroy;
	ready->data = user_data;

	queue_push_tail(client->ready_cbs, ready);

	return PTR_TO_UINT(ready);
}

bool bt_gatt_client_ready_unregister(struct bt_gatt_client *client,
						unsigned int id)
{
	struct ready_cb *ready = UINT_TO_PTR(id);

	if (queue_remove(client->ready_cbs, ready)) {
		ready_destroy(ready);
		return true;
	}

	return false;
}

bool bt_gatt_client_set_service_changed(struct bt_gatt_client *client,
			bt_gatt_client_service_changed_callback_t callback,
			void *user_data,
			bt_gatt_client_destroy_func_t destroy)
{
	if (!client)
		return false;

	if (client->svc_chngd_destroy)
		client->svc_chngd_destroy(client->svc_chngd_data);

	client->svc_chngd_callback = callback;
	client->svc_chngd_destroy = destroy;
	client->svc_chngd_data = user_data;

	return true;
}

bool bt_gatt_client_set_debug(struct bt_gatt_client *client,
					bt_gatt_client_debug_func_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy) {
	if (!client)
		return false;

	if (client->debug_destroy)
		client->debug_destroy(client->debug_data);

	client->debug_callback = callback;
	client->debug_destroy = destroy;
	client->debug_data = user_data;

	return true;
}

uint16_t bt_gatt_client_get_mtu(struct bt_gatt_client *client)
{
	if (!client || !client->att)
		return 0;

	return bt_att_get_mtu(client->att);
}

struct bt_att *bt_gatt_client_get_att(struct bt_gatt_client *client)
{
	if (!client)
		return NULL;

	return client->att;
}

struct gatt_db *bt_gatt_client_get_db(struct bt_gatt_client *client)
{
	if (!client || !client->db)
		return NULL;

	return client->db;
}

uint8_t bt_gatt_client_get_features(struct bt_gatt_client *client)
{
	if (!client)
		return 0;

	if (client->parent)
		return client->parent->features;

	return client->features;
}

static bool match_req_id(const void *a, const void *b)
{
	const struct request *req = a;
	unsigned int id = PTR_TO_UINT(b);

	return req->id == id;
}

static void cancel_long_write_cb(uint8_t opcode, const void *pdu, uint16_t len,
								void *user_data)
{
	struct bt_gatt_client *client = user_data;

	if (queue_isempty(client->long_write_queue))
		client->in_long_write = false;
}

static bool cancel_long_write_req(struct bt_gatt_client *client,
							struct request *req)
{
	uint8_t pdu = 0x00;

	/*
	 * att_id == 0 means that request has been queued and no prepare write
	 * has been sent so far.Let's just remove if from the queue.
	 * Otherwise execute write needs to be send.
	 */
	if (!req->att_id)
		return queue_remove(client->long_write_queue, req);

	return !!bt_att_send(client->att, BT_ATT_OP_EXEC_WRITE_REQ, &pdu,
							sizeof(pdu),
							cancel_long_write_cb,
							client, NULL);

}

static void cancel_prep_write_cb(uint8_t opcode, const void *pdu, uint16_t len,
								void *user_data)
{
	struct request *req = user_data;
	struct bt_gatt_client *client = req->client;

	client->reliable_write_session_id = 0;
}

static bool cancel_prep_write_session(struct bt_gatt_client *client,
							struct request *req)
{
	uint8_t pdu = 0x00;

	return !!bt_att_send(client->att, BT_ATT_OP_EXEC_WRITE_REQ, &pdu,
							sizeof(pdu),
							cancel_prep_write_cb,
							req, request_unref);
}

static bool cancel_request(struct request *req)
{
	req->removed = true;

	if (req->long_write)
		return cancel_long_write_req(req->client, req);

	if (req->prep_write)
		return cancel_prep_write_session(req->client, req);

	return bt_att_cancel(req->client->att, req->att_id);
}

bool bt_gatt_client_cancel(struct bt_gatt_client *client, unsigned int id)
{
	struct request *req;

	if (!client || !id || !client->att)
		return false;

	req = queue_remove_if(client->pending_requests, match_req_id,
							UINT_TO_PTR(id));
	if (!req)
		return false;

	return cancel_request(req);
}

static void cancel_pending(void *data)
{
	cancel_request(data);
}

bool bt_gatt_client_cancel_all(struct bt_gatt_client *client)
{
	if (!client || !client->att)
		return false;

	queue_remove_all(client->pending_requests, NULL, NULL, cancel_pending);

	if (client->discovery_req) {
		bt_gatt_request_cancel(client->discovery_req);
		bt_gatt_request_unref(client->discovery_req);
		client->discovery_req = NULL;
	}

	if (client->mtu_req_id)
		bt_att_cancel(client->att, client->mtu_req_id);

	return true;
}

struct read_op {
	bt_gatt_client_read_callback_t callback;
	void *user_data;
	bt_gatt_client_destroy_func_t destroy;
};

static void destroy_read_op(void *data)
{
	struct read_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op);
}

static void read_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct read_op *op = req->data;
	bool success;
	uint8_t att_ecode = 0;
	const uint8_t *value = NULL;
	uint16_t value_len = 0;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_READ_RSP || (!pdu && length)) {
		success = false;
		goto done;
	}

	success = true;
	value_len = length;
	if (value_len)
		value = pdu;

done:
	if (op->callback)
		op->callback(success, att_ecode, value, length, op->user_data);
}

unsigned int bt_gatt_client_read_value(struct bt_gatt_client *client,
					uint16_t value_handle,
					bt_gatt_client_read_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct read_op *op;
	uint8_t pdu[2];

	if (!client)
		return 0;

	op = new0(struct read_op, 1);

	req = request_create(client);
	if (!req) {
		free(op);
		return 0;
	}

	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->data = op;
	req->destroy = destroy_read_op;

	put_le16(value_handle, pdu);

	req->att_id = bt_att_send(client->att, BT_ATT_OP_READ_REQ,
							pdu, sizeof(pdu),
							read_cb, req,
							request_unref);
	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	return req->id;
}

static void read_multiple_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct read_op *op = req->data;
	uint8_t att_ecode;
	bool success;

	if ((opcode != BT_ATT_OP_READ_MULT_RSP &&
			opcode != BT_ATT_OP_READ_MULT_VL_RSP) ||
			(!pdu && length)) {
		success = false;

		if (opcode == BT_ATT_OP_ERROR_RSP)
			att_ecode = process_error(pdu, length);
		else
			att_ecode = 0;

		pdu = NULL;
		length = 0;
	} else {
		success = true;
		att_ecode = 0;
	}

	if (!op->callback)
		return;

	if (opcode == BT_ATT_OP_READ_MULT_RSP || att_ecode) {
		op->callback(success, att_ecode, pdu, length, op->user_data);
		return;
	}

	if (length < 2) {
		op->callback(success, att_ecode, pdu, length, op->user_data);
		return;
	}

	/* Parse response */
	while (length >= 2) {
		uint16_t len;

		len = get_le16(pdu);
		length -= 2;
		pdu += 2;

		/* The Length Value Tuple List may be truncated within the
		 * first two octets of a tuple due to the size limits of the
		 * current ATT_MTU.
		 */
		if (len > length)
			length = len;

		op->callback(success, att_ecode, pdu, len, op->user_data);
	}
}

unsigned int bt_gatt_client_read_multiple(struct bt_gatt_client *client,
					uint16_t *handles, uint8_t num_handles,
					bt_gatt_client_read_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	uint8_t pdu[num_handles * 2];
	struct request *req;
	struct read_op *op;
	uint8_t opcode;
	int i;

	if (!client)
		return 0;

	if (num_handles < 2)
		return 0;

	if (num_handles * 2 > bt_att_get_mtu(client->att) - 1)
		return 0;

	op = new0(struct read_op, 1);

	req = request_create(client);
	if (!req) {
		free(op);
		return 0;
	}

	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->data = op;
	req->destroy = destroy_read_op;

	for (i = 0; i < num_handles; i++)
		put_le16(handles[i], pdu + (2 * i));

	opcode = bt_gatt_client_get_features(client) &
		BT_GATT_CHRC_CLI_FEAT_EATT ? BT_ATT_OP_READ_MULT_VL_REQ :
		BT_ATT_OP_READ_MULT_REQ;

	req->att_id = bt_att_send(client->att, opcode, pdu, sizeof(pdu),
							read_multiple_cb, req,
							request_unref);
	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	return req->id;
}

struct read_long_op {
	struct bt_gatt_client *client;
	int ref_count;
	uint16_t value_handle;
	uint16_t offset;
	struct iovec iov;
	bt_gatt_client_read_callback_t callback;
	void *user_data;
	bt_gatt_client_destroy_func_t destroy;
};

static void destroy_read_long_op(void *data)
{
	struct read_long_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op->iov.iov_base);
	free(op);
}

static bool append_chunk(struct read_long_op *op, const uint8_t *data,
								uint16_t len)
{
	void *buf;

	/* Truncate if the data would exceed maximum length */
	if (op->offset + len > BT_ATT_MAX_VALUE_LEN)
		len = BT_ATT_MAX_VALUE_LEN - op->offset;

	buf = realloc(op->iov.iov_base, op->iov.iov_len + len);
	if (!buf)
		return false;

	op->iov.iov_base = buf;

	memcpy(op->iov.iov_base + op->iov.iov_len, data, len);

	op->iov.iov_len += len;
	op->offset += len;

	return true;
}

static void read_long_cb(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data)
{
	struct request *req = user_data;
	struct read_long_op *op = req->data;
	bool success;
	uint8_t att_ecode = 0;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if ((!op->offset && opcode != BT_ATT_OP_READ_RSP)
			|| (op->offset && opcode != BT_ATT_OP_READ_BLOB_RSP)
			|| (!pdu && length)) {
		success = false;
		goto done;
	}

	if (!length)
		goto success;

	if (!append_chunk(op, pdu, length)) {
		success = false;
		goto done;
	}

	if (op->offset >= BT_ATT_MAX_VALUE_LEN)
		goto success;

	if (length >= bt_att_get_mtu(op->client->att) - 1) {
		uint8_t pdu[4];

		put_le16(op->value_handle, pdu);
		put_le16(op->offset, pdu + 2);

		req->att_id = bt_att_send(op->client->att,
							BT_ATT_OP_READ_BLOB_REQ,
							pdu, sizeof(pdu),
							read_long_cb,
							request_ref(req),
							request_unref);
		if (req->att_id)
			return;

		request_unref(req);
		success = false;
		goto done;
	}

success:
	success = true;

done:
	if (op->callback)
		op->callback(success, att_ecode, op->iov.iov_base,
						op->iov.iov_len, op->user_data);
}

unsigned int bt_gatt_client_read_long_value(struct bt_gatt_client *client,
					uint16_t value_handle, uint16_t offset,
					bt_gatt_client_read_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct read_long_op *op;
	uint8_t att_op;
	uint8_t pdu[4];
	uint16_t pdu_len;

	if (!client)
		return 0;

	op = new0(struct read_long_op, 1);

	req = request_create(client);
	if (!req) {
		free(op);
		return 0;
	}

	op->client = client;
	op->value_handle = value_handle;
	op->offset = offset;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->data = op;
	req->destroy = destroy_read_long_op;

	put_le16(value_handle, pdu);
	pdu_len = sizeof(value_handle);

	/*
	 * Core v4.2, part F, section 1.3.4.4.5:
	 * If the attribute value has a fixed length that is less than or equal
	 * to (ATT_MTU - 3) octets in length, then an Error Response can be sent
	 * with the error code «Attribute Not Long».
	 *
	 * To remove need for caller to handle "Attribute Not Long" error when
	 * reading characteristics with short values, use Read Request for
	 * reading first part of characteristics value instead of Read Blob
	 * Request. Both are allowed in this case.
	 */

	if (op->offset) {
		att_op = BT_ATT_OP_READ_BLOB_REQ;
		pdu_len += sizeof(op->offset);

		put_le16(op->offset, pdu + 2);
	} else {
		att_op = BT_ATT_OP_READ_REQ;
	}

	req->att_id = bt_att_send(client->att, att_op, pdu, pdu_len,
					read_long_cb, req, request_unref);

	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	return req->id;
}

unsigned int bt_gatt_client_write_without_response(
					struct bt_gatt_client *client,
					uint16_t value_handle,
					bool signed_write,
					const uint8_t *value, uint16_t length) {
	uint8_t pdu[2 + length];
	struct request *req;
	int security;
	uint8_t op;

	if (!client)
		return 0;

	req = request_create(client);
	if (!req)
		return 0;

	/* Only use signed write if unencrypted */
	if (signed_write) {
		security = bt_att_get_security(client->att, NULL);
		op = security > BT_SECURITY_LOW ?  BT_ATT_OP_WRITE_CMD :
						BT_ATT_OP_SIGNED_WRITE_CMD;
	} else
		op = BT_ATT_OP_WRITE_CMD;

	put_le16(value_handle, pdu);
	memcpy(pdu + 2, value, length);

	req->att_id = bt_att_send(client->att, op, pdu, sizeof(pdu), NULL, req,
								request_unref);
	if (!req->att_id) {
		request_unref(req);
		return 0;
	}

	return req->id;
}

struct write_op {
	struct bt_gatt_client *client;
	bt_gatt_client_callback_t callback;
	void *user_data;
	bt_gatt_destroy_func_t destroy;
};

static void destroy_write_op(void *data)
{
	struct write_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op);
}

static void write_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct write_op *op = req->data;
	bool success = true;
	uint8_t att_ecode = 0;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_WRITE_RSP || pdu || length)
		success = false;

done:
	if (op->callback)
		op->callback(success, att_ecode, op->user_data);
}

unsigned int bt_gatt_client_write_value(struct bt_gatt_client *client,
					uint16_t value_handle,
					const uint8_t *value, uint16_t length,
					bt_gatt_client_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct write_op *op;
	uint8_t pdu[2 + length];

	if (!client)
		return 0;

	op = new0(struct write_op, 1);

	req = request_create(client);
	if (!req) {
		free(op);
		return 0;
	}

	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->data = op;
	req->destroy = destroy_write_op;

	put_le16(value_handle, pdu);
	memcpy(pdu + 2, value, length);

	req->att_id = bt_att_send(client->att, BT_ATT_OP_WRITE_REQ,
							pdu, sizeof(pdu),
							write_cb, req,
							request_unref);
	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	return req->id;
}

struct long_write_op {
	struct bt_gatt_client *client;
	bool reliable;
	bool success;
	uint8_t att_ecode;
	bool reliable_error;
	uint16_t value_handle;
	uint8_t *value;
	uint16_t length;
	uint16_t offset;
	uint16_t index;
	uint16_t cur_length;
	bt_gatt_client_write_long_callback_t callback;
	void *user_data;
	bt_gatt_client_destroy_func_t destroy;
};

static void long_write_op_free(void *data)
{
	struct long_write_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op->value);
	free(op);
}

static void prepare_write_cb(uint8_t opcode, const void *pdu, uint16_t length,
							void *user_data);
static void complete_write_long_op(struct request *req, bool success,
					uint8_t att_ecode, bool reliable_error);

static void handle_next_prep_write(struct request *req)
{
	struct long_write_op *op = req->data;
	bool success = true;
	uint8_t *pdu;

	pdu = malloc(op->cur_length + 4);
	if (!pdu) {
		success = false;
		goto done;
	}

	put_le16(op->value_handle, pdu);
	put_le16(op->offset + op->index, pdu + 2);
	memcpy(pdu + 4, op->value + op->index, op->cur_length);

	req->att_id = bt_att_send(op->client->att, BT_ATT_OP_PREP_WRITE_REQ,
							pdu, op->cur_length + 4,
							prepare_write_cb,
							request_ref(req),
							request_unref);
	if (!req->att_id) {
		request_unref(req);
		success = false;
	}

	free(pdu);

	/* If so far successful, then the operation should continue.
	 * Otherwise, there was an error and the procedure should be
	 * completed.
	 */
	if (success)
		return;

done:
	complete_write_long_op(req, success, 0, false);
}

static void start_next_long_write(struct bt_gatt_client *client)
{
	struct request *req;

	if (queue_isempty(client->long_write_queue)) {
		client->in_long_write = false;
		return;
	}

	req  = queue_pop_head(client->long_write_queue);
	if (!req)
		return;

	handle_next_prep_write(req);

	/*
	 * send_next_prep_write adds an extra ref. Unref here to clean up if
	 * necessary, since we also added a ref before pushing to the queue.
	 */
	request_unref(req);
}

static void execute_write_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct long_write_op *op = req->data;
	bool success = op->success;
	uint8_t att_ecode = op->att_ecode;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
	} else if (opcode != BT_ATT_OP_EXEC_WRITE_RSP || pdu || length)
		success = false;

	bt_gatt_client_ref(op->client);

	if (op->callback)
		op->callback(success, op->reliable_error, att_ecode,
								op->user_data);

	start_next_long_write(op->client);

	bt_gatt_client_unref(op->client);
}

static void complete_write_long_op(struct request *req, bool success,
					uint8_t att_ecode, bool reliable_error)
{
	struct long_write_op *op = req->data;
	uint8_t pdu;

	op->success = success;
	op->att_ecode = att_ecode;
	op->reliable_error = reliable_error;

	if (success)
		pdu = 0x01;  /* Write */
	else
		pdu = 0x00;  /* Cancel */

	req->att_id = bt_att_send(op->client->att, BT_ATT_OP_EXEC_WRITE_REQ,
							&pdu, sizeof(pdu),
							execute_write_cb,
							request_ref(req),
							request_unref);
	if (req->att_id)
		return;

	request_unref(req);
	success = false;

	bt_gatt_client_ref(op->client);

	if (op->callback)
		op->callback(success, reliable_error, att_ecode, op->user_data);

	start_next_long_write(op->client);

	bt_gatt_client_unref(op->client);
}

static void prepare_write_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct long_write_op *op = req->data;
	bool success = true;
	bool reliable_error = false;
	uint8_t att_ecode = 0;
	uint16_t next_index;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_PREP_WRITE_RSP) {
		success = false;
		goto done;
	}

	if (op->reliable) {
		if (!pdu || length != (op->cur_length + 4)) {
			success = false;
			reliable_error = true;
			goto done;
		}

		if (get_le16(pdu) != op->value_handle ||
				get_le16(pdu + 2) != (op->offset + op->index)) {
			success = false;
			reliable_error = true;
			goto done;
		}

		if (memcmp(pdu + 4, op->value + op->index, op->cur_length)) {
			success = false;
			reliable_error = true;
			goto done;
		}
	}

	next_index = op->index + op->cur_length;
	if (next_index == op->length) {
		/* All bytes written */
		goto done;
	}

	/* If the last written length was greater than or equal to what can fit
	 * inside a PDU, then there is more data to send.
	 */
	if (op->cur_length >= bt_att_get_mtu(op->client->att) - 5) {
		op->index = next_index;
		op->cur_length = MIN(op->length - op->index,
					bt_att_get_mtu(op->client->att) - 5);
		handle_next_prep_write(req);
		return;
	}

done:
	complete_write_long_op(req, success, att_ecode, reliable_error);
}

unsigned int bt_gatt_client_write_long_value(struct bt_gatt_client *client,
				bool reliable,
				uint16_t value_handle, uint16_t offset,
				const uint8_t *value, uint16_t length,
				bt_gatt_client_write_long_callback_t callback,
				void *user_data,
				bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct long_write_op *op;
	uint8_t *pdu;

	if (!client)
		return 0;

	if ((size_t)(length + offset) > UINT16_MAX)
		return 0;

	/* Don't allow writing a 0-length value using this procedure. The
	 * upper-layer should use bt_gatt_write_value for that instead.
	 */
	if (!length || !value)
		return 0;

	op = new0(struct long_write_op, 1);
	op->value = malloc(length);
	if (!op->value) {
		free(op);
		return 0;
	}

	req = request_create(client);
	if (!req) {
		free(op->value);
		free(op);
		return 0;
	}

	memcpy(op->value, value, length);

	op->client = client;
	op->reliable = reliable;
	op->value_handle = value_handle;
	op->length = length;
	op->offset = offset;
	op->cur_length = MIN(length, bt_att_get_mtu(client->att) - 5);
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->data = op;
	req->destroy = long_write_op_free;
	req->long_write = true;

	if (client->in_long_write || client->reliable_write_session_id > 0) {
		queue_push_tail(client->long_write_queue, req);
		return req->id;
	}

	pdu = malloc(op->cur_length + 4);
	if (!pdu) {
		free(op->value);
		free(op);
		return 0;
	}

	put_le16(value_handle, pdu);
	put_le16(offset, pdu + 2);
	memcpy(pdu + 4, op->value, op->cur_length);

	req->att_id = bt_att_send(client->att, BT_ATT_OP_PREP_WRITE_REQ,
							pdu, op->cur_length + 4,
							prepare_write_cb, req,
							request_unref);
	free(pdu);

	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	client->in_long_write = true;

	return req->id;
}

struct prep_write_op {
	bt_gatt_client_write_long_callback_t callback;
	void *user_data;
	bt_gatt_destroy_func_t destroy;
	uint8_t *pdu;
	uint16_t pdu_len;
};

static void destroy_prep_write_op(void *data)
{
	struct prep_write_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	free(op->pdu);
	free(op);
}

static void prep_write_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct prep_write_op *op = req->data;
	bool success;
	uint8_t att_ecode;
	bool reliable_error;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		reliable_error = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_PREP_WRITE_RSP) {
		success = false;
		reliable_error = false;
		att_ecode = 0;
		goto done;
	}

	if (!pdu || length != op->pdu_len ||
					memcmp(pdu, op->pdu, op->pdu_len)) {
		success = false;
		reliable_error = true;
		att_ecode = 0;
		goto done;
	}

	success = true;
	reliable_error = false;
	att_ecode = 0;

done:
	if (op->callback)
		op->callback(success, reliable_error, att_ecode, op->user_data);
}

static struct request *get_reliable_request(struct bt_gatt_client *client,
							unsigned int id)
{
	struct request *req;
	struct prep_write_op *op;

	op = new0(struct prep_write_op, 1);

	/* Following prepare writes */
	if (id != 0)
		req = queue_find(client->pending_requests, match_req_id,
							UINT_TO_PTR(id));
	else
		req = request_create(client);

	if (!req) {
		free(op);
		return NULL;
	}

	req->data = op;

	return req;
}

unsigned int bt_gatt_client_prepare_write(struct bt_gatt_client *client,
				unsigned int id, uint16_t value_handle,
				uint16_t offset, const uint8_t *value,
				uint16_t length,
				bt_gatt_client_write_long_callback_t callback,
				void *user_data,
				bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct prep_write_op *op;
	uint8_t pdu[4 + length];

	if (!client)
		return 0;

	if (client->in_long_write)
		return 0;

	/*
	 * Make sure that client who owns reliable session continues with
	 * prepare writes or this is brand new reliable session (id == 0)
	 */
	if (id != client->reliable_write_session_id) {
		util_debug(client->debug_callback, client->debug_data,
			"There is other reliable write session ongoing %u",
			client->reliable_write_session_id);

		return 0;
	}

	req = get_reliable_request(client, id);
	if (!req)
		return 0;

	op = (struct prep_write_op *)req->data;

	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	req->destroy = destroy_prep_write_op;
	req->prep_write = true;

	put_le16(value_handle, pdu);
	put_le16(offset, pdu + 2);
	memcpy(pdu + 4, value, length);

	/*
	 * Before sending command we need to remember pdu as we need to validate
	 * it in the response. Store handle, offset and value. Therefore
	 * increase length by 4 (handle + offset) as we need it in couple places
	 * below
	 */
	length += 4;

	op->pdu = malloc(length);
	if (!op->pdu) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	memcpy(op->pdu, pdu, length);
	op->pdu_len = length;

	/*
	 * Now we are ready to send command
	 * Note that request_unref will be done on write execute
	 */
	req->att_id = bt_att_send(client->att, BT_ATT_OP_PREP_WRITE_REQ, pdu,
					sizeof(pdu), prep_write_cb, req,
					NULL);
	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	/*
	 * Store first request id for prepare write and treat it as a session id
	 * valid until write execute is done
	 */
	if (client->reliable_write_session_id == 0)
		client->reliable_write_session_id = req->id;

	return client->reliable_write_session_id;
}

static void exec_write_cb(uint8_t opcode, const void *pdu, uint16_t length,
								void *user_data)
{
	struct request *req = user_data;
	struct write_op *op = req->data;
	bool success;
	uint8_t att_ecode;

	if (opcode == BT_ATT_OP_ERROR_RSP) {
		success = false;
		att_ecode = process_error(pdu, length);
		goto done;
	}

	if (opcode != BT_ATT_OP_EXEC_WRITE_RSP || pdu || length) {
		success = false;
		att_ecode = 0;
		goto done;
	}

	success = true;
	att_ecode = 0;

done:
	if (op->callback)
		op->callback(success, att_ecode, op->user_data);

	op->client->reliable_write_session_id = 0;

	start_next_long_write(op->client);
}

unsigned int bt_gatt_client_write_execute(struct bt_gatt_client *client,
					unsigned int id,
					bt_gatt_client_callback_t callback,
					void *user_data,
					bt_gatt_client_destroy_func_t destroy)
{
	struct request *req;
	struct write_op *op;
	uint8_t pdu;

	if (!client)
		return 0;

	if (client->in_long_write)
		return 0;

	if (client->reliable_write_session_id != id)
		return 0;

	op = new0(struct write_op, 1);

	req = queue_find(client->pending_requests, match_req_id,
							UINT_TO_PTR(id));
	if (!req) {
		free(op);
		return 0;
	}

	op->client = client;
	op->callback = callback;
	op->user_data = user_data;
	op->destroy = destroy;

	pdu = 0x01;

	req->data = op;
	req->destroy = destroy_write_op;

	req->att_id = bt_att_send(client->att, BT_ATT_OP_EXEC_WRITE_REQ, &pdu,
						sizeof(pdu), exec_write_cb,
						req, request_unref);
	if (!req->att_id) {
		op->destroy = NULL;
		request_unref(req);
		return 0;
	}

	return id;
}

unsigned int bt_gatt_client_register_notify(struct bt_gatt_client *client,
				uint16_t chrc_value_handle,
				bt_gatt_client_register_callback_t callback,
				bt_gatt_client_notify_callback_t notify,
				void *user_data,
				bt_gatt_client_destroy_func_t destroy)
{
	if (!client || !client->db || !chrc_value_handle || !callback)
		return 0;

	if (client->in_svc_chngd)
		return 0;

	return register_notify(client, chrc_value_handle, callback, notify,
							user_data, destroy);
}

bool bt_gatt_client_unregister_notify(struct bt_gatt_client *client,
							unsigned int id)
{
	struct notify_data *notify_data;

	if (!client || !id)
		return false;

	notify_data = queue_remove_if(client->notify_list, match_notify_data_id,
							UINT_TO_PTR(id));
	if (!notify_data)
		return false;

	/* Remove data if it has been queued */
	queue_remove(notify_data->chrc->reg_notify_queue, notify_data);

	/* Reset callbacks */
	notify_data->callback = NULL;
	notify_data->notify = NULL;

	complete_unregister_notify(notify_data);
	return true;
}

bool bt_gatt_client_set_security(struct bt_gatt_client *client, int level)
{
	if (!client)
		return false;

	return bt_att_set_security(client->att, level);
}

int bt_gatt_client_get_security(struct bt_gatt_client *client)
{
	if (!client)
		return -1;

	return bt_att_get_security(client->att, NULL);
}
