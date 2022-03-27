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

#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <glib.h>

#include "ipc-common.h"
#include "ipc.h"
#include "src/log.h"

struct service_handler {
	const struct ipc_handler *handler;
	uint8_t size;
};

struct ipc {
	struct service_handler *services;
	int service_max;

	const char *path;
	size_t size;

	GIOChannel *cmd_io;
	guint cmd_watch;

	bool notifications;
	GIOChannel *notif_io;
	guint notif_watch;

	ipc_disconnect_cb disconnect_cb;
	void *disconnect_cb_data;
};

static void ipc_disconnect(struct ipc *ipc, bool in_cleanup)
{
	if (ipc->cmd_watch) {
		g_source_remove(ipc->cmd_watch);
		ipc->cmd_watch = 0;
	}

	if (ipc->cmd_io) {
		g_io_channel_shutdown(ipc->cmd_io, TRUE, NULL);
		g_io_channel_unref(ipc->cmd_io);
		ipc->cmd_io = NULL;
	}

	if (ipc->notif_watch) {
		g_source_remove(ipc->notif_watch);
		ipc->notif_watch = 0;
	}

	if (ipc->notif_io) {
		g_io_channel_shutdown(ipc->notif_io, TRUE, NULL);
		g_io_channel_unref(ipc->notif_io);
		ipc->notif_io = NULL;
	}

	if (in_cleanup)
		return;

	if (ipc->disconnect_cb)
		ipc->disconnect_cb(ipc->disconnect_cb_data);
}

static int ipc_handle_msg(struct service_handler *handlers, size_t max_index,
						const void *buf, ssize_t len)
{
	const struct ipc_hdr *msg = buf;
	const struct ipc_handler *handler;

	if (len < (ssize_t) sizeof(*msg)) {
		DBG("message too small (%zd bytes)", len);
		return -EBADMSG;
	}

	if (len != (ssize_t) (sizeof(*msg) + msg->len)) {
		DBG("message malformed (%zd bytes)", len);
		return -EBADMSG;
	}

	/* if service is valid */
	if (msg->service_id > max_index) {
		DBG("unknown service (0x%x)", msg->service_id);
		return -EOPNOTSUPP;
	}

	/* if service is registered */
	if (!handlers[msg->service_id].handler) {
		DBG("service not registered (0x%x)", msg->service_id);
		return -EOPNOTSUPP;
	}

	/* if opcode is valid */
	if (msg->opcode == IPC_OP_STATUS ||
			msg->opcode > handlers[msg->service_id].size) {
		DBG("invalid opcode 0x%x for service 0x%x", msg->opcode,
							msg->service_id);
		return -EOPNOTSUPP;
	}

	/* opcode is table offset + 1 */
	handler = &handlers[msg->service_id].handler[msg->opcode - 1];

	/* if payload size is valid */
	if ((handler->var_len && handler->data_len > msg->len) ||
			(!handler->var_len && handler->data_len != msg->len)) {
		DBG("invalid size for opcode 0x%x service 0x%x",
						msg->opcode, msg->service_id);
		return -EMSGSIZE;
	}

	handler->handler(msg->payload, msg->len);

	return 0;
}

static gboolean cmd_watch_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct ipc *ipc = user_data;

	char buf[IPC_MTU];
	ssize_t ret;
	int fd, err;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		info("IPC: command socket closed");

		ipc->cmd_watch = 0;
		goto fail;
	}

	fd = g_io_channel_unix_get_fd(io);

	ret = read(fd, buf, sizeof(buf));
	if (ret < 0) {
		error("IPC: command read failed (%s)", strerror(errno));
		goto fail;
	}

	err = ipc_handle_msg(ipc->services, ipc->service_max, buf, ret);
	if (err < 0) {
		error("IPC: failed to handle message (%s)", strerror(-err));
		goto fail;
	}

	return TRUE;

fail:
	ipc_disconnect(ipc, false);

	return FALSE;
}

static gboolean notif_watch_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct ipc *ipc = user_data;

	info("IPC: notification socket closed");

	ipc->notif_watch = 0;

	ipc_disconnect(ipc, false);

	return FALSE;
}

static GIOChannel *ipc_connect(const char *path, size_t size,
					GIOFunc connect_cb, void *user_data)
{
	struct sockaddr_un addr;
	GIOCondition cond;
	GIOChannel *io;
	int sk;

	sk = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (sk < 0) {
		error("IPC: failed to create socket: %d (%s)", errno,
							strerror(errno));
		return NULL;
	}

	io = g_io_channel_unix_new(sk);

	g_io_channel_set_close_on_unref(io, TRUE);
	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, path, size);

	connect(sk, (struct sockaddr *) &addr, sizeof(addr));

	cond = G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	g_io_add_watch(io, cond, connect_cb, user_data);

	return io;
}

static gboolean notif_connect_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct ipc *ipc = user_data;

	DBG("");

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		error("IPC: notification socket connect failed");

		ipc_disconnect(ipc, false);

		return FALSE;
	}

	cond = G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	ipc->notif_watch = g_io_add_watch(io, cond, notif_watch_cb, ipc);

	cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	ipc->cmd_watch = g_io_add_watch(ipc->cmd_io, cond, cmd_watch_cb, ipc);

	info("IPC: successfully connected (with notifications)");

	return FALSE;
}

static gboolean cmd_connect_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct ipc *ipc = user_data;

	DBG("");

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		error("IPC: command socket connect failed");
		ipc_disconnect(ipc, false);

		return FALSE;
	}

	if (ipc->notifications) {
		ipc->notif_io = ipc_connect(ipc->path, ipc->size,
							notif_connect_cb, ipc);
		if (!ipc->notif_io)
			ipc_disconnect(ipc, false);

		return FALSE;
	}

	cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	ipc->cmd_watch = g_io_add_watch(ipc->cmd_io, cond, cmd_watch_cb, ipc);

	info("IPC: successfully connected (without notifications)");

	return FALSE;
}

struct ipc *ipc_init(const char *path, size_t size, int max_service_id,
					bool notifications,
					ipc_disconnect_cb cb, void *cb_data)
{
	struct ipc *ipc;

	ipc = g_new0(struct ipc, 1);

	ipc->services = g_new0(struct service_handler, max_service_id + 1);
	ipc->service_max = max_service_id;

	ipc->path = path;
	ipc->size = size;

	ipc->notifications = notifications;

	ipc->cmd_io = ipc_connect(path, size, cmd_connect_cb, ipc);
	if (!ipc->cmd_io) {
		g_free(ipc->services);
		g_free(ipc);
		return NULL;
	}

	ipc->disconnect_cb = cb;
	ipc->disconnect_cb_data = cb_data;

	return ipc;
}

void ipc_cleanup(struct ipc *ipc)
{
	ipc_disconnect(ipc, true);

	g_free(ipc->services);
	g_free(ipc);
}

static void ipc_send(int sk, uint8_t service_id, uint8_t opcode, uint16_t len,
							void *param, int fd)
{
	struct msghdr msg;
	struct iovec iv[2];
	struct ipc_hdr m;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;

	memset(&msg, 0, sizeof(msg));
	memset(&m, 0, sizeof(m));
	memset(cmsgbuf, 0, sizeof(cmsgbuf));

	m.service_id = service_id;
	m.opcode = opcode;
	m.len = len;

	iv[0].iov_base = &m;
	iv[0].iov_len = sizeof(m);

	iv[1].iov_base = param;
	iv[1].iov_len = len;

	msg.msg_iov = iv;
	msg.msg_iovlen = 2;

	if (fd >= 0) {
		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof(cmsgbuf);

		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));

		/* Initialize the payload */
		memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
	}

	if (sendmsg(sk, &msg, 0) < 0) {
		error("IPC send failed :%s", strerror(errno));

		/* TODO disconnect IPC here when this function becomes static */
		raise(SIGTERM);
	}
}

void ipc_send_rsp(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
								uint8_t status)
{
	struct ipc_status s;
	int sk;

	sk = g_io_channel_unix_get_fd(ipc->cmd_io);

	if (status == IPC_STATUS_SUCCESS) {
		ipc_send(sk, service_id, opcode, 0, NULL, -1);
		return;
	}

	s.code = status;

	ipc_send(sk, service_id, IPC_OP_STATUS, sizeof(s), &s, -1);
}

void ipc_send_rsp_full(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
					uint16_t len, void *param, int fd)
{
	ipc_send(g_io_channel_unix_get_fd(ipc->cmd_io), service_id, opcode, len,
								param, fd);
}

void ipc_send_notif(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
						uint16_t len, void *param)
{
	return ipc_send_notif_with_fd(ipc, service_id, opcode, len, param, -1);
}

void ipc_send_notif_with_fd(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
					uint16_t len, void *param, int fd)
{
	if (!ipc || !ipc->notif_io)
		return;

	ipc_send(g_io_channel_unix_get_fd(ipc->notif_io), service_id, opcode,
								len, param, fd);
}

void ipc_register(struct ipc *ipc, uint8_t service,
			const struct ipc_handler *handlers, uint8_t size)
{
	if (service > ipc->service_max)
		return;

	ipc->services[service].handler = handlers;
	ipc->services[service].size = size;
}

void ipc_unregister(struct ipc *ipc, uint8_t service)
{
	if (service > ipc->service_max)
		return;

	ipc->services[service].handler = NULL;
	ipc->services[service].size = 0;
}
