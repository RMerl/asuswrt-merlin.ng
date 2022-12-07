// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2017  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <ell/ell.h>

#include "lib/bluetooth.h"
#include "src/shared/btp.h"

#define BTP_MTU 512

struct btp_handler {
	unsigned int id;
	uint8_t service;
	uint8_t opcode;

	btp_cmd_func_t callback;
	void *user_data;
	btp_destroy_func_t destroy;
};

struct btp {
	struct l_io *io;

	struct l_queue *pending;
	bool writer_active;
	bool reader_active;

	struct l_queue *handlers;
	unsigned int next_handler;

	uint8_t buf[BTP_MTU];

	btp_disconnect_func_t disconnect_cb;
	void *disconnect_cb_data;
	btp_destroy_func_t disconnect_cb_data_destroy;
};


static struct l_io *btp_connect(const char *path)
{
	struct sockaddr_un addr;
	struct l_io *io;
	int sk;

	sk = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sk < 0)
		return NULL;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(sk);
		return NULL;
	}

	io = l_io_new(sk);
	if (!io) {
		close(sk);
		return NULL;
	}

	l_io_set_close_on_destroy(io, true);
	return io;
}

static void disconnect_handler(struct l_io *io, void *user_data)
{
	struct btp *btp = user_data;

	btp->disconnect_cb(btp, btp->disconnect_cb_data);
}

static void disconnect_handler_destroy(void *user_data)
{
	struct btp *btp = user_data;

	if (btp->disconnect_cb_data_destroy)
		btp->disconnect_cb_data_destroy(btp->disconnect_cb_data);
}

struct handler_match_data {
	uint8_t service;
	uint8_t opcode;
};

static bool handler_match(const void *a, const void *b)
{
	const struct btp_handler *handler = a;
	const struct handler_match_data *match = b;

	return (handler->service == match->service) &&
					(handler->opcode == match->opcode);
}

static bool can_read_data(struct l_io *io, void *user_data)
{
	struct handler_match_data match;
	struct btp *btp = user_data;
	struct btp_handler *handler;
	struct btp_hdr *hdr;
	ssize_t bytes_read;
	uint16_t data_len;

	bytes_read = read(l_io_get_fd(btp->io), btp->buf, sizeof(btp->buf));
	if (bytes_read < 0)
		return false;

	if ((size_t) bytes_read < sizeof(*hdr))
		return false;

	hdr = (void *)btp->buf;

	data_len = L_LE16_TO_CPU(hdr->data_len);

	if ((size_t) bytes_read < sizeof(*hdr) + data_len)
		return false;

	match.service = hdr->service;
	match.opcode = hdr->opcode;

	handler = l_queue_find(btp->handlers, handler_match, &match);
	if (handler) {
		handler->callback(hdr->index, hdr->data, data_len,
							handler->user_data);
		return false;
	}

	/* keep reader active if we sent error reply */
	btp_send_error(btp, match.service, hdr->index, BTP_ERROR_UNKNOWN_CMD);
	return true;
}

static void read_watch_destroy(void *user_data)
{
	struct btp *btp = user_data;

	btp->reader_active = false;
}

static void wakeup_reader(struct btp *btp)
{
	if (btp->reader_active)
		return;

	btp->reader_active = l_io_set_read_handler(btp->io, can_read_data, btp,
							read_watch_destroy);
}

struct btp *btp_new(const char *path)
{
	struct btp *btp;
	struct l_io *io;

	io = btp_connect(path);
	if (!io)
		return NULL;

	btp = l_new(struct btp, 1);
	btp->pending = l_queue_new();
	btp->handlers = l_queue_new();
	btp->io = io;
	btp->next_handler = 1;

	wakeup_reader(btp);

	return btp;
}

struct pending_message {
	size_t len;
	void *data;
	bool wakeup_read;
};

static void destroy_message(struct pending_message *msg)
{
	l_free(msg->data);
	l_free(msg);
}

void btp_cleanup(struct btp *btp)
{
	if (!btp)
		return;

	l_io_destroy(btp->io);
	l_queue_destroy(btp->pending, (l_queue_destroy_func_t)destroy_message);
	l_queue_destroy(btp->handlers, (l_queue_destroy_func_t)l_free);
	l_free(btp);
}

bool btp_set_disconnect_handler(struct btp *btp, btp_disconnect_func_t callback,
				void *user_data, btp_destroy_func_t destroy)
{
	if (callback) {
		if (!l_io_set_disconnect_handler(btp->io, disconnect_handler,
					btp, disconnect_handler_destroy))
			return false;
	} else {
		if (!l_io_set_disconnect_handler(btp->io, NULL, NULL, NULL))
			return false;
	}

	btp->disconnect_cb = callback;
	btp->disconnect_cb_data = user_data;
	btp->disconnect_cb_data_destroy = destroy;

	return true;
}

static bool can_write_data(struct l_io *io, void *user_data)
{
	struct btp *btp = user_data;
	struct pending_message *msg;

	msg = l_queue_pop_head(btp->pending);
	if (!msg)
		return false;

	if (msg->wakeup_read)
		wakeup_reader(btp);

	if (write(l_io_get_fd(btp->io), msg->data, msg->len) < 0) {
		l_error("Failed to send BTP message");
		destroy_message(msg);
		return false;
	}

	destroy_message(msg);

	return !l_queue_isempty(btp->pending);
}

static void write_watch_destroy(void *user_data)
{
	struct btp *btp = user_data;

	btp->writer_active = false;
}

static void wakeup_writer(struct btp *btp)
{
	if (l_queue_isempty(btp->pending))
		return;

	if (btp->writer_active)
		return;

	btp->writer_active = l_io_set_write_handler(btp->io, can_write_data,
						btp, write_watch_destroy);
}

bool btp_send_error(struct btp *btp, uint8_t service, uint8_t index,
								uint8_t status)
{
	struct btp_error rsp;

	rsp.status = status;

	return btp_send(btp, service, BTP_OP_ERROR, index, sizeof(rsp), &rsp);
}

bool btp_send(struct btp *btp, uint8_t service, uint8_t opcode, uint8_t index,
					uint16_t length, const void *param)
{
	struct btp_hdr *hdr;
	struct pending_message *msg;
	size_t len;

	if (!btp)
		return false;

	len = sizeof(*hdr) + length;
	hdr = l_malloc(len);
	if (!hdr)
		return false;

	hdr->service = service;
	hdr->opcode = opcode;
	hdr->index = index;
	hdr->data_len = L_CPU_TO_LE16(length);
	if (length)
		memcpy(hdr->data, param, length);

	msg = l_new(struct pending_message, 1);
	msg->len = len;
	msg->data = hdr;
	msg->wakeup_read = opcode < 0x80;

	l_queue_push_tail(btp->pending, msg);
	wakeup_writer(btp);

	return true;
}

unsigned int btp_register(struct btp *btp, uint8_t service, uint8_t opcode,
				btp_cmd_func_t callback, void *user_data,
				btp_destroy_func_t destroy)
{
	struct btp_handler *handler;

	handler = l_new(struct btp_handler, 1);

	handler->id = btp->next_handler++;
	handler->service = service;
	handler->opcode = opcode;
	handler->callback = callback;
	handler->user_data = user_data;
	handler->destroy = destroy;

	l_queue_push_tail(btp->handlers, handler);

	return handler->id;
}

static bool handler_match_by_id(const void *a, const void *b)
{
	const struct btp_handler *handler = a;
	unsigned int id = L_PTR_TO_UINT(b);

	return handler->id == id;
}

bool btp_unregister(struct btp *btp, unsigned int id)
{
	struct btp_handler *handler;

	handler = l_queue_remove_if(btp->handlers, handler_match_by_id,
							L_UINT_TO_PTR(id));
	if (!handler)
		return false;

	if (handler->destroy)
		handler->destroy(handler->user_data);

	l_free(handler);

	return true;
}

static bool handler_remove_by_service(void *a, void *b)
{
	struct btp_handler *handler = a;
	uint8_t service = L_PTR_TO_UINT(b);

	if (handler->service != service)
		return false;

	if (handler->destroy)
		handler->destroy(handler->user_data);

	l_free(handler);
	return true;
}

void btp_unregister_service(struct btp *btp, uint8_t service)
{
	l_queue_foreach_remove(btp->handlers, handler_remove_by_service,
							L_UINT_TO_PTR(service));
}
