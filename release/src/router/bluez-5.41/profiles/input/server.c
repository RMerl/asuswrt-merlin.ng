/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/log.h"
#include "src/uuid-helper.h"
#include "btio/btio.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"

#include "device.h"
#include "server.h"

struct confirm_data {
	bdaddr_t dst;
	GIOChannel *io;
};

static GSList *servers = NULL;
struct input_server {
	bdaddr_t src;
	GIOChannel *ctrl;
	GIOChannel *intr;
	struct confirm_data *confirm;
};

static int server_cmp(gconstpointer s, gconstpointer user_data)
{
	const struct input_server *server = s;
	const bdaddr_t *src = user_data;

	return bacmp(&server->src, src);
}

struct sixaxis_data {
	GIOChannel *chan;
	uint16_t psm;
};

static void sixaxis_sdp_cb(struct btd_device *dev, int err, void *user_data)
{
	struct sixaxis_data *data = user_data;
	const bdaddr_t *src;

	DBG("err %d (%s)", err, strerror(-err));

	if (err < 0)
		goto fail;

	src = btd_adapter_get_address(device_get_adapter(dev));

	if (input_device_set_channel(src, device_get_address(dev), data->psm,
								data->chan) < 0)
		goto fail;

	g_io_channel_unref(data->chan);
	g_free(data);

	return;

fail:
	g_io_channel_shutdown(data->chan, TRUE, NULL);
	g_io_channel_unref(data->chan);
	g_free(data);
}

static void sixaxis_browse_sdp(const bdaddr_t *src, const bdaddr_t *dst,
						GIOChannel *chan, uint16_t psm)
{
	struct btd_device *device;
	struct sixaxis_data *data;

	device = btd_adapter_find_device(adapter_find(src), dst, BDADDR_BREDR);
	if (!device)
		return;

	data = g_new0(struct sixaxis_data, 1);
	data->chan = g_io_channel_ref(chan);
	data->psm = psm;

	if (psm == L2CAP_PSM_HIDP_CTRL)
		device_discover_services(device);

	device_wait_for_svc_complete(device, sixaxis_sdp_cb, data);
}

static bool dev_is_sixaxis(const bdaddr_t *src, const bdaddr_t *dst)
{
	struct btd_device *device;
	uint16_t vid, pid;

	device = btd_adapter_find_device(adapter_find(src), dst, BDADDR_BREDR);
	if (!device)
		return false;

	vid = btd_device_get_vendor(device);
	pid = btd_device_get_product(device);

	/* DualShock 3 */
	if (vid == 0x054c && pid == 0x0268)
		return true;

	/* DualShock 4 */
	if (vid == 0x054c && pid == 0x05c4)
		return true;

	/* Navigation Controller */
	if (vid == 0x054c && pid == 0x042f)
		return true;

	return false;
}

static void connect_event_cb(GIOChannel *chan, GError *err, gpointer data)
{
	uint16_t psm;
	bdaddr_t src, dst;
	char address[18];
	GError *gerr = NULL;
	int ret;

	if (err) {
		error("%s", err->message);
		return;
	}

	bt_io_get(chan, &gerr,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_PSM, &psm,
			BT_IO_OPT_INVALID);
	if (gerr) {
		error("%s", gerr->message);
		g_error_free(gerr);
		g_io_channel_shutdown(chan, TRUE, NULL);
		return;
	}

	ba2str(&dst, address);
	DBG("Incoming connection from %s on PSM %d", address, psm);

	ret = input_device_set_channel(&src, &dst, psm, chan);
	if (ret == 0)
		return;

	if (ret == -ENOENT && dev_is_sixaxis(&src, &dst)) {
		sixaxis_browse_sdp(&src, &dst, chan, psm);
		return;
	}

	error("Refusing input device connect: %s (%d)", strerror(-ret), -ret);

	/* Send unplug virtual cable to unknown devices */
	if (ret == -ENOENT && psm == L2CAP_PSM_HIDP_CTRL) {
		unsigned char unplug = 0x15;
		int sk = g_io_channel_unix_get_fd(chan);
		if (write(sk, &unplug, sizeof(unplug)) < 0)
			error("Unable to send virtual cable unplug");
	}

	g_io_channel_shutdown(chan, TRUE, NULL);
}

static void auth_callback(DBusError *derr, void *user_data)
{
	struct input_server *server = user_data;
	struct confirm_data *confirm = server->confirm;
	GError *err = NULL;

	if (derr) {
		error("Access denied: %s", derr->message);
		goto reject;
	}

	if (!input_device_exists(&server->src, &confirm->dst) &&
				!dev_is_sixaxis(&server->src, &confirm->dst))
		return;

	if (!bt_io_accept(confirm->io, connect_event_cb, server, NULL, &err)) {
		error("bt_io_accept: %s", err->message);
		g_error_free(err);
		goto reject;
	}

	g_io_channel_unref(confirm->io);
	g_free(server->confirm);
	server->confirm = NULL;

	return;

reject:
	g_io_channel_shutdown(confirm->io, TRUE, NULL);
	g_io_channel_unref(confirm->io);
	server->confirm = NULL;
	input_device_close_channels(&server->src, &confirm->dst);
	g_free(confirm);
}

static void confirm_event_cb(GIOChannel *chan, gpointer user_data)
{
	struct input_server *server = user_data;
	bdaddr_t src, dst;
	GError *err = NULL;
	char addr[18];
	guint ret;

	DBG("");

	bt_io_get(chan, &err,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		g_io_channel_shutdown(chan, TRUE, NULL);
		return;
	}

	ba2str(&dst, addr);

	if (server->confirm) {
		error("Refusing connection from %s: setup in progress", addr);
		goto drop;
	}

	if (!input_device_exists(&src, &dst) && !dev_is_sixaxis(&src, &dst)) {
		error("Refusing connection from %s: unknown device", addr);
		goto drop;
	}

	server->confirm = g_new0(struct confirm_data, 1);
	server->confirm->io = g_io_channel_ref(chan);
	bacpy(&server->confirm->dst, &dst);

	ret = btd_request_authorization(&src, &dst, HID_UUID,
					auth_callback, server);
	if (ret != 0)
		return;

	error("input: authorization for device %s failed", addr);

	g_io_channel_unref(server->confirm->io);
	g_free(server->confirm);
	server->confirm = NULL;

drop:
	input_device_close_channels(&src, &dst);
	g_io_channel_shutdown(chan, TRUE, NULL);
}

int server_start(const bdaddr_t *src)
{
	struct input_server *server;
	GError *err = NULL;

	server = g_new0(struct input_server, 1);
	bacpy(&server->src, src);

	server->ctrl = bt_io_listen(connect_event_cb, NULL,
				server, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_PSM, L2CAP_PSM_HIDP_CTRL,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	if (!server->ctrl) {
		error("Failed to listen on control channel");
		g_error_free(err);
		g_free(server);
		return -1;
	}

	server->intr = bt_io_listen(NULL, confirm_event_cb,
				server, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_PSM, L2CAP_PSM_HIDP_INTR,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	if (!server->intr) {
		error("Failed to listen on interrupt channel");
		g_io_channel_unref(server->ctrl);
		g_error_free(err);
		g_free(server);
		return -1;
	}

	servers = g_slist_append(servers, server);

	return 0;
}

void server_stop(const bdaddr_t *src)
{
	struct input_server *server;
	GSList *l;

	l = g_slist_find_custom(servers, src, server_cmp);
	if (!l)
		return;

	server = l->data;

	g_io_channel_shutdown(server->intr, TRUE, NULL);
	g_io_channel_unref(server->intr);

	g_io_channel_shutdown(server->ctrl, TRUE, NULL);
	g_io_channel_unref(server->ctrl);

	servers = g_slist_remove(servers, server);
	g_free(server);
}
