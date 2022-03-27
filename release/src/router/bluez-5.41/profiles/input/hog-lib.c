/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation.
 *  Copyright (C) 2012  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2012  Nordic Semiconductor Inc.
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "src/shared/uhid.h"
#include "src/shared/queue.h"
#include "src/log.h"

#include "attrib/att.h"
#include "attrib/gattrib.h"
#include "attrib/gatt.h"

#include "btio/btio.h"

#include "profiles/scanparam/scpp.h"
#include "profiles/deviceinfo/dis.h"
#include "profiles/battery/bas.h"
#include "profiles/input/hog-lib.h"

#define HOG_UUID		"00001812-0000-1000-8000-00805f9b34fb"

#define HOG_INFO_UUID		0x2A4A
#define HOG_REPORT_MAP_UUID	0x2A4B
#define HOG_REPORT_UUID		0x2A4D
#define HOG_PROTO_MODE_UUID	0x2A4E
#define HOG_CONTROL_POINT_UUID	0x2A4C

#define HOG_REPORT_TYPE_INPUT	1
#define HOG_REPORT_TYPE_OUTPUT	2
#define HOG_REPORT_TYPE_FEATURE	3

#define HOG_PROTO_MODE_BOOT    0
#define HOG_PROTO_MODE_REPORT  1

#define HOG_REPORT_MAP_MAX_SIZE        512
#define HID_INFO_SIZE			4
#define ATT_NOTIFICATION_HEADER_SIZE	3

struct bt_hog {
	int			ref_count;
	char			*name;
	uint16_t		vendor;
	uint16_t		product;
	uint16_t		version;
	struct gatt_primary	*primary;
	GAttrib			*attrib;
	GSList			*reports;
	struct bt_uhid		*uhid;
	int			uhid_fd;
	bool			uhid_created;
	gboolean		has_report_id;
	uint16_t		bcdhid;
	uint8_t			bcountrycode;
	uint16_t		proto_mode_handle;
	uint16_t		ctrlpt_handle;
	uint8_t			flags;
	unsigned int		getrep_att;
	uint16_t		getrep_id;
	unsigned int		setrep_att;
	uint16_t		setrep_id;
	struct bt_scpp		*scpp;
	struct bt_dis		*dis;
	struct queue		*bas;
	GSList			*instances;
	struct queue		*gatt_op;
};

struct report {
	struct bt_hog		*hog;
	uint8_t			id;
	uint8_t			type;
	uint16_t		ccc_handle;
	guint			notifyid;
	struct gatt_char	*decl;
	uint16_t		len;
	uint8_t			*value;
};

struct gatt_request {
	unsigned int id;
	struct bt_hog *hog;
	void *user_data;
};

static struct gatt_request *create_request(struct bt_hog *hog,
							void *user_data)
{
	struct gatt_request *req;

	req = new0(struct gatt_request, 1);
	if (!req)
		return NULL;

	req->user_data = user_data;
	req->hog = bt_hog_ref(hog);

	return req;
}

static bool set_and_store_gatt_req(struct bt_hog *hog,
						struct gatt_request *req,
						unsigned int id)
{
	req->id = id;
	return queue_push_head(hog->gatt_op, req);
}

static void destroy_gatt_req(struct gatt_request *req)
{
	queue_remove(req->hog->gatt_op, req);
	bt_hog_unref(req->hog);
	free(req);
}

static void write_char(struct bt_hog *hog, GAttrib *attrib, uint16_t handle,
					const uint8_t *value, size_t vlen,
					GAttribResultFunc func,
					gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_write_char(attrib, handle, value, vlen, func, req);

	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("hog: Could not read char");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void read_char(struct bt_hog *hog, GAttrib *attrib, uint16_t handle,
				GAttribResultFunc func, gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_read_char(attrib, handle, func, req);

	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("hog: Could not read char");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void discover_desc(struct bt_hog *hog, GAttrib *attrib,
				uint16_t start, uint16_t end, gatt_cb_t func,
				gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_discover_desc(attrib, start, end, NULL, func, req);
	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("hog: Could not discover descriptors");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void discover_char(struct bt_hog *hog, GAttrib *attrib,
						uint16_t start, uint16_t end,
						bt_uuid_t *uuid, gatt_cb_t func,
						gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_discover_char(attrib, start, end, uuid, func, req);

	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("hog: Could not discover characteristic");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void discover_primary(struct bt_hog *hog, GAttrib *attrib,
						bt_uuid_t *uuid, gatt_cb_t func,
						gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_discover_primary(attrib, uuid, func, req);

	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("hog: Could not send discover primary");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void find_included(struct bt_hog *hog, GAttrib *attrib,
					uint16_t start, uint16_t end,
					gatt_cb_t func, gpointer user_data)
{
	struct gatt_request *req;
	unsigned int id;

	req = create_request(hog, user_data);
	if (!req)
		return;

	id = gatt_find_included(attrib, start, end, func, req);

	if (set_and_store_gatt_req(hog, req, id))
		return;

	error("Could not find included");
	g_attrib_cancel(attrib, id);
	free(req);
}

static void report_value_cb(const guint8 *pdu, guint16 len, gpointer user_data)
{
	struct report *report = user_data;
	struct bt_hog *hog = report->hog;
	struct uhid_event ev;
	uint8_t *buf;
	int err;

	if (len < ATT_NOTIFICATION_HEADER_SIZE) {
		error("Malformed ATT notification");
		return;
	}

	pdu += ATT_NOTIFICATION_HEADER_SIZE;
	len -= ATT_NOTIFICATION_HEADER_SIZE;

	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_INPUT;
	buf = ev.u.input.data;

	if (hog->has_report_id) {
		buf[0] = report->id;
		len = MIN(len, sizeof(ev.u.input.data) - 1);
		memcpy(buf + 1, pdu, len);
		ev.u.input.size = ++len;
	} else {
		len = MIN(len, sizeof(ev.u.input.data));
		memcpy(buf, pdu, len);
		ev.u.input.size = len;
	}

	err = bt_uhid_send(hog->uhid, &ev);
	if (err < 0) {
		error("bt_uhid_send: %s (%d)", strerror(-err), -err);
		return;
	}

	DBG("HoG report (%u bytes)", ev.u.input.size);
}

static void report_ccc_written_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct report *report = req->user_data;
	struct bt_hog *hog = report->hog;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Write report characteristic descriptor failed: %s",
							att_ecode2str(status));
		return;
	}

	report->notifyid = g_attrib_register(hog->attrib,
					ATT_OP_HANDLE_NOTIFY,
					report->decl->value_handle,
					report_value_cb, report, NULL);

	DBG("Report characteristic descriptor written: notifications enabled");
}

static void write_ccc(struct bt_hog *hog, GAttrib *attrib, uint16_t handle,
							void *user_data)
{
	uint8_t value[2];

	put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, value);

	write_char(hog, attrib, handle, value, sizeof(value),
					report_ccc_written_cb, user_data);
}

static void ccc_read_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct report *report = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Error reading CCC value: %s", att_ecode2str(status));
		return;
	}

	write_ccc(report->hog, report->hog->attrib, report->ccc_handle, report);
}

static const char *type_to_string(uint8_t type)
{
	switch (type) {
	case HOG_REPORT_TYPE_INPUT:
		return "input";
	case HOG_REPORT_TYPE_OUTPUT:
		return "output";
	case HOG_REPORT_TYPE_FEATURE:
		return "feature";
	}

	return NULL;
}

static void report_reference_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct report *report = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Read Report Reference descriptor failed: %s",
							att_ecode2str(status));
		return;
	}

	if (plen != 3) {
		error("Malformed ATT read response");
		return;
	}

	report->id = pdu[1];
	report->type = pdu[2];

	DBG("Report 0x%04x: id 0x%02x type %s", report->decl->value_handle,
				report->id, type_to_string(report->type));

	/* Enable notifications only for Input Reports */
	if (report->type == HOG_REPORT_TYPE_INPUT)
		read_char(report->hog, report->hog->attrib, report->ccc_handle,
							ccc_read_cb, report);
}

static void external_report_reference_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data);

static void discover_external_cb(uint8_t status, GSList *descs, void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Discover external descriptors failed: %s",
							att_ecode2str(status));
		return;
	}

	for ( ; descs; descs = descs->next) {
		struct gatt_desc *desc = descs->data;

		read_char(hog, hog->attrib, desc->handle,
						external_report_reference_cb,
						hog);
	}
}

static void discover_external(struct bt_hog *hog, GAttrib *attrib,
						uint16_t start, uint16_t end,
						gpointer user_data)
{
	bt_uuid_t uuid;

	if (start > end)
		return;

	bt_uuid16_create(&uuid, GATT_EXTERNAL_REPORT_REFERENCE);

	discover_desc(hog, attrib, start, end, discover_external_cb,
								user_data);
}

static void discover_report_cb(uint8_t status, GSList *descs, void *user_data)
{
	struct gatt_request *req = user_data;
	struct report *report = req->user_data;
	struct bt_hog *hog = report->hog;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Discover report descriptors failed: %s",
							att_ecode2str(status));
		return;
	}

	for ( ; descs; descs = descs->next) {
		struct gatt_desc *desc = descs->data;

		switch (desc->uuid16) {
		case GATT_CLIENT_CHARAC_CFG_UUID:
			report->ccc_handle = desc->handle;
			break;
		case GATT_REPORT_REFERENCE:
			read_char(hog, hog->attrib, desc->handle,
						report_reference_cb, report);
			break;
		}
	}
}

static void discover_report(struct bt_hog *hog, GAttrib *attrib,
						uint16_t start, uint16_t end,
							gpointer user_data)
{
	if (start > end)
		return;

	discover_desc(hog, attrib, start, end, discover_report_cb, user_data);
}

static void report_read_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct report *report = req->user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Error reading Report value: %s", att_ecode2str(status));
		return;
	}

	if (report->value)
		g_free(report->value);

	report->value = g_memdup(pdu, len);
	report->len = len;
}

static int report_chrc_cmp(const void *data, const void *user_data)
{
	const struct report *report = data;
	const struct gatt_char *decl = user_data;

	return report->decl->handle - decl->handle;
}

static struct report *report_new(struct bt_hog *hog, struct gatt_char *chr)
{
	struct report *report;
	GSList *l;

	/* Skip if report already exists */
	l = g_slist_find_custom(hog->reports, chr, report_chrc_cmp);
	if (l)
		return l->data;

	report = g_new0(struct report, 1);
	report->hog = hog;
	report->decl = g_memdup(chr, sizeof(*chr));
	hog->reports = g_slist_append(hog->reports, report);

	read_char(hog, hog->attrib, chr->value_handle, report_read_cb, report);

	return report;
}

static void external_service_char_cb(uint8_t status, GSList *chars,
								void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	struct gatt_primary *primary = hog->primary;
	struct report *report;
	GSList *l;

	destroy_gatt_req(req);

	if (status != 0) {
		const char *str = att_ecode2str(status);
		DBG("Discover external service characteristic failed: %s", str);
		return;
	}

	for (l = chars; l; l = g_slist_next(l)) {
		struct gatt_char *chr, *next;
		uint16_t start, end;

		chr = l->data;
		next = l->next ? l->next->data : NULL;

		DBG("0x%04x UUID: %s properties: %02x",
				chr->handle, chr->uuid, chr->properties);

		report = report_new(hog, chr);
		start = chr->value_handle + 1;
		end = (next ? next->handle - 1 : primary->range.end);
		discover_report(hog, hog->attrib, start, end, report);
	}
}

static void external_report_reference_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	uint16_t uuid16;
	bt_uuid_t uuid;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Read External Report Reference descriptor failed: %s",
							att_ecode2str(status));
		return;
	}

	if (plen != 3) {
		error("Malformed ATT read response");
		return;
	}

	uuid16 = get_le16(&pdu[1]);
	DBG("External report reference read, external report characteristic "
						"UUID: 0x%04x", uuid16);

	/* Do not discover if is not a Report */
	if (uuid16 != HOG_REPORT_UUID)
		return;

	bt_uuid16_create(&uuid, uuid16);
	discover_char(hog, hog->attrib, 0x0001, 0xffff, &uuid,
					external_service_char_cb, hog);
}

static int report_cmp(gconstpointer a, gconstpointer b)
{
	const struct report *ra = a, *rb = b;

	/* sort by type first.. */
	if (ra->type != rb->type)
		return ra->type - rb->type;

	/* skip id check in case of report id 0 */
	if (!rb->id)
		return 0;

	/* ..then by id */
	return ra->id - rb->id;
}

static struct report *find_report(struct bt_hog *hog, uint8_t type, uint8_t id)
{
	struct report cmp;
	GSList *l;

	cmp.type = type;
	cmp.id = hog->has_report_id ? id : 0;

	l = g_slist_find_custom(hog->reports, &cmp, report_cmp);

	return l ? l->data : NULL;
}

static struct report *find_report_by_rtype(struct bt_hog *hog, uint8_t rtype,
								uint8_t id)
{
	uint8_t type;

	switch (rtype) {
	case UHID_FEATURE_REPORT:
		type = HOG_REPORT_TYPE_FEATURE;
		break;
	case UHID_OUTPUT_REPORT:
		type = HOG_REPORT_TYPE_OUTPUT;
		break;
	case UHID_INPUT_REPORT:
		type = HOG_REPORT_TYPE_INPUT;
		break;
	default:
		return NULL;
	}

	return find_report(hog, type, id);
}

static void output_written_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct gatt_request *req = user_data;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Write output report failed: %s", att_ecode2str(status));
		return;
	}
}

static void forward_report(struct uhid_event *ev, void *user_data)
{
	struct bt_hog *hog = user_data;
	struct report *report;
	void *data;
	int size;

	report = find_report_by_rtype(hog, ev->u.output.rtype,
							ev->u.output.data[0]);
	if (!report)
		return;

	data = ev->u.output.data;
	size = ev->u.output.size;
	if (hog->has_report_id && size > 0) {
		data++;
		--size;
	}

	DBG("Sending report type %d ID %d to handle 0x%X", report->type,
				report->id, report->decl->value_handle);

	if (hog->attrib == NULL)
		return;

	if (report->decl->properties & GATT_CHR_PROP_WRITE)
		write_char(hog, hog->attrib, report->decl->value_handle,
				data, size, output_written_cb, hog);
	else if (report->decl->properties & GATT_CHR_PROP_WRITE_WITHOUT_RESP)
		gatt_write_cmd(hog->attrib, report->decl->value_handle,
						data, size, NULL, NULL);
}

static void get_feature(struct uhid_event *ev, void *user_data)
{
	struct bt_hog *hog = user_data;
	struct report *report;
	struct uhid_event rsp;
	int err;

	memset(&rsp, 0, sizeof(rsp));
	rsp.type = UHID_FEATURE_ANSWER;
	rsp.u.feature_answer.id = ev->u.feature.id;

	report = find_report_by_rtype(hog, ev->u.feature.rtype,
							ev->u.feature.rnum);
	if (!report) {
		rsp.u.feature_answer.err = ENOTSUP;
		goto done;
	}

	if (!report->value) {
		rsp.u.feature_answer.err = EIO;
		goto done;
	}

	rsp.u.feature_answer.size = report->len;
	memcpy(rsp.u.feature_answer.data, report->value, report->len);

done:
	err = bt_uhid_send(hog->uhid, &rsp);
	if (err < 0)
		error("bt_uhid_send: %s", strerror(-err));
}

static void set_report_cb(guint8 status, const guint8 *pdu,
					guint16 plen, gpointer user_data)
{
	struct bt_hog *hog = user_data;
	struct uhid_event rsp;
	int err;

	hog->setrep_att = 0;

	memset(&rsp, 0, sizeof(rsp));
	rsp.type = UHID_SET_REPORT_REPLY;
	rsp.u.set_report_reply.id = hog->setrep_id;
	rsp.u.set_report_reply.err = status;

	if (status != 0)
		error("Error setting Report value: %s", att_ecode2str(status));

	err = bt_uhid_send(hog->uhid, &rsp);
	if (err < 0)
		error("bt_uhid_send: %s", strerror(-err));
}

static void set_report(struct uhid_event *ev, void *user_data)
{
	struct bt_hog *hog = user_data;
	struct report *report;
	void *data;
	int size;
	int err;

	/* uhid never sends reqs in parallel; if there's a req, it timed out */
	if (hog->setrep_att) {
		g_attrib_cancel(hog->attrib, hog->setrep_att);
		hog->setrep_att = 0;
	}

	hog->setrep_id = ev->u.set_report.id;

	report = find_report_by_rtype(hog, ev->u.set_report.rtype,
							ev->u.set_report.rnum);
	if (!report) {
		err = ENOTSUP;
		goto fail;
	}

	data = ev->u.set_report.data;
	size = ev->u.set_report.size;
	if (hog->has_report_id && size > 0) {
		data++;
		--size;
	}

	DBG("Sending report type %d ID %d to handle 0x%X", report->type,
				report->id, report->decl->value_handle);

	if (hog->attrib == NULL)
		return;

	hog->setrep_att = gatt_write_char(hog->attrib,
						report->decl->value_handle,
						data, size, set_report_cb,
						hog);
	if (!hog->setrep_att) {
		err = ENOMEM;
		goto fail;
	}

	return;
fail:
	/* cancel the request on failure */
	set_report_cb(err, NULL, 0, hog);
}

static void get_report_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct bt_hog *hog = user_data;
	struct uhid_event rsp;
	int err;

	hog->getrep_att = 0;

	memset(&rsp, 0, sizeof(rsp));
	rsp.type = UHID_GET_REPORT_REPLY;
	rsp.u.get_report_reply.id = hog->getrep_id;

	if (status != 0) {
		error("Error reading Report value: %s", att_ecode2str(status));
		goto exit;
	}

	if (len == 0) {
		error("Error reading Report, length %d", len);
		status = EIO;
		goto exit;
	}

	if (pdu[0] != 0x0b) {
		error("Error reading Report, invalid response: %02x", pdu[0]);
		status = EPROTO;
		goto exit;
	}

	--len;
	++pdu;
	if (hog->has_report_id && len > 0) {
		--len;
		++pdu;
	}

	rsp.u.get_report_reply.size = len;
	memcpy(rsp.u.get_report_reply.data, pdu, len);

exit:
	rsp.u.get_report_reply.err = status;
	err = bt_uhid_send(hog->uhid, &rsp);
	if (err < 0)
		error("bt_uhid_send: %s", strerror(-err));
}

static void get_report(struct uhid_event *ev, void *user_data)
{
	struct bt_hog *hog = user_data;
	struct report *report;
	guint8 err;

	/* uhid never sends reqs in parallel; if there's a req, it timed out */
	if (hog->getrep_att) {
		g_attrib_cancel(hog->attrib, hog->getrep_att);
		hog->getrep_att = 0;
	}

	hog->getrep_id = ev->u.get_report.id;

	report = find_report_by_rtype(hog, ev->u.get_report.rtype,
							ev->u.get_report.rnum);
	if (!report) {
		err = ENOTSUP;
		goto fail;
	}

	hog->getrep_att = gatt_read_char(hog->attrib,
						report->decl->value_handle,
						get_report_cb, hog);
	if (!hog->getrep_att) {
		err = ENOMEM;
		goto fail;
	}

	return;

fail:
	/* cancel the request on failure */
	get_report_cb(err, NULL, 0, hog);
}

static bool get_descriptor_item_info(uint8_t *buf, ssize_t blen, ssize_t *len,
								bool *is_long)
{
	if (!blen)
		return false;

	*is_long = (buf[0] == 0xfe);

	if (*is_long) {
		if (blen < 3)
			return false;

		/*
		 * long item:
		 * byte 0 -> 0xFE
		 * byte 1 -> data size
		 * byte 2 -> tag
		 * + data
		 */

		*len = buf[1] + 3;
	} else {
		uint8_t b_size;

		/*
		 * short item:
		 * byte 0[1..0] -> data size (=0, 1, 2, 4)
		 * byte 0[3..2] -> type
		 * byte 0[7..4] -> tag
		 * + data
		 */

		b_size = buf[0] & 0x03;
		*len = (b_size ? 1 << (b_size - 1) : 0) + 1;
	}

	/* item length should be no more than input buffer length */
	return *len <= blen;
}

static char *item2string(char *str, uint8_t *buf, uint8_t len)
{
	char *p = str;
	int i;

	/*
	 * Since long item tags are not defined except for vendor ones, we
	 * just ensure that short items are printed properly (up to 5 bytes).
	 */
	for (i = 0; i < 6 && i < len; i++)
		p += sprintf(p, " %02x", buf[i]);

	/*
	 * If there are some data left, just add continuation mark to indicate
	 * this.
	 */
	if (i < len)
		sprintf(p, " ...");

	return str;
}

static void report_map_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	uint8_t value[HOG_REPORT_MAP_MAX_SIZE];
	struct uhid_event ev;
	ssize_t vlen;
	char itemstr[20]; /* 5x3 (data) + 4 (continuation) + 1 (null) */
	int i, err;
	GError *gerr = NULL;

	destroy_gatt_req(req);

	DBG("HoG inspecting report map");

	if (status != 0) {
		error("Report Map read failed: %s", att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, plen, value, sizeof(value));
	if (vlen < 0) {
		error("ATT protocol error");
		return;
	}

	DBG("Report MAP:");
	for (i = 0; i < vlen;) {
		ssize_t ilen = 0;
		bool long_item = false;

		if (get_descriptor_item_info(&value[i], vlen - i, &ilen,
								&long_item)) {
			/* Report ID is short item with prefix 100001xx */
			if (!long_item && (value[i] & 0xfc) == 0x84)
				hog->has_report_id = TRUE;

			DBG("\t%s", item2string(itemstr, &value[i], ilen));

			i += ilen;
		} else {
			error("Report Map parsing failed at %d", i);

			/* Just print remaining items at once and break */
			DBG("\t%s", item2string(itemstr, &value[i], vlen - i));
			break;
		}
	}

	/* create uHID device */
	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_CREATE;

	bt_io_get(g_attrib_get_channel(hog->attrib), &gerr,
			BT_IO_OPT_SOURCE, ev.u.create.phys,
			BT_IO_OPT_DEST, ev.u.create.uniq,
			BT_IO_OPT_INVALID);
	if (gerr) {
		error("Failed to connection details: %s", gerr->message);
		g_error_free(gerr);
		return;
	}

	strcpy((char *) ev.u.create.name, hog->name);
	ev.u.create.vendor = hog->vendor;
	ev.u.create.product = hog->product;
	ev.u.create.version = hog->version;
	ev.u.create.country = hog->bcountrycode;
	ev.u.create.bus = BUS_BLUETOOTH;
	ev.u.create.rd_data = value;
	ev.u.create.rd_size = vlen;

	err = bt_uhid_send(hog->uhid, &ev);
	if (err < 0) {
		error("bt_uhid_send: %s", strerror(-err));
		return;
	}

	bt_uhid_register(hog->uhid, UHID_OUTPUT, forward_report, hog);
	bt_uhid_register(hog->uhid, UHID_FEATURE, get_feature, hog);
	bt_uhid_register(hog->uhid, UHID_GET_REPORT, get_report, hog);
	bt_uhid_register(hog->uhid, UHID_SET_REPORT, set_report, hog);

	hog->uhid_created = true;

	DBG("HoG created uHID device");
}

static void info_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	uint8_t value[HID_INFO_SIZE];
	ssize_t vlen;

	destroy_gatt_req(req);

	if (status != 0) {
		error("HID Information read failed: %s",
						att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, plen, value, sizeof(value));
	if (vlen != 4) {
		error("ATT protocol error");
		return;
	}

	hog->bcdhid = get_le16(&value[0]);
	hog->bcountrycode = value[2];
	hog->flags = value[3];

	DBG("bcdHID: 0x%04X bCountryCode: 0x%02X Flags: 0x%02X",
			hog->bcdhid, hog->bcountrycode, hog->flags);
}

static void proto_mode_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	uint8_t value;
	ssize_t vlen;

	destroy_gatt_req(req);

	if (status != 0) {
		error("Protocol Mode characteristic read failed: %s",
							att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, plen, &value, sizeof(value));
	if (vlen < 0) {
		error("ATT protocol error");
		return;
	}

	if (value == HOG_PROTO_MODE_BOOT) {
		uint8_t nval = HOG_PROTO_MODE_REPORT;

		DBG("HoG is operating in Boot Procotol Mode");

		gatt_write_cmd(hog->attrib, hog->proto_mode_handle, &nval,
						sizeof(nval), NULL, NULL);
	} else if (value == HOG_PROTO_MODE_REPORT)
		DBG("HoG is operating in Report Protocol Mode");
}

static void char_discovered_cb(uint8_t status, GSList *chars, void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	struct gatt_primary *primary = hog->primary;
	bt_uuid_t report_uuid, report_map_uuid, info_uuid;
	bt_uuid_t proto_mode_uuid, ctrlpt_uuid;
	struct report *report;
	GSList *l;
	uint16_t info_handle = 0, proto_mode_handle = 0;

	destroy_gatt_req(req);

	DBG("HoG inspecting characteristics");

	if (status != 0) {
		const char *str = att_ecode2str(status);
		DBG("Discover all characteristics failed: %s", str);
		return;
	}

	bt_uuid16_create(&report_uuid, HOG_REPORT_UUID);
	bt_uuid16_create(&report_map_uuid, HOG_REPORT_MAP_UUID);
	bt_uuid16_create(&info_uuid, HOG_INFO_UUID);
	bt_uuid16_create(&proto_mode_uuid, HOG_PROTO_MODE_UUID);
	bt_uuid16_create(&ctrlpt_uuid, HOG_CONTROL_POINT_UUID);

	for (l = chars; l; l = g_slist_next(l)) {
		struct gatt_char *chr, *next;
		bt_uuid_t uuid;
		uint16_t start, end;

		chr = l->data;
		next = l->next ? l->next->data : NULL;

		DBG("0x%04x UUID: %s properties: %02x",
				chr->handle, chr->uuid, chr->properties);

		bt_string_to_uuid(&uuid, chr->uuid);

		start = chr->value_handle + 1;
		end = (next ? next->handle - 1 : primary->range.end);

		if (bt_uuid_cmp(&uuid, &report_uuid) == 0) {
			report = report_new(hog, chr);
			discover_report(hog, hog->attrib, start, end, report);
		} else if (bt_uuid_cmp(&uuid, &report_map_uuid) == 0) {
			DBG("HoG discovering report map");
			read_char(hog, hog->attrib, chr->value_handle,
						report_map_read_cb, hog);
			discover_external(hog, hog->attrib, start, end, hog);
		} else if (bt_uuid_cmp(&uuid, &info_uuid) == 0)
			info_handle = chr->value_handle;
		else if (bt_uuid_cmp(&uuid, &proto_mode_uuid) == 0)
			proto_mode_handle = chr->value_handle;
		else if (bt_uuid_cmp(&uuid, &ctrlpt_uuid) == 0)
			hog->ctrlpt_handle = chr->value_handle;
	}

	if (proto_mode_handle) {
		hog->proto_mode_handle = proto_mode_handle;
		read_char(hog, hog->attrib, proto_mode_handle,
						proto_mode_read_cb, hog);
	}

	if (info_handle)
		read_char(hog, hog->attrib, info_handle, info_read_cb, hog);
}

static void report_free(void *data)
{
	struct report *report = data;

	g_free(report->value);
	g_free(report->decl);
	g_free(report);
}

static void cancel_gatt_req(struct gatt_request *req)
{
	if (g_attrib_cancel(req->hog->attrib, req->id))
		destroy_gatt_req(req);
}

static void hog_free(void *data)
{
	struct bt_hog *hog = data;

	bt_hog_detach(hog);

	queue_destroy(hog->bas, (void *) bt_bas_unref);
	g_slist_free_full(hog->instances, hog_free);

	bt_scpp_unref(hog->scpp);
	bt_dis_unref(hog->dis);
	bt_uhid_unref(hog->uhid);
	g_slist_free_full(hog->reports, report_free);
	g_free(hog->name);
	g_free(hog->primary);
	queue_destroy(hog->gatt_op, (void *) destroy_gatt_req);
	g_free(hog);
}

struct bt_hog *bt_hog_new_default(const char *name, uint16_t vendor,
					uint16_t product, uint16_t version,
					void *primary)
{
	return bt_hog_new(-1, name, vendor, product, version, primary);
}

struct bt_hog *bt_hog_new(int fd, const char *name, uint16_t vendor,
					uint16_t product, uint16_t version,
					void *primary)
{
	struct bt_hog *hog;

	hog = g_try_new0(struct bt_hog, 1);
	if (!hog)
		return NULL;

	hog->gatt_op = queue_new();
	hog->bas = queue_new();

	if (fd < 0)
		hog->uhid = bt_uhid_new_default();
	else
		hog->uhid = bt_uhid_new(fd);

	hog->uhid_fd = fd;

	if (!hog->gatt_op || !hog->bas || !hog->uhid) {
		hog_free(hog);
		return NULL;
	}

	hog->name = g_strdup(name);
	hog->vendor = vendor;
	hog->product = product;
	hog->version = version;

	if (primary)
		hog->primary = g_memdup(primary, sizeof(*hog->primary));

	return bt_hog_ref(hog);
}

struct bt_hog *bt_hog_ref(struct bt_hog *hog)
{
	if (!hog)
		return NULL;

	__sync_fetch_and_add(&hog->ref_count, 1);

	return hog;
}

void bt_hog_unref(struct bt_hog *hog)
{
	if (!hog)
		return;

	if (__sync_sub_and_fetch(&hog->ref_count, 1))
		return;

	hog_free(hog);
}

static void find_included_cb(uint8_t status, GSList *services, void *user_data)
{
	struct gatt_request *req = user_data;
	GSList *l;

	DBG("");

	destroy_gatt_req(req);

	if (status) {
		const char *str = att_ecode2str(status);
		DBG("Find included failed: %s", str);
		return;
	}

	for (l = services; l; l = l->next) {
		struct gatt_included *include = l->data;

		DBG("included: handle %x, uuid %s",
			include->handle, include->uuid);
	}
}

static void hog_attach_scpp(struct bt_hog *hog, struct gatt_primary *primary)
{
	if (hog->scpp) {
		bt_scpp_attach(hog->scpp, hog->attrib);
		return;
	}

	hog->scpp = bt_scpp_new(primary);
	if (hog->scpp)
		bt_scpp_attach(hog->scpp, hog->attrib);
}

static void dis_notify(uint8_t source, uint16_t vendor, uint16_t product,
					uint16_t version, void *user_data)
{
	struct bt_hog *hog = user_data;

	hog->vendor = vendor;
	hog->product = product;
	hog->version = version;
}

static void hog_attach_dis(struct bt_hog *hog, struct gatt_primary *primary)
{
	if (hog->dis) {
		bt_dis_attach(hog->dis, hog->attrib);
		return;
	}

	hog->dis = bt_dis_new(primary);
	if (hog->dis) {
		bt_dis_set_notification(hog->dis, dis_notify, hog);
		bt_dis_attach(hog->dis, hog->attrib);
	}
}

static void hog_attach_bas(struct bt_hog *hog, struct gatt_primary *primary)
{
	struct bt_bas *instance;

	instance = bt_bas_new(primary);

	bt_bas_attach(instance, hog->attrib);
	queue_push_head(hog->bas, instance);
}

static void hog_attach_hog(struct bt_hog *hog, struct gatt_primary *primary)
{
	struct bt_hog *instance;

	if (!hog->primary) {
		hog->primary = g_memdup(primary, sizeof(*primary));
		discover_char(hog, hog->attrib, primary->range.start,
						primary->range.end, NULL,
						char_discovered_cb, hog);
		find_included(hog, hog->attrib, primary->range.start,
				primary->range.end, find_included_cb, hog);
		return;
	}

	instance = bt_hog_new(hog->uhid_fd, hog->name, hog->vendor,
					hog->product, hog->version, primary);
	if (!instance)
		return;

	find_included(instance, hog->attrib, primary->range.start,
			primary->range.end, find_included_cb, instance);

	bt_hog_attach(instance, hog->attrib);
	hog->instances = g_slist_append(hog->instances, instance);
}

static void primary_cb(uint8_t status, GSList *services, void *user_data)
{
	struct gatt_request *req = user_data;
	struct bt_hog *hog = req->user_data;
	struct gatt_primary *primary;
	GSList *l;

	DBG("");

	destroy_gatt_req(req);

	if (status) {
		const char *str = att_ecode2str(status);
		DBG("Discover primary failed: %s", str);
		return;
	}

	if (!services) {
		DBG("No primary service found");
		return;
	}

	for (l = services; l; l = l->next) {
		primary = l->data;

		if (strcmp(primary->uuid, SCAN_PARAMETERS_UUID) == 0) {
			hog_attach_scpp(hog, primary);
			continue;
		}

		if (strcmp(primary->uuid, DEVICE_INFORMATION_UUID) == 0) {
			hog_attach_dis(hog, primary);
			continue;
		}

		if (strcmp(primary->uuid, BATTERY_UUID) == 0) {
			hog_attach_bas(hog, primary);
			continue;
		}

		if (strcmp(primary->uuid, HOG_UUID) == 0)
			hog_attach_hog(hog, primary);
	}
}

bool bt_hog_attach(struct bt_hog *hog, void *gatt)
{
	struct gatt_primary *primary = hog->primary;
	GSList *l;

	if (hog->attrib)
		return false;

	hog->attrib = g_attrib_ref(gatt);

	if (!primary) {
		discover_primary(hog, hog->attrib, NULL, primary_cb, hog);
		return true;
	}

	if (hog->scpp)
		bt_scpp_attach(hog->scpp, gatt);

	if (hog->dis)
		bt_dis_attach(hog->dis, gatt);

	queue_foreach(hog->bas, (void *) bt_bas_attach, gatt);

	for (l = hog->instances; l; l = l->next) {
		struct bt_hog *instance = l->data;

		bt_hog_attach(instance, gatt);
	}

	if (!hog->uhid_created) {
		DBG("HoG discovering characteristics");
		discover_char(hog, hog->attrib, primary->range.start,
						primary->range.end, NULL,
						char_discovered_cb, hog);
		return true;
	}

	for (l = hog->reports; l; l = l->next) {
		struct report *r = l->data;

		r->notifyid = g_attrib_register(hog->attrib,
					ATT_OP_HANDLE_NOTIFY,
					r->decl->value_handle,
					report_value_cb, r, NULL);
	}

	return true;
}

void bt_hog_detach(struct bt_hog *hog)
{
	GSList *l;

	if (!hog->attrib)
		return;

	queue_foreach(hog->bas, (void *) bt_bas_detach, NULL);

	for (l = hog->instances; l; l = l->next) {
		struct bt_hog *instance = l->data;

		bt_hog_detach(instance);
	}

	for (l = hog->reports; l; l = l->next) {
		struct report *r = l->data;

		if (r->notifyid > 0) {
			g_attrib_unregister(hog->attrib, r->notifyid);
			r->notifyid = 0;
		}
	}

	if (hog->scpp)
		bt_scpp_detach(hog->scpp);

	if (hog->dis)
		bt_dis_detach(hog->dis);

	queue_foreach(hog->gatt_op, (void *) cancel_gatt_req, NULL);
	g_attrib_unref(hog->attrib);
	hog->attrib = NULL;
}

int bt_hog_set_control_point(struct bt_hog *hog, bool suspend)
{
	uint8_t value = suspend ? 0x00 : 0x01;

	if (hog->attrib == NULL)
		return -ENOTCONN;

	if (hog->ctrlpt_handle == 0)
		return -ENOTSUP;

	gatt_write_cmd(hog->attrib, hog->ctrlpt_handle, &value,
					sizeof(value), NULL, NULL);

	return 0;
}

int bt_hog_send_report(struct bt_hog *hog, void *data, size_t size, int type)
{
	struct report *report;
	GSList *l;

	if (!hog)
		return -EINVAL;

	if (!hog->attrib)
		return -ENOTCONN;

	report = find_report(hog, type, 0);
	if (!report)
		return -ENOTSUP;

	DBG("hog: Write report, handle 0x%X", report->decl->value_handle);

	if (report->decl->properties & GATT_CHR_PROP_WRITE)
		write_char(hog, hog->attrib, report->decl->value_handle,
				data, size, output_written_cb, hog);

	if (report->decl->properties & GATT_CHR_PROP_WRITE_WITHOUT_RESP)
		gatt_write_cmd(hog->attrib, report->decl->value_handle,
						data, size, NULL, NULL);

	for (l = hog->instances; l; l = l->next) {
		struct bt_hog *instance = l->data;

		bt_hog_send_report(instance, data, size, type);
	}

	return 0;
}
