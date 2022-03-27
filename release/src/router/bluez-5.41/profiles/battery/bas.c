/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can rebastribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is bastributed in the hope that it will be useful,
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

#include <glib.h>

#include "src/log.h"

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "src/shared/queue.h"

#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"

#include "profiles/battery/bas.h"

#define ATT_NOTIFICATION_HEADER_SIZE 3
#define ATT_READ_RESPONSE_HEADER_SIZE 1

struct bt_bas {
	int ref_count;
	GAttrib *attrib;
	struct gatt_primary *primary;
	uint16_t handle;
	uint16_t ccc_handle;
	guint id;
	struct queue *gatt_op;
};

struct gatt_request {
	unsigned int id;
	struct bt_bas *bas;
	void *user_data;
};

static void destroy_gatt_req(struct gatt_request *req)
{
	queue_remove(req->bas->gatt_op, req);
	bt_bas_unref(req->bas);
	free(req);
}

static void bas_free(struct bt_bas *bas)
{
	bt_bas_detach(bas);

	g_free(bas->primary);
	queue_destroy(bas->gatt_op, (void *) destroy_gatt_req);
	free(bas);
}

struct bt_bas *bt_bas_new(void *primary)
{
	struct bt_bas *bas;

	bas = new0(struct bt_bas, 1);
	bas->gatt_op = queue_new();

	if (primary)
		bas->primary = g_memdup(primary, sizeof(*bas->primary));

	return bt_bas_ref(bas);
}

struct bt_bas *bt_bas_ref(struct bt_bas *bas)
{
	if (!bas)
		return NULL;

	__sync_fetch_and_add(&bas->ref_count, 1);

	return bas;
}

void bt_bas_unref(struct bt_bas *bas)
{
	if (!bas)
		return;

	if (__sync_sub_and_fetch(&bas->ref_count, 1))
		return;

	bas_free(bas);
}

static struct gatt_request *create_request(struct bt_bas *bas,
							void *user_data)
{
	struct gatt_request *req;

	req = new0(struct gatt_request, 1);
	req->user_data = user_data;
	req->bas = bt_bas_ref(bas);

	return req;
}

static void set_and_store_gatt_req(struct bt_bas *bas,
						struct gatt_request *req,
						unsigned int id)
{
	req->id = id;
	queue_push_head(bas->gatt_op, req);
}

static void write_char(struct bt_bas *bas, GAttrib *attrib, uint16_t handle,
					const uint8_t *value, size_t vlen,
					GAttribResultFunc func,
					gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(bas, user_data);

	id = gatt_write_char(attrib, handle, value, vlen, func, req);

	set_and_store_gatt_req(bas, req, id);
}

static void read_char(struct bt_bas *bas, GAttrib *attrib, uint16_t handle,
				GAttribResultFunc func, gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(bas, user_data);

	id = gatt_read_char(attrib, handle, func, req);

	set_and_store_gatt_req(bas, req, id);
}

static void discover_char(struct bt_bas *bas, GAttrib *attrib,
						uint16_t start, uint16_t end,
						bt_uuid_t *uuid, gatt_cb_t func,
						gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(bas, user_data);

	id = gatt_discover_char(attrib, start, end, uuid, func, req);

	set_and_store_gatt_req(bas, req, id);
}

static void discover_desc(struct bt_bas *bas, GAttrib *attrib,
				uint16_t start, uint16_t end, bt_uuid_t *uuid,
				gatt_cb_t func, gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(bas, user_data);

	id = gatt_discover_desc(attrib, start, end, uuid, func, req);
	set_and_store_gatt_req(bas, req, id);
}

static void notification_cb(const guint8 *pdu, guint16 len, gpointer user_data)
{
	DBG("Battery Level at %u", pdu[ATT_NOTIFICATION_HEADER_SIZE]);
}

static void read_value_cb(guint8 status, const guint8 *pdu, guint16 len,
					gpointer user_data)
{
	DBG("Battery Level at %u", pdu[ATT_READ_RESPONSE_HEADER_SIZE]);
}

static void ccc_written_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_bas *bas = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Write Scan Refresh CCC failed: %s",
						att_ecode2str(status));
		return;
	}

	DBG("Battery Level: notification enabled");

	bas->id = g_attrib_register(bas->attrib, ATT_OP_HANDLE_NOTIFY,
					bas->handle, notification_cb, bas,
					NULL);
}

static void write_ccc(struct bt_bas *bas, GAttrib *attrib, uint16_t handle,
							void *user_data)
{
	uint8_t value[2];

	put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, value);

	write_char(bas, attrib, handle, value, sizeof(value), ccc_written_cb,
								user_data);
}

static void ccc_read_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_bas *bas = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Error reading CCC value: %s", att_ecode2str(status));
		return;
	}

	write_ccc(bas, bas->attrib, bas->ccc_handle, bas);
}

static void discover_descriptor_cb(uint8_t status, GSList *descs,
								void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_bas *bas = req->user_data;
	struct gatt_desc *desc;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Discover descriptors failed: %s", att_ecode2str(status));
		return;
	}

	/* There will be only one descriptor on list and it will be CCC */
	desc = descs->data;
	bas->ccc_handle = desc->handle;

	read_char(bas, bas->attrib, desc->handle, ccc_read_cb, bas);
}

static void bas_discovered_cb(uint8_t status, GSList *chars, void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_bas *bas = req->user_data;
	struct gatt_char *chr;
	uint16_t start, end;
	bt_uuid_t uuid;

	destroy_gatt_req(req);

	if (status) {
		error("Battery: %s", att_ecode2str(status));
		return;
	}

	chr = chars->data;
	bas->handle = chr->value_handle;

	DBG("Battery handle: 0x%04x", bas->handle);

	read_char(bas, bas->attrib, bas->handle, read_value_cb, bas);

	start = chr->value_handle + 1;
	end = bas->primary->range.end;

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);

	discover_desc(bas, bas->attrib, start, end, &uuid,
						discover_descriptor_cb, bas);
}

bool bt_bas_attach(struct bt_bas *bas, void *attrib)
{
	if (!bas || bas->attrib || !bas->primary)
		return false;

	bas->attrib = g_attrib_ref(attrib);

	if (bas->handle > 0)
		return true;

	discover_char(bas, bas->attrib, bas->primary->range.start,
					bas->primary->range.end, NULL,
					bas_discovered_cb, bas);

	return true;
}

static void cancel_gatt_req(struct gatt_request *req)
{
	if (g_attrib_cancel(req->bas->attrib, req->id))
		destroy_gatt_req(req);
}

void bt_bas_detach(struct bt_bas *bas)
{
	if (!bas || !bas->attrib)
		return;

	if (bas->id > 0) {
		g_attrib_unregister(bas->attrib, bas->id);
		bas->id = 0;
	}

	queue_foreach(bas->gatt_op, (void *) cancel_gatt_req, NULL);
	g_attrib_unref(bas->attrib);
	bas->attrib = NULL;
}
