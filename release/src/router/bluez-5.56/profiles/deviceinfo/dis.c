// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Texas Instruments, Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <errno.h>

#include <glib.h>

#include "src/log.h"

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"

#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"

#include "profiles/deviceinfo/dis.h"

#define DIS_UUID16	0x180a
#define PNP_ID_SIZE	7

struct bt_dis {
	int			ref_count;
	uint16_t		handle;
	uint8_t			source;
	uint16_t		vendor;
	uint16_t		product;
	uint16_t		version;
	GAttrib			*attrib;	/* GATT connection */
	struct gatt_primary	*primary;	/* Primary details */
	bt_dis_notify		notify;
	void			*notify_data;
	struct queue		*gatt_op;
};

struct characteristic {
	struct gatt_char	attr;	/* Characteristic */
	struct bt_dis		*d;	/* deviceinfo where the char belongs */
};

struct gatt_request {
	unsigned int id;
	struct bt_dis *dis;
	void *user_data;
};

static void destroy_gatt_req(struct gatt_request *req)
{
	queue_remove(req->dis->gatt_op, req);
	bt_dis_unref(req->dis);
	free(req);
}

static void dis_free(struct bt_dis *dis)
{
	bt_dis_detach(dis);

	g_free(dis->primary);
	queue_destroy(dis->gatt_op, (void *) destroy_gatt_req);
	g_free(dis);
}

static void foreach_dis_char(struct gatt_db_attribute *attr, void *user_data)
{
	struct bt_dis *dis = user_data;
	bt_uuid_t pnpid_uuid, uuid;
	uint16_t value_handle;

	/* Ignore if there are multiple instances */
	if (dis->handle)
		return;

	if (!gatt_db_attribute_get_char_data(attr, NULL, &value_handle, NULL, NULL, &uuid))
		return;

	/* Find PNPID characteristic's value handle */
	bt_string_to_uuid(&pnpid_uuid, PNPID_UUID);
	if (bt_uuid_cmp(&pnpid_uuid, &uuid) == 0)
		dis->handle = value_handle;
}

static void foreach_dis_service(struct gatt_db_attribute *attr, void *user_data)
{
	struct bt_dis *dis = user_data;

	/* Ignore if there are multiple instances */
	if (dis->handle)
		return;

	gatt_db_service_foreach_char(attr, foreach_dis_char, dis);
}

struct bt_dis *bt_dis_new(struct gatt_db *db)
{
	struct bt_dis *dis;

	dis = g_try_new0(struct bt_dis, 1);
	if (!dis)
		return NULL;

	dis->gatt_op = queue_new();

	if (db) {
		bt_uuid_t uuid;

		/* Handle the DIS service */
		bt_uuid16_create(&uuid, DIS_UUID16);
		gatt_db_foreach_service(db, &uuid, foreach_dis_service, dis);
		if (!dis->handle) {
			dis_free(dis);
			return NULL;
		}
	}

	return bt_dis_ref(dis);
}

struct bt_dis *bt_dis_new_primary(void *primary)
{
	struct bt_dis *dis;

	dis = g_try_new0(struct bt_dis, 1);
	if (!dis)
		return NULL;

	dis->gatt_op = queue_new();

	if (primary)
		dis->primary = g_memdup(primary, sizeof(*dis->primary));

	return bt_dis_ref(dis);
}

struct bt_dis *bt_dis_ref(struct bt_dis *dis)
{
	if (!dis)
		return NULL;

	__sync_fetch_and_add(&dis->ref_count, 1);

	return dis;
}

void bt_dis_unref(struct bt_dis *dis)
{
	if (!dis)
		return;

	if (__sync_sub_and_fetch(&dis->ref_count, 1))
		return;

	dis_free(dis);
}

static struct gatt_request *create_request(struct bt_dis *dis,
							void *user_data)
{
	struct gatt_request *req;

	req = new0(struct gatt_request, 1);
	req->user_data = user_data;
	req->dis = bt_dis_ref(dis);

	return req;
}

static bool set_and_store_gatt_req(struct bt_dis *dis,
						struct gatt_request *req,
						unsigned int id)
{
	req->id = id;
	return queue_push_head(dis->gatt_op, req);
}

static void read_pnpid_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_dis *dis = req->user_data;
	uint8_t value[PNP_ID_SIZE];
	ssize_t vlen;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Error reading PNP_ID value: %s", att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, value, sizeof(value));
	if (vlen < 0) {
		error("Error reading PNP_ID: Protocol error");
		return;
	}

	if (vlen < 7) {
		error("Error reading PNP_ID: Invalid pdu length received");
		return;
	}

	dis->source = value[0];
	dis->vendor = get_le16(&value[1]);
	dis->product = get_le16(&value[3]);
	dis->version = get_le16(&value[5]);

	DBG("source: 0x%02X vendor: 0x%04X product: 0x%04X version: 0x%04X",
			dis->source, dis->vendor, dis->product, dis->version);

	if (dis->notify)
		dis->notify(dis->source, dis->vendor, dis->product,
						dis->version, dis->notify_data);
}

static void read_char(struct bt_dis *dis, GAttrib *attrib, uint16_t handle,
				GAttribResultFunc func, gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(dis, user_data);

	id = gatt_read_char(attrib, handle, func, req);

	if (set_and_store_gatt_req(dis, req, id))
		return;

	error("dis: Could not read characteristic");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void discover_char(struct bt_dis *dis, GAttrib *attrib,
						uint16_t start, uint16_t end,
						bt_uuid_t *uuid, gatt_cb_t func,
						gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(dis, user_data);

	id = gatt_discover_char(attrib, start, end, uuid, func, req);

	if (set_and_store_gatt_req(dis, req, id))
		return;

	error("dis: Could not send discover characteristic");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void configure_deviceinfo_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_dis *d = req->user_data;
	GSList *l;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Discover deviceinfo characteristics: %s",
							att_ecode2str(status));
		return;
	}

	for (l = characteristics; l; l = l->next) {
		struct gatt_char *c = l->data;

		if (strcmp(c->uuid, PNPID_UUID) == 0) {
			d->handle = c->value_handle;
			read_char(d, d->attrib, d->handle, read_pnpid_cb, d);
			break;
		}
	}
}

bool bt_dis_attach(struct bt_dis *dis, void *attrib)
{
	struct gatt_primary *primary = dis->primary;

	if (dis->attrib)
		return false;

	dis->attrib = g_attrib_ref(attrib);

	if (!dis->handle)
		discover_char(dis, dis->attrib, primary->range.start,
						primary->range.end, NULL,
						configure_deviceinfo_cb, dis);
	else
		read_char(dis, attrib, dis->handle, read_pnpid_cb, dis);

	return true;
}

static void cancel_gatt_req(struct gatt_request *req)
{
	if (g_attrib_cancel(req->dis->attrib, req->id))
		destroy_gatt_req(req);
}

void bt_dis_detach(struct bt_dis *dis)
{
	if (!dis->attrib)
		return;

	queue_foreach(dis->gatt_op, (void *) cancel_gatt_req, NULL);
	g_attrib_unref(dis->attrib);
	dis->attrib = NULL;
}

bool bt_dis_set_notification(struct bt_dis *dis, bt_dis_notify func,
							void *user_data)
{
	if (!dis)
		return false;

	dis->notify = func;
	dis->notify_data = user_data;

	return true;
}
