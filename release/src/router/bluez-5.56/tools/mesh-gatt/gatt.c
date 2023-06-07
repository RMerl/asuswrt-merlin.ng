// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wordexp.h>

#include <glib.h>

#include "src/shared/io.h"
#include "src/shared/shell.h"
#include "gdbus/gdbus.h"
#include "lib/bluetooth.h"
#include "lib/uuid.h"

#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/util.h"
#include "tools/mesh-gatt/gatt.h"
#include "tools/mesh-gatt/prov.h"
#include "tools/mesh-gatt/net.h"

#define MESH_PROV_DATA_OUT_UUID_STR	"00002adc-0000-1000-8000-00805f9b34fb"
#define MESH_PROXY_DATA_OUT_UUID_STR	"00002ade-0000-1000-8000-00805f9b34fb"

static struct io *write_io;
static uint16_t write_mtu;

static struct io *notify_io;
static uint16_t notify_mtu;

struct write_data {
	GDBusProxy *proxy;
	void *user_data;
	struct iovec iov;
	GDBusReturnFunction cb;
	uint8_t *gatt_data;
	uint8_t gatt_len;
};

struct notify_data {
	GDBusProxy *proxy;
	bool enable;
	GDBusReturnFunction cb;
	void *user_data;
};

bool mesh_gatt_is_child(GDBusProxy *proxy, GDBusProxy *parent,
			const char *name)
{
	DBusMessageIter iter;
	const char *parent_path;

	if (!parent)
		return FALSE;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return FALSE;

	dbus_message_iter_get_basic(&iter, &parent_path);

	if (!strcmp(parent_path, g_dbus_proxy_get_path(parent)))
		return TRUE;
	else
		return FALSE;
}

/* Refactor this once actual MTU is available */
#define GATT_MTU	23

static void write_data_free(void *user_data)
{
	struct write_data *data = user_data;

	g_free(data->gatt_data);
	free(data);
}

uint16_t mesh_gatt_sar(uint8_t **pkt, uint16_t size)
{
	const uint8_t *data = *pkt;
	uint8_t gatt_hdr = *data++;
	uint8_t type = gatt_hdr & GATT_TYPE_MASK;
	static uint8_t gatt_size;
	static uint8_t gatt_pkt[67];

	print_byte_array("GATT-RX:\t", *pkt, size);
	if (size < 1) {
		gatt_pkt[0] = GATT_TYPE_INVALID;
		/* TODO: Disconnect GATT per last paragraph sec 6.6 */
		return 0;
	}

	size--;

	switch (gatt_hdr & GATT_SAR_MASK) {
	case GATT_SAR_FIRST:
		gatt_size = 1;
		gatt_pkt[0] = type;
		/* TODO: Start Proxy Timeout */
		/* fall through */

	case GATT_SAR_CONTINUE:
		if (gatt_pkt[0] != type ||
				gatt_size + size > MAX_GATT_SIZE) {

			/* Invalidate packet and return failure */
			gatt_pkt[0] = GATT_TYPE_INVALID;
			/* TODO: Disconnect GATT per last paragraph sec 6.6 */
			return 0;
		}

		memcpy(gatt_pkt + gatt_size, data, size);
		gatt_size += size;

		/* We are good to this point, but incomplete */
		return 0;

	default:
	case GATT_SAR_COMPLETE:
		gatt_size = 1;
		gatt_pkt[0] = type;

		/* fall through */

	case GATT_SAR_LAST:
		if (gatt_pkt[0] != type ||
				gatt_size + size > MAX_GATT_SIZE) {

			/* Invalidate packet and return failure */
			gatt_pkt[0] = GATT_TYPE_INVALID;
			/* Disconnect GATT per last paragraph sec 6.6 */
			return 0;
		}

		memcpy(gatt_pkt + gatt_size, data, size);
		gatt_size += size;
		*pkt = gatt_pkt;
		return gatt_size;
	}
}

static int sock_send(struct io *io, struct iovec *iov, size_t iovlen)
{
	struct msghdr msg;
	int ret;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = iovlen;

	ret = sendmsg(io_get_fd(io), &msg, MSG_NOSIGNAL);
	if (ret < 0) {
		ret = -errno;
		bt_shell_printf("sendmsg: %s", strerror(-ret));
	}

	return ret;
}

static bool sock_write(struct io *io, void *user_data)
{
	struct write_data *data = user_data;
	struct iovec iov[2];
	uint8_t sar;
	uint8_t max_len;

	if (data == NULL)
		return true;

	max_len = write_mtu ? write_mtu - 3 - 1 : GATT_MTU - 3 - 1;
	print_byte_array("GATT-TX:\t", data->gatt_data, data->gatt_len);

	sar = data->gatt_data[0];

	data->iov.iov_base = data->gatt_data + 1;
	data->iov.iov_len--;

	sar = data->gatt_data[0] & GATT_TYPE_MASK;
	data->gatt_len--;

	if (data->gatt_len > max_len) {
		sar |= GATT_SAR_FIRST;
		data->iov.iov_len = max_len;
	}

	iov[0].iov_base = &sar;
	iov[0].iov_len = sizeof(sar);

	while (1) {
		int err;

		iov[1] = data->iov;

		err = sock_send(io, iov, 2);
		if (err < 0) {
			write_data_free(data);
			return false;
		}

		switch (sar & GATT_SAR_MASK) {
		case GATT_SAR_FIRST:
		case GATT_SAR_CONTINUE:
			data->gatt_len -= max_len;
			data->iov.iov_base = data->iov.iov_base + max_len;

			sar &= GATT_TYPE_MASK;
			if (max_len < data->gatt_len) {
				data->iov.iov_len = max_len;
				sar |= GATT_SAR_CONTINUE;
			} else {
				data->iov.iov_len = data->gatt_len;
				sar |= GATT_SAR_LAST;
			}

			break;

		default:
			if(data->cb)
				data->cb(NULL, data->user_data);
			write_data_free(data);
			return true;
		}
	}
}

static void write_io_destroy(void)
{
	io_destroy(write_io);
	write_io = NULL;
	write_mtu = 0;
}

static void notify_io_destroy(void)
{
	io_destroy(notify_io);
	notify_io = NULL;
	notify_mtu = 0;
}

static bool sock_hup(struct io *io, void *user_data)
{
	bt_shell_printf("%s closed\n", io == notify_io ? "Notify" : "Write");

	if (io == notify_io)
		notify_io_destroy();
	else
		write_io_destroy();

	return false;
}

static struct io *sock_io_new(int fd)
{
	struct io *io;

	io = io_new(fd);

	io_set_close_on_destroy(io, true);

	io_set_disconnect_handler(io, sock_hup, NULL, NULL);

	return io;
}

static void acquire_write_reply(DBusMessage *message, void *user_data)
{
	struct write_data *data = user_data;
	DBusError error;
	int fd;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		dbus_error_free(&error);
		bt_shell_printf("Failed to write\n");
		write_data_free(data);
		return;
	}

	if ((dbus_message_get_args(message, NULL, DBUS_TYPE_UNIX_FD, &fd,
					DBUS_TYPE_UINT16, &write_mtu,
					DBUS_TYPE_INVALID) == false)) {
		bt_shell_printf("Invalid AcquireWrite response\n");
		return;
	}

	bt_shell_printf("AcquireWrite success: fd %d MTU %u\n", fd, write_mtu);

	write_io = sock_io_new(fd);

	sock_write(write_io, data);
}

static void acquire_setup(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

bool mesh_gatt_write(GDBusProxy *proxy, uint8_t *buf, uint16_t len,
			GDBusReturnFunction cb, void *user_data)
{
	struct write_data *data;

	if (!buf || !len)
		return false;

	if (len > 69)
		return false;

	data = g_new0(struct write_data, 1);
	if (!data)
		return false;

	/* TODO: should keep in queue in case we need to cancel write? */

	data->gatt_len = len;
	data->gatt_data = g_memdup(buf, len);
	data->gatt_data[0] &= GATT_TYPE_MASK;
	data->iov.iov_base = data->gatt_data;
	data->iov.iov_len = len;
	data->proxy = proxy;
	data->user_data = user_data;
	data->cb = cb;

	if (write_io)
		return sock_write(write_io, data);

	if (g_dbus_proxy_method_call(proxy, "AcquireWrite",
				acquire_setup, acquire_write_reply,
				data, NULL) == FALSE) {
		bt_shell_printf("Failed to AcquireWrite\n");
		write_data_free(data);
		return false;
	}
	return true;
}

static void notify_reply(DBusMessage *message, void *user_data)
{
	struct notify_data *data = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to %s notify: %s\n",
				data->enable ? "start" : "stop", error.name);
		dbus_error_free(&error);
		goto done;
	}

	bt_shell_printf("Notify %s\n", data->enable ? "started" : "stopped");

done:
	if (data->cb)
		data->cb(message, data->user_data);

	g_free(data);
}

static bool sock_read(struct io *io, bool prov, void *user_data)
{
	struct mesh_node *node = user_data;
	struct msghdr msg;
	struct iovec iov;
	uint8_t buf[512];
	uint8_t *res;
	int fd = io_get_fd(io);
	ssize_t len, len_sar;

	if (io != notify_io)
		return true;

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while ((len = recvmsg(fd, &msg, MSG_DONTWAIT))) {
		if (len < 0) {
			if (errno == EAGAIN)
				break;
			return false;
		}

		res = buf;
		len_sar = mesh_gatt_sar(&res, len);
		if (len_sar) {
			if (prov)
				prov_data_ready(node, res, len_sar);
			else
				net_data_ready(res, len_sar);
		}
	}

	/* When socket is orderly closed, then recvmsg returns 0 */
	if (len == 0)
		return false;

	return true;
}

static bool sock_read_prov(struct io *io, void *user_data)
{
	return sock_read(io, true, user_data);
}

static bool sock_read_proxy(struct io *io, void *user_data)
{
	return sock_read(io, false, user_data);
}

static void acquire_notify_reply(DBusMessage *message, void *user_data)
{
	struct notify_data *data = user_data;
	DBusMessageIter iter;
	DBusError error;
	int fd;
	const char *uuid;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		dbus_error_free(&error);
		if (g_dbus_proxy_method_call(data->proxy, "StartNotify", NULL,
					notify_reply, data, NULL) == FALSE) {
			bt_shell_printf("Failed to StartNotify\n");
			g_free(data);
		}
		return;
	}

	if (notify_io) {
		io_destroy(notify_io);
		notify_io = NULL;
	}

	notify_mtu = 0;

	if ((dbus_message_get_args(message, NULL, DBUS_TYPE_UNIX_FD, &fd,
					DBUS_TYPE_UINT16, &notify_mtu,
					DBUS_TYPE_INVALID) == false)) {
		if (g_dbus_proxy_method_call(data->proxy, "StartNotify", NULL,
					notify_reply, data, NULL) == FALSE) {
			bt_shell_printf("Failed to StartNotify\n");
			g_free(data);
		}
		return;
	}

	bt_shell_printf("AcquireNotify success: fd %d MTU %u\n", fd, notify_mtu);

	if (g_dbus_proxy_get_property(data->proxy, "UUID", &iter) == FALSE)
		goto done;

	notify_io = sock_io_new(fd);

	dbus_message_iter_get_basic(&iter, &uuid);

	if (!bt_uuid_strcmp(uuid, MESH_PROV_DATA_OUT_UUID_STR))
		io_set_read_handler(notify_io, sock_read_prov, data->user_data,
									NULL);
	else if (!bt_uuid_strcmp(uuid, MESH_PROXY_DATA_OUT_UUID_STR))
		io_set_read_handler(notify_io, sock_read_proxy, data->user_data,
									NULL);

done:
	if (data->cb)
		data->cb(message, data->user_data);

	g_free(data);
}

bool mesh_gatt_notify(GDBusProxy *proxy, bool enable, GDBusReturnFunction cb,
			void *user_data)
{
	struct notify_data *data;
	DBusMessageIter iter;
	const char *method;
	GDBusSetupFunction setup = NULL;

	data = g_new0(struct notify_data, 1);
	data->proxy = proxy;
	data->enable = enable;
	data->cb = cb;
	data->user_data = user_data;

	if (enable == TRUE) {
		if (g_dbus_proxy_get_property(proxy, "NotifyAcquired", &iter)) {
			method = "AcquireNotify";
			cb = acquire_notify_reply;
			setup = acquire_setup;
		} else {
			method = "StartNotify";
			cb = notify_reply;
		}
	} else {
		if (notify_io) {
			notify_io_destroy();
			if (cb)
				cb(NULL, user_data);
			return true;
		} else {
			method = "StopNotify";
			cb = notify_reply;
		}
	}

	if (g_dbus_proxy_method_call(proxy, method, setup, cb,
					data, NULL) == FALSE) {
		bt_shell_printf("Failed to %s\n", method);
		return false;
	}
	return true;
}
