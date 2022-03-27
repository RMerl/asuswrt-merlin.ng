/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
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

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <glib.h>

#include "btio/btio.h"
#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "src/shared/mgmt.h"
#include "src/shared/util.h"
#include "src/shared/uhid.h"
#include "src/sdp-client.h"
#include "src/uuid-helper.h"
#include "src/log.h"
#include "profiles/input/hog-lib.h"

#include "hal-msg.h"
#include "ipc-common.h"
#include "ipc.h"
#include "bluetooth.h"
#include "gatt.h"
#include "hidhost.h"
#include "utils.h"

#define L2CAP_PSM_HIDP_CTRL	0x11
#define L2CAP_PSM_HIDP_INTR	0x13

/* HID message types */
#define HID_MSG_HANDSHAKE	0x00
#define HID_MSG_CONTROL		0x10
#define HID_MSG_GET_REPORT	0x40
#define HID_MSG_SET_REPORT	0x50
#define HID_MSG_GET_PROTOCOL	0x60
#define HID_MSG_SET_PROTOCOL	0x70
#define HID_MSG_DATA		0xa0

#define HID_MSG_TYPE_MASK	0xf0

/* HID data types */
#define HID_DATA_TYPE_INPUT	0x01
#define HID_DATA_TYPE_OUTPUT	0x02
#define HID_DATA_TYPE_FEATURE	0x03

/* HID protocol header parameters */
#define HID_PROTO_BOOT		0x00
#define HID_PROTO_REPORT	0x01

/* HID GET REPORT Size Field */
#define HID_GET_REPORT_SIZE_FIELD	0x08

/* HID Virtual Cable Unplug */
#define HID_VIRTUAL_CABLE_UNPLUG	0x05

#define HOG_UUID		"00001812-0000-1000-8000-00805f9b34fb"

static bdaddr_t adapter_addr;

static GIOChannel *ctrl_io = NULL;
static GIOChannel *intr_io = NULL;
static GSList *devices = NULL;
static unsigned int hog_app = 0;

static struct ipc *hal_ipc = NULL;

struct hid_device {
	bdaddr_t	dst;
	uint8_t		state;
	uint8_t		subclass;
	uint16_t	vendor;
	uint16_t	product;
	uint16_t	version;
	uint8_t		country;
	int		rd_size;
	void		*rd_data;
	uint8_t		boot_dev;
	GIOChannel	*ctrl_io;
	GIOChannel	*intr_io;
	guint		ctrl_watch;
	guint		intr_watch;
	struct bt_uhid	*uhid;
	uint8_t		last_hid_msg;
	struct bt_hog	*hog;
	int		sec_level;
};

static int device_cmp(gconstpointer s, gconstpointer user_data)
{
	const struct hid_device *dev = s;
	const bdaddr_t *dst = user_data;

	return bacmp(&dev->dst, dst);
}

static void hid_device_free(void *data)
{
	struct hid_device *dev = data;

	if (dev->ctrl_watch > 0)
		g_source_remove(dev->ctrl_watch);

	if (dev->intr_watch > 0)
		g_source_remove(dev->intr_watch);

	if (dev->intr_io)
		g_io_channel_unref(dev->intr_io);

	if (dev->ctrl_io)
		g_io_channel_unref(dev->ctrl_io);

	if (dev->uhid)
		bt_uhid_unref(dev->uhid);

	if (dev->hog)
		bt_hog_unref(dev->hog);

	g_free(dev->rd_data);
	g_free(dev);
}

static void hid_device_remove(struct hid_device *dev)
{
	devices = g_slist_remove(devices, dev);
	hid_device_free(dev);
}

static struct hid_device *hid_device_new(const bdaddr_t *addr)
{
	struct hid_device *dev;

	dev = g_new0(struct hid_device, 1);
	bacpy(&dev->dst, addr);
	dev->state = HAL_HIDHOST_STATE_DISCONNECTED;
	dev->sec_level = BT_IO_SEC_LOW;

	devices = g_slist_append(devices, dev);

	return dev;
}

static bool hex2buf(const uint8_t *hex, uint8_t *buf, int buf_size)
{
	int i, j;
	char c;
	uint8_t b;

	for (i = 0, j = 0; i < buf_size; i++, j++) {
		c = toupper(hex[j]);

		if (c >= '0' && c <= '9')
			b = c - '0';
		else if (c >= 'A' && c <= 'F')
			b = 10 + c - 'A';
		else
			return false;

		j++;

		c = toupper(hex[j]);

		if (c >= '0' && c <= '9')
			b = b * 16 + c - '0';
		else if (c >= 'A' && c <= 'F')
			b = b * 16 + 10 + c - 'A';
		else
			return false;

		buf[i] = b;
	}

	return true;
}

static void handle_uhid_output(struct uhid_event *event, void *user_data)
{
	struct uhid_output_req *output = &event->u.output;
	struct hid_device *dev = user_data;
	int fd, req_size;
	uint8_t *req;

	if (!dev->ctrl_io)
		return;

	req_size = 1 + output->size;
	req = malloc0(req_size);
	if (!req)
		return;

	req[0] = HID_MSG_SET_REPORT | output->rtype;
	memcpy(req + 1, output->data, req_size - 1);

	fd = g_io_channel_unix_get_fd(dev->ctrl_io);

	if (write(fd, req, req_size) < 0)
		error("hidhost: error writing set_report: %s (%d)",
							strerror(errno), errno);

	free(req);
}

static gboolean intr_io_watch_cb(GIOChannel *chan, gpointer data)
{
	struct hid_device *dev = data;
	uint8_t buf[UHID_DATA_MAX];
	struct uhid_event ev;
	int fd, bread, err;

	/* Wait uHID if not ready */
	if (!dev->uhid)
		return TRUE;

	fd = g_io_channel_unix_get_fd(chan);
	bread = read(fd, buf, sizeof(buf));
	if (bread < 0) {
		error("hidhost: read from interrupt failed: %s(%d)",
						strerror(errno), -errno);
		return TRUE;
	}

	/* Discard non-data packets */
	if (bread == 0 || buf[0] != (HID_MSG_DATA | HID_DATA_TYPE_INPUT))
		return TRUE;

	/* send data to uHID device skipping HIDP header byte */
	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_INPUT;
	ev.u.input.size = bread - 1;
	memcpy(ev.u.input.data, &buf[1], ev.u.input.size);

	err = bt_uhid_send(dev->uhid, &ev);
	if (err < 0)
		DBG("bt_uhid_send: %s (%d)", strerror(-err), -err);

	return TRUE;
}

static void bt_hid_notify_state(struct hid_device *dev, uint8_t state)
{
	struct hal_ev_hidhost_conn_state ev;
	char address[18];

	if (dev->state == state)
		return;

	dev->state = state;

	ba2str(&dev->dst, address);
	DBG("device %s state %u", address, state);

	bdaddr2android(&dev->dst, ev.bdaddr);
	ev.state = state;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST,
				HAL_EV_HIDHOST_CONN_STATE, sizeof(ev), &ev);
}

static gboolean intr_watch_cb(GIOChannel *chan, GIOCondition cond,
								gpointer data)
{
	struct hid_device *dev = data;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL))
		goto error;

	if (cond & G_IO_IN)
		return intr_io_watch_cb(chan, data);

error:
	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);

	/*
	 * Checking for ctrl_watch avoids a double g_io_channel_shutdown since
	 * it's likely that ctrl_watch_cb has been queued for dispatching in
	 * this mainloop iteration
	 */
	if ((cond & (G_IO_HUP | G_IO_ERR)) && dev->ctrl_watch)
		g_io_channel_shutdown(chan, TRUE, NULL);

	/* Close control channel */
	if (dev->ctrl_io && !(cond & G_IO_NVAL))
		g_io_channel_shutdown(dev->ctrl_io, TRUE, NULL);

	hid_device_remove(dev);

	return FALSE;
}

static void bt_hid_notify_proto_mode(struct hid_device *dev, uint8_t *buf,
									int len)
{
	struct hal_ev_hidhost_proto_mode ev;
	char address[18];

	ba2str(&dev->dst, address);
	DBG("device %s", address);

	memset(&ev, 0, sizeof(ev));
	bdaddr2android(&dev->dst, ev.bdaddr);

	if (buf[0] == HID_MSG_DATA) {
		ev.status = HAL_HIDHOST_STATUS_OK;
		if (buf[1] == HID_PROTO_REPORT)
			ev.mode = HAL_HIDHOST_REPORT_PROTOCOL;
		else if (buf[1] == HID_PROTO_BOOT)
			ev.mode = HAL_HIDHOST_BOOT_PROTOCOL;
		else
			ev.mode = HAL_HIDHOST_UNSUPPORTED_PROTOCOL;

	} else {
		ev.status = buf[0];
		ev.mode = HAL_HIDHOST_UNSUPPORTED_PROTOCOL;
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST,
				HAL_EV_HIDHOST_PROTO_MODE, sizeof(ev), &ev);
}

static void bt_hid_notify_get_report(struct hid_device *dev, uint8_t *buf,
									int len)
{
	struct hal_ev_hidhost_get_report *ev;
	int ev_len;
	char address[18];

	ba2str(&dev->dst, address);
	DBG("device %s", address);

	ev_len = sizeof(*ev);

	if (!((buf[0] == (HID_MSG_DATA | HID_DATA_TYPE_INPUT)) ||
			(buf[0] == (HID_MSG_DATA | HID_DATA_TYPE_OUTPUT)) ||
			(buf[0] == (HID_MSG_DATA | HID_DATA_TYPE_FEATURE)))) {
		ev = g_malloc0(ev_len);
		ev->status = buf[0];
		bdaddr2android(&dev->dst, ev->bdaddr);
		goto send;
	}

	/*
	 * Report porotocol mode reply contains id after hdr, in boot
	 * protocol mode id doesn't exist
	 */
	ev_len += (dev->boot_dev) ? (len - 1) : (len - 2);
	ev = g_malloc0(ev_len);
	ev->status = HAL_HIDHOST_STATUS_OK;
	bdaddr2android(&dev->dst, ev->bdaddr);

	/*
	 * Report porotocol mode reply contains id after hdr, in boot
	 * protocol mode id doesn't exist
	 */
	if (dev->boot_dev) {
		ev->len = len - 1;
		memcpy(ev->data, buf + 1, ev->len);
	} else {
		ev->len = len - 2;
		memcpy(ev->data, buf + 2, ev->len);
	}

send:
	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST,
				HAL_EV_HIDHOST_GET_REPORT, ev_len, ev);
	g_free(ev);
}

static void bt_hid_notify_handshake(struct hid_device *dev, uint8_t *buf,
									int len)
{
	struct hal_ev_hidhost_handshake ev;

	bdaddr2android(&dev->dst, ev.bdaddr);

	/* crop result code to handshake status range from HAL */
	ev.status = buf[0];
	if (ev.status > HAL_HIDHOST_HS_ERROR)
		ev.status = HAL_HIDHOST_HS_ERROR;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST,
				HAL_EV_HIDHOST_HANDSHAKE, sizeof(ev), &ev);
}

static void bt_hid_notify_virtual_unplug(struct hid_device *dev,
							uint8_t *buf, int len)
{
	struct hal_ev_hidhost_virtual_unplug ev;
	char address[18];

	ba2str(&dev->dst, address);
	DBG("device %s", address);
	bdaddr2android(&dev->dst, ev.bdaddr);

	ev.status = HAL_HIDHOST_GENERAL_ERROR;

	/* Wait either channels to HUP */
	if (dev->intr_io && dev->ctrl_io) {
		g_io_channel_shutdown(dev->intr_io, TRUE, NULL);
		g_io_channel_shutdown(dev->ctrl_io, TRUE, NULL);
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTING);
		ev.status = HAL_HIDHOST_STATUS_OK;
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST,
				HAL_EV_HIDHOST_VIRTUAL_UNPLUG, sizeof(ev), &ev);
}

static gboolean ctrl_io_watch_cb(GIOChannel *chan, gpointer data)
{
	struct hid_device *dev = data;
	int fd, bread;
	uint8_t buf[UHID_DATA_MAX];

	DBG("");

	fd = g_io_channel_unix_get_fd(chan);
	bread = read(fd, buf, sizeof(buf));
	if (bread < 0) {
		error("hidhost: read from control failed: %s(%d)",
						strerror(errno), -errno);
		return TRUE;
	}

	switch (dev->last_hid_msg) {
	case HID_MSG_GET_PROTOCOL:
	case HID_MSG_SET_PROTOCOL:
		bt_hid_notify_proto_mode(dev, buf, bread);
		break;
	case HID_MSG_GET_REPORT:
		bt_hid_notify_get_report(dev, buf, bread);
		break;
	}

	switch (buf[0] & HID_MSG_TYPE_MASK) {
	case HID_MSG_HANDSHAKE:
		bt_hid_notify_handshake(dev, buf, bread);
		break;
	case HID_MSG_CONTROL:
		if ((buf[0] & ~HID_MSG_TYPE_MASK) == HID_VIRTUAL_CABLE_UNPLUG)
			bt_hid_notify_virtual_unplug(dev, buf, bread);
		break;
	default:
		break;
	}

	/* reset msg type request */
	dev->last_hid_msg = 0;

	return TRUE;
}

static gboolean ctrl_watch_cb(GIOChannel *chan, GIOCondition cond,
								gpointer data)
{
	struct hid_device *dev = data;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL))
		goto error;

	if (cond & G_IO_IN)
		return ctrl_io_watch_cb(chan, data);

error:
	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);

	/*
	 * Checking for intr_watch avoids a double g_io_channel_shutdown since
	 * it's likely that intr_watch_cb has been queued for dispatching in
	 * this mainloop iteration
	 */
	if ((cond & (G_IO_HUP | G_IO_ERR)) && dev->intr_watch)
		g_io_channel_shutdown(chan, TRUE, NULL);

	if (dev->intr_io && !(cond & G_IO_NVAL))
		g_io_channel_shutdown(dev->intr_io, TRUE, NULL);

	hid_device_remove(dev);

	return FALSE;
}

static void bt_hid_set_info(struct hid_device *dev)
{
	struct hal_ev_hidhost_info ev;

	DBG("");

	bdaddr2android(&dev->dst, ev.bdaddr);
	ev.attr = 0; /* TODO: Check what is this field */
	ev.subclass = dev->subclass;
	ev.app_id = 0; /* TODO: Check what is this field */
	ev.vendor = dev->vendor;
	ev.product = dev->product;
	ev.version = dev->version;
	ev.country = dev->country;
	ev.descr_len = dev->rd_size;
	memset(ev.descr, 0, sizeof(ev.descr));
	memcpy(ev.descr, dev->rd_data, ev.descr_len);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_EV_HIDHOST_INFO,
							sizeof(ev), &ev);
}

static int uhid_create(struct hid_device *dev)
{
	struct uhid_event ev;
	int err;

	dev->uhid = bt_uhid_new_default();
	if (!dev->uhid) {
		err = -errno;
		error("hidhost: Failed to create bt_uhid instance");
		return err;
	}

	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_CREATE;
	strcpy((char *) ev.u.create.name, "bluez-input-device");
	ev.u.create.bus = BUS_BLUETOOTH;
	ev.u.create.vendor = dev->vendor;
	ev.u.create.product = dev->product;
	ev.u.create.version = dev->version;
	ev.u.create.country = dev->country;
	ev.u.create.rd_size = dev->rd_size;
	ev.u.create.rd_data = dev->rd_data;

	err = bt_uhid_send(dev->uhid, &ev);
	if (err < 0) {
		error("hidhost: Failed to create uHID device: %s",
							strerror(-err));
		bt_uhid_unref(dev->uhid);
		dev->uhid = NULL;
		return err;
	}

	bt_uhid_register(dev->uhid, UHID_OUTPUT, handle_uhid_output, dev);
	bt_hid_set_info(dev);

	return 0;
}

static void interrupt_connect_cb(GIOChannel *chan, GError *conn_err,
							gpointer user_data)
{
	struct hid_device *dev = user_data;
	uint8_t state;

	DBG("");

	if (conn_err) {
		error("hidhost: Failed to connect interrupt channel (%s)",
							conn_err->message);
		state = HAL_HIDHOST_STATE_FAILED;
		goto failed;
	}

	if (uhid_create(dev) < 0) {
		state = HAL_HIDHOST_STATE_NO_HID;
		goto failed;
	}

	dev->intr_watch = g_io_add_watch(dev->intr_io,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				intr_watch_cb, dev);

	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_CONNECTED);

	return;

failed:
	bt_hid_notify_state(dev, state);
	hid_device_remove(dev);
}

static void control_connect_cb(GIOChannel *chan, GError *conn_err,
							gpointer user_data)
{
	struct hid_device *dev = user_data;
	GError *err = NULL;

	DBG("");

	if (conn_err) {
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);
		error("hidhost: Failed to connect control channel (%s)",
							conn_err->message);
		goto failed;
	}

	/* Connect to the HID interrupt channel */
	dev->intr_io = bt_io_connect(interrupt_connect_cb, dev, NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_DEST_BDADDR, &dev->dst,
					BT_IO_OPT_PSM, L2CAP_PSM_HIDP_INTR,
					BT_IO_OPT_SEC_LEVEL, dev->sec_level,
					BT_IO_OPT_INVALID);
	if (!dev->intr_io) {
		error("hidhost: Failed to connect interrupt channel (%s)",
								err->message);
		g_error_free(err);
		goto failed;
	}

	dev->ctrl_watch = g_io_add_watch(dev->ctrl_io,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				ctrl_watch_cb, dev);

	return;

failed:
	hid_device_remove(dev);
}

static void hid_sdp_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct hid_device *dev = data;
	sdp_list_t *list;
	GError *gerr = NULL;

	DBG("");

	if (err < 0) {
		error("hidhost: Unable to get SDP record: %s", strerror(-err));
		goto fail;
	}

	if (!recs || !recs->data) {
		error("hidhost: No SDP records found");
		goto fail;
	}

	for (list = recs; list != NULL; list = list->next) {
		sdp_record_t *rec = list->data;
		sdp_data_t *data;

		data = sdp_data_get(rec, SDP_ATTR_HID_COUNTRY_CODE);
		if (data)
			dev->country = data->val.uint8;

		data = sdp_data_get(rec, SDP_ATTR_HID_DEVICE_SUBCLASS);
		if (data) {
			dev->subclass = data->val.uint8;

			/* Encryption is mandatory for keyboards */
			if (dev->subclass & 0x40)
				dev->sec_level = BT_IO_SEC_MEDIUM;
		}

		data = sdp_data_get(rec, SDP_ATTR_HID_BOOT_DEVICE);
		if (data)
			dev->boot_dev = data->val.uint8;

		data = sdp_data_get(rec, SDP_ATTR_HID_DESCRIPTOR_LIST);
		if (data) {
			if (!SDP_IS_SEQ(data->dtd))
				goto fail;

			/* First HIDDescriptor */
			data = data->val.dataseq;
			if (!SDP_IS_SEQ(data->dtd))
				goto fail;

			/* ClassDescriptorType */
			data = data->val.dataseq;
			if (data->dtd != SDP_UINT8)
				goto fail;

			/* ClassDescriptorData */
			data = data->next;
			if (!data || !SDP_IS_TEXT_STR(data->dtd))
				goto fail;

			dev->rd_size = data->unitSize;
			dev->rd_data = g_memdup(data->val.str, data->unitSize);
		}
	}

	if (dev->ctrl_io) {
		/* Raise the security level for this device if needed. */
		if ((dev->sec_level > BT_IO_SEC_LOW) &&
			!bt_io_set(dev->ctrl_io, &gerr,
					BT_IO_OPT_SEC_LEVEL, dev->sec_level,
					BT_IO_OPT_INVALID)) {
			error("hidhost: Cannot raise security level: %s",
								gerr->message);
			g_error_free(gerr);

			goto fail;
		}

		if (uhid_create(dev) < 0)
			goto fail;
		return;
	}

	dev->ctrl_io = bt_io_connect(control_connect_cb, dev, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_DEST_BDADDR, &dev->dst,
					BT_IO_OPT_PSM, L2CAP_PSM_HIDP_CTRL,
					BT_IO_OPT_SEC_LEVEL, dev->sec_level,
					BT_IO_OPT_INVALID);
	if (gerr) {
		error("hidhost: Failed to connect control channel (%s)",
								gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	return;

fail:
	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);
	hid_device_remove(dev);
}

static void hid_sdp_did_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct hid_device *dev = data;
	sdp_list_t *list;
	uuid_t uuid;

	DBG("");

	if (err < 0) {
		error("hidhost: Unable to get Device ID SDP record: %s",
								strerror(-err));
		goto fail;
	}

	if (!recs || !recs->data) {
		error("hidhost: No Device ID SDP records found");
		goto fail;
	}

	for (list = recs; list; list = list->next) {
		sdp_record_t *rec = list->data;
		sdp_data_t *data;

		data = sdp_data_get(rec, SDP_ATTR_VENDOR_ID);
		if (data)
			dev->vendor = data->val.uint16;

		data = sdp_data_get(rec, SDP_ATTR_PRODUCT_ID);
		if (data)
			dev->product = data->val.uint16;

		data = sdp_data_get(rec, SDP_ATTR_VERSION);
		if (data)
			dev->version = data->val.uint16;
	}

	sdp_uuid16_create(&uuid, HID_SVCLASS_ID);
	if (bt_search_service(&adapter_addr, &dev->dst, &uuid,
				hid_sdp_search_cb, dev, NULL, 0) < 0) {
		error("hidhost: Failed to search SDP details");
		goto fail;
	}

	return;

fail:
	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);
	hid_device_remove(dev);
}

static void hog_conn_cb(const bdaddr_t *addr, int err, void *attrib)
{
	GSList *l;
	struct hid_device *dev;

	l = g_slist_find_custom(devices, addr, device_cmp);
	dev = l ? l->data : NULL;

	if (err < 0) {
		if (!dev)
			return;
		if (dev->hog) {
			bt_hid_notify_state(dev,
						HAL_HIDHOST_STATE_DISCONNECTED);
			bt_hog_detach(dev->hog);
			return;
		}
		goto fail;
	}

	if (!dev)
		dev = hid_device_new(addr);

	if (!dev->hog) {
		/* TODO: Get device details and primary */
		dev->hog = bt_hog_new_default("bluez-input-device", dev->vendor,
					dev->product, dev->version, NULL);
		if (!dev->hog) {
			error("HoG: unable to create session");
			goto fail;
		}
	}

	if (!bt_hog_attach(dev->hog, attrib)) {
		error("HoG: unable to attach");
		goto fail;
	}

	if (!bt_gatt_set_security(addr, BT_IO_SEC_MEDIUM)) {
		error("Failed to set security level");
		goto fail;
	}

	DBG("");

	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_CONNECTED);

	if (!bt_gatt_add_autoconnect(hog_app, &dev->dst))
		error("hidhost: Could not add to autoconnect list");

	return;

fail:
	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);
	hid_device_remove(dev);
}

static bool hog_connect(struct hid_device *dev)
{
	DBG("");

	if (hog_app)
		return bt_gatt_connect_app(hog_app, &dev->dst);

	hog_app = bt_gatt_register_app(HOG_UUID, GATT_CLIENT, hog_conn_cb);
	if (!hog_app) {
		error("hidhost: bt_gatt_register_app failed");
		return false;
	}

	return bt_gatt_connect_app(hog_app, &dev->dst);
}

static void bt_hid_connect(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_connect *cmd = buf;
	struct hid_device *dev;
	uint8_t status;
	char addr[18];
	bdaddr_t dst;
	GSList *l;
	uuid_t uuid;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (l)
		dev = l->data;
	else
		dev = hid_device_new(&dst);

	if (dev->state != HAL_HIDHOST_STATE_DISCONNECTED)
		goto done;

	ba2str(&dev->dst, addr);
	DBG("connecting to %s", addr);

	if (bt_device_last_seen_bearer(&dev->dst) != BDADDR_BREDR) {
		if (!hog_connect(dev)) {
			status = HAL_STATUS_FAILED;
			hid_device_remove(dev);
			goto failed;
		}
		goto done;
	}

	sdp_uuid16_create(&uuid, PNP_INFO_SVCLASS_ID);
	if (bt_search_service(&adapter_addr, &dev->dst, &uuid,
				hid_sdp_did_search_cb, dev, NULL, 0) < 0) {
		error("hidhost: Failed to search DeviceID SDP details");
		hid_device_remove(dev);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

done:
	if (dev->state == HAL_HIDHOST_STATE_DISCONNECTED)
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_CONNECTING);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_CONNECT,
									status);
}

static bool hog_disconnect(struct hid_device *dev)
{
	DBG("");

	if (dev->state == HAL_HIDHOST_STATE_DISCONNECTED)
		return false;

	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTING);

	if (!bt_gatt_disconnect_app(hog_app, &dev->dst)) {
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTED);
		hid_device_remove(dev);
	}

	return true;
}

static void bt_hid_disconnect(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_disconnect *cmd = buf;
	struct hid_device *dev;
	uint8_t status;
	GSList *l;
	bdaddr_t dst;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;
	if (bt_is_device_le(&dst)) {
		if (!hog_disconnect(dev)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}
		goto done;
	}

	/* Wait either channels to HUP */
	if (dev->intr_io)
		g_io_channel_shutdown(dev->intr_io, TRUE, NULL);

	if (dev->ctrl_io)
		g_io_channel_shutdown(dev->ctrl_io, TRUE, NULL);

	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTING);


done:
	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_DISCONNECT,
									status);
}

static bool bt_hid_write_virtual_unplug(GIOChannel *chan)
{
	uint8_t hdr = HID_MSG_CONTROL | HID_VIRTUAL_CABLE_UNPLUG;
	int fd = g_io_channel_unix_get_fd(chan);

	if (write(fd, &hdr, sizeof(hdr)) == sizeof(hdr))
		return true;

	error("hidhost: Error writing virtual unplug command: %s (%d)",
							strerror(errno), errno);
	return false;
}

static void bt_hid_virtual_unplug(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_virtual_unplug *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	uint8_t status;
	bdaddr_t dst;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;

	if (!(dev->ctrl_io)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (!bt_hid_write_virtual_unplug(dev->ctrl_io)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/* Wait either channels to HUP */
	if (dev->intr_io)
		g_io_channel_shutdown(dev->intr_io, TRUE, NULL);

	if (dev->ctrl_io)
		g_io_channel_shutdown(dev->ctrl_io, TRUE, NULL);

	bt_hid_notify_state(dev, HAL_HIDHOST_STATE_DISCONNECTING);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST,
					HAL_OP_HIDHOST_VIRTUAL_UNPLUG, status);
}

static void bt_hid_info(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_set_info *cmd = buf;

	if (len != sizeof(*cmd) + cmd->descr_len) {
		error("Invalid hid set info size (%u bytes), terminating", len);
		raise(SIGTERM);
		return;
	}

	/*
	 * Data from hal_cmd_hidhost_set_info is usefull only when we create
	 * UHID device. Once device is created all the transactions will be
	 * done through the fd. There is no way to use this information
	 * once device is created with HID internals.
	 */
	DBG("Not supported");

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SET_INFO,
							HAL_STATUS_UNSUPPORTED);
}

static void bt_hid_get_protocol(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_get_protocol *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	bdaddr_t dst;
	int fd;
	uint8_t hdr;
	uint8_t status;

	DBG("");

	switch (cmd->mode) {
	case HAL_HIDHOST_REPORT_PROTOCOL:
	case HAL_HIDHOST_BOOT_PROTOCOL:
		break;
	default:
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;

	hdr = HID_MSG_GET_PROTOCOL | cmd->mode;
	fd = g_io_channel_unix_get_fd(dev->ctrl_io);

	if (write(fd, &hdr, sizeof(hdr)) < 0) {
		error("hidhost: Error writing device_get_protocol: %s (%d)",
						strerror(errno), errno);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev->last_hid_msg = HID_MSG_GET_PROTOCOL;

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST,
					HAL_OP_HIDHOST_GET_PROTOCOL, status);
}

static void bt_hid_set_protocol(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_set_protocol *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	bdaddr_t dst;
	int fd;
	uint8_t hdr;
	uint8_t status;

	DBG("");

	switch (cmd->mode) {
	case HAL_HIDHOST_REPORT_PROTOCOL:
	case HAL_HIDHOST_BOOT_PROTOCOL:
		break;
	default:
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;

	hdr = HID_MSG_SET_PROTOCOL | cmd->mode;
	fd = g_io_channel_unix_get_fd(dev->ctrl_io);

	if (write(fd, &hdr, sizeof(hdr)) < 0) {
		error("hidhost: error writing device_set_protocol: %s (%d)",
						strerror(errno), errno);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev->last_hid_msg = HID_MSG_SET_PROTOCOL;

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST,
					HAL_OP_HIDHOST_SET_PROTOCOL, status);
}

static void bt_hid_get_report(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_get_report *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	bdaddr_t dst;
	int fd;
	uint8_t *req;
	uint8_t req_size;
	uint8_t status;

	DBG("");

	switch (cmd->type) {
	case HAL_HIDHOST_INPUT_REPORT:
	case HAL_HIDHOST_OUTPUT_REPORT:
	case HAL_HIDHOST_FEATURE_REPORT:
		break;
	default:
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;
	req_size = (cmd->buf_size > 0) ? 4 : 2;
	req = g_try_malloc0(req_size);
	if (!req) {
		status = HAL_STATUS_NOMEM;
		goto failed;
	}

	req[0] = HID_MSG_GET_REPORT | cmd->type;
	req[1] = cmd->id;

	if (cmd->buf_size > 0) {
		req[0] = req[0] | HID_GET_REPORT_SIZE_FIELD;
		put_le16(cmd->buf_size, &req[2]);
	}

	fd = g_io_channel_unix_get_fd(dev->ctrl_io);

	if (write(fd, req, req_size) < 0) {
		error("hidhost: error writing hid_get_report: %s (%d)",
						strerror(errno), errno);
		g_free(req);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev->last_hid_msg = HID_MSG_GET_REPORT;
	g_free(req);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_GET_REPORT,
									status);
}

static void bt_hid_set_report(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_set_report *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	bdaddr_t dst;
	int fd;
	uint8_t *req = NULL;
	uint8_t req_size;
	uint8_t status;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid hid set report size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	switch (cmd->type) {
	case HAL_HIDHOST_INPUT_REPORT:
	case HAL_HIDHOST_OUTPUT_REPORT:
	case HAL_HIDHOST_FEATURE_REPORT:
		break;
	default:
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;

	if (!dev->ctrl_io && !dev->hog) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	req_size = 1 + (cmd->len / 2);
	req = g_try_malloc0(req_size);
	if (!req) {
		status = HAL_STATUS_NOMEM;
		goto failed;
	}

	req[0] = HID_MSG_SET_REPORT | cmd->type;
	/*
	 * Report data coming to HAL is in ascii format, HAL sends
	 * data in hex to daemon, so convert to binary.
	 */
	if (!hex2buf(cmd->data, req + 1, req_size - 1)) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	if (dev->hog) {
		if (bt_hog_send_report(dev->hog, req + 1, req_size - 1,
							cmd->type) < 0) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		goto done;
	}

	fd = g_io_channel_unix_get_fd(dev->ctrl_io);

	if (write(fd, req, req_size) < 0) {
		error("hidhost: error writing hid_set_report: %s (%d)",
						strerror(errno), errno);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev->last_hid_msg = HID_MSG_SET_REPORT;

done:
	status = HAL_STATUS_SUCCESS;

failed:
	g_free(req);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SET_REPORT,
									status);
}

static void bt_hid_send_data(const void *buf, uint16_t len)
{
	const struct hal_cmd_hidhost_send_data *cmd = buf;
	struct hid_device *dev;
	GSList *l;
	bdaddr_t dst;
	int fd;
	uint8_t *req = NULL;
	uint8_t req_size;
	uint8_t status;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid hid send data size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	android2bdaddr(&cmd->bdaddr, &dst);

	l = g_slist_find_custom(devices, &dst, device_cmp);
	if (!l) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	dev = l->data;

	if (!(dev->intr_io)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	req_size = 1 + (cmd->len / 2);
	req = g_try_malloc0(req_size);
	if (!req) {
		status = HAL_STATUS_NOMEM;
		goto failed;
	}

	req[0] = HID_MSG_DATA | HID_DATA_TYPE_OUTPUT;
	/*
	 * Report data coming to HAL is in ascii format, HAL sends
	 * data in hex to daemon, so convert to binary.
	 */
	if (!hex2buf(cmd->data, req + 1, req_size - 1)) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	fd = g_io_channel_unix_get_fd(dev->intr_io);

	if (write(fd, req, req_size) < 0) {
		error("hidhost: error writing data to HID device: %s (%d)",
						strerror(errno), errno);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	g_free(req);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SEND_DATA,
									status);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_HIDHOST_CONNECT */
	{ bt_hid_connect, false, sizeof(struct hal_cmd_hidhost_connect) },
	/* HAL_OP_HIDHOST_DISCONNECT */
	{ bt_hid_disconnect, false, sizeof(struct hal_cmd_hidhost_disconnect) },
	/* HAL_OP_HIDHOST_VIRTUAL_UNPLUG */
	{ bt_hid_virtual_unplug, false,
				sizeof(struct hal_cmd_hidhost_virtual_unplug) },
	/* HAL_OP_HIDHOST_SET_INFO */
	{ bt_hid_info, true, sizeof(struct hal_cmd_hidhost_set_info) },
	/* HAL_OP_HIDHOST_GET_PROTOCOL */
	{ bt_hid_get_protocol, false,
				sizeof(struct hal_cmd_hidhost_get_protocol) },
	/* HAL_OP_HIDHOST_SET_PROTOCOL */
	{ bt_hid_set_protocol, false,
				sizeof(struct hal_cmd_hidhost_get_protocol) },
	/* HAL_OP_HIDHOST_GET_REPORT */
	{ bt_hid_get_report, false, sizeof(struct hal_cmd_hidhost_get_report) },
	/* HAL_OP_HIDHOST_SET_REPORT */
	{ bt_hid_set_report, true, sizeof(struct hal_cmd_hidhost_set_report) },
	/* HAL_OP_HIDHOST_SEND_DATA */
	{ bt_hid_send_data, true, sizeof(struct hal_cmd_hidhost_send_data)  },
};

static void connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct hid_device *dev;
	bdaddr_t dst;
	char address[18];
	uint16_t psm;
	GError *gerr = NULL;
	GSList *l;
	uuid_t uuid;

	if (err) {
		error("hidhost: Connect failed (%s)", err->message);
		return;
	}

	bt_io_get(chan, &gerr,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_PSM, &psm,
			BT_IO_OPT_INVALID);
	if (gerr) {
		error("hidhost: Failed to read remote address (%s)",
								gerr->message);
		g_io_channel_shutdown(chan, TRUE, NULL);
		g_error_free(gerr);
		return;
	}

	ba2str(&dst, address);
	DBG("Incoming connection from %s on PSM %d", address, psm);

	if (!bt_device_is_bonded(&dst)) {
		warn("hidhost: Rejecting connection from unknown device %s",
								address);
		if (psm == L2CAP_PSM_HIDP_CTRL)
			bt_hid_write_virtual_unplug(chan);

		g_io_channel_shutdown(chan, TRUE, NULL);
		return;
	}

	switch (psm) {
	case L2CAP_PSM_HIDP_CTRL:
		l = g_slist_find_custom(devices, &dst, device_cmp);
		if (l)
			return;

		dev = hid_device_new(&dst);
		dev->ctrl_io = g_io_channel_ref(chan);

		sdp_uuid16_create(&uuid, PNP_INFO_SVCLASS_ID);
		if (bt_search_service(&adapter_addr, &dev->dst, &uuid,
				hid_sdp_did_search_cb, dev, NULL, 0) < 0) {
			error("hidhost: Failed to search DID SDP details");
			hid_device_remove(dev);
			return;
		}

		dev->ctrl_watch = g_io_add_watch(dev->ctrl_io,
					G_IO_HUP | G_IO_ERR | G_IO_NVAL,
					ctrl_watch_cb, dev);
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_CONNECTING);
		break;

	case L2CAP_PSM_HIDP_INTR:
		l = g_slist_find_custom(devices, &dst, device_cmp);
		if (!l)
			return;

		dev = l->data;
		dev->intr_io = g_io_channel_ref(chan);
		dev->intr_watch = g_io_add_watch(dev->intr_io,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				intr_watch_cb, dev);
		bt_hid_notify_state(dev, HAL_HIDHOST_STATE_CONNECTED);
		break;
	}
}

static void hid_unpaired_cb(const bdaddr_t *addr)
{
	GSList *l;
	struct hid_device *dev;
	char address[18];

	l = g_slist_find_custom(devices, addr, device_cmp);
	if (!l)
		return;

	dev = l->data;

	ba2str(addr, address);
	DBG("Unpaired device %s", address);

	if (hog_app)
		bt_gatt_remove_autoconnect(hog_app, addr);

	hid_device_remove(dev);
}

bool bt_hid_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode)
{
	GError *err = NULL;

	DBG("");

	if (!bt_unpaired_register(hid_unpaired_cb)) {
		error("hidhost: Could not register unpaired callback");
		return false;
	}

	bacpy(&adapter_addr, addr);

	ctrl_io = bt_io_listen(connect_cb, NULL, NULL, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_PSM, L2CAP_PSM_HIDP_CTRL,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	if (!ctrl_io) {
		error("hidhost: Failed to listen on control channel: %s",
								err->message);
		g_error_free(err);
		return false;
	}

	intr_io = bt_io_listen(connect_cb, NULL, NULL, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_PSM, L2CAP_PSM_HIDP_INTR,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	if (!intr_io) {
		error("hidhost: Failed to listen on interrupt channel: %s",
								err->message);
		g_error_free(err);

		g_io_channel_shutdown(ctrl_io, TRUE, NULL);
		g_io_channel_unref(ctrl_io);
		ctrl_io = NULL;

		return false;
	}

	hal_ipc = ipc;

	ipc_register(hal_ipc, HAL_SERVICE_ID_HIDHOST, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	return true;
}

void bt_hid_unregister(void)
{
	DBG("");

	if (hog_app > 0)
		bt_gatt_unregister_app(hog_app);

	g_slist_free_full(devices, hid_device_free);
	devices = NULL;

	if (ctrl_io) {
		g_io_channel_shutdown(ctrl_io, TRUE, NULL);
		g_io_channel_unref(ctrl_io);
		ctrl_io = NULL;
	}

	if (intr_io) {
		g_io_channel_shutdown(intr_io, TRUE, NULL);
		g_io_channel_unref(intr_io);
		intr_io = NULL;
	}

	ipc_unregister(hal_ipc, HAL_SERVICE_ID_HIDHOST);
	hal_ipc = NULL;

	bt_unpaired_unregister(hid_unpaired_cb);
}
