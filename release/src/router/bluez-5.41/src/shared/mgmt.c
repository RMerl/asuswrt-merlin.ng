/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "lib/hci.h"

#include "src/shared/io.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "src/shared/mgmt.h"

struct mgmt {
	int ref_count;
	int fd;
	bool close_on_unref;
	struct io *io;
	bool writer_active;
	struct queue *request_queue;
	struct queue *reply_queue;
	struct queue *pending_list;
	struct queue *notify_list;
	unsigned int next_request_id;
	unsigned int next_notify_id;
	bool need_notify_cleanup;
	bool in_notify;
	void *buf;
	uint16_t len;
	mgmt_debug_func_t debug_callback;
	mgmt_destroy_func_t debug_destroy;
	void *debug_data;
};

struct mgmt_request {
	unsigned int id;
	uint16_t opcode;
	uint16_t index;
	void *buf;
	uint16_t len;
	mgmt_request_func_t callback;
	mgmt_destroy_func_t destroy;
	void *user_data;
};

struct mgmt_notify {
	unsigned int id;
	uint16_t event;
	uint16_t index;
	bool removed;
	mgmt_notify_func_t callback;
	mgmt_destroy_func_t destroy;
	void *user_data;
};

static void destroy_request(void *data)
{
	struct mgmt_request *request = data;

	if (request->destroy)
		request->destroy(request->user_data);

	free(request->buf);
	free(request);
}

static bool match_request_id(const void *a, const void *b)
{
	const struct mgmt_request *request = a;
	unsigned int id = PTR_TO_UINT(b);

	return request->id == id;
}

static bool match_request_index(const void *a, const void *b)
{
	const struct mgmt_request *request = a;
	uint16_t index = PTR_TO_UINT(b);

	return request->index == index;
}

static void destroy_notify(void *data)
{
	struct mgmt_notify *notify = data;

	if (notify->destroy)
		notify->destroy(notify->user_data);

	free(notify);
}

static bool match_notify_id(const void *a, const void *b)
{
	const struct mgmt_notify *notify = a;
	unsigned int id = PTR_TO_UINT(b);

	return notify->id == id;
}

static bool match_notify_index(const void *a, const void *b)
{
	const struct mgmt_notify *notify = a;
	uint16_t index = PTR_TO_UINT(b);

	return notify->index == index;
}

static bool match_notify_removed(const void *a, const void *b)
{
	const struct mgmt_notify *notify = a;

	return notify->removed;
}

static void mark_notify_removed(void *data , void *user_data)
{
	struct mgmt_notify *notify = data;
	uint16_t index = PTR_TO_UINT(user_data);

	if (notify->index == index || index == MGMT_INDEX_NONE)
		notify->removed = true;
}

static void write_watch_destroy(void *user_data)
{
	struct mgmt *mgmt = user_data;

	mgmt->writer_active = false;
}

static bool send_request(struct mgmt *mgmt, struct mgmt_request *request)
{
	struct iovec iov;
	ssize_t ret;

	iov.iov_base = request->buf;
	iov.iov_len = request->len;

	ret = io_send(mgmt->io, &iov, 1);
	if (ret < 0) {
		util_debug(mgmt->debug_callback, mgmt->debug_data,
				"write failed: %s", strerror(-ret));
		if (request->callback)
			request->callback(MGMT_STATUS_FAILED, 0, NULL,
							request->user_data);
		destroy_request(request);
		return false;
	}

	util_debug(mgmt->debug_callback, mgmt->debug_data,
				"[0x%04x] command 0x%04x",
				request->index, request->opcode);

	util_hexdump('<', request->buf, ret, mgmt->debug_callback,
							mgmt->debug_data);

	queue_push_tail(mgmt->pending_list, request);

	return true;
}

static bool can_write_data(struct io *io, void *user_data)
{
	struct mgmt *mgmt = user_data;
	struct mgmt_request *request;
	bool can_write;

	request = queue_pop_head(mgmt->reply_queue);
	if (!request) {
		/* only reply commands can jump the queue */
		if (!queue_isempty(mgmt->pending_list))
			return false;

		request = queue_pop_head(mgmt->request_queue);
		if (!request)
			return false;

		can_write = false;
	} else {
		/* allow multiple replies to jump the queue */
		can_write = !queue_isempty(mgmt->reply_queue);
	}

	if (!send_request(mgmt, request))
		return true;

	return can_write;
}

static void wakeup_writer(struct mgmt *mgmt)
{
	if (!queue_isempty(mgmt->pending_list)) {
		/* only queued reply commands trigger wakeup */
		if (queue_isempty(mgmt->reply_queue))
			return;
	}

	if (mgmt->writer_active)
		return;

	mgmt->writer_active = true;

	io_set_write_handler(mgmt->io, can_write_data, mgmt,
						write_watch_destroy);
}

struct opcode_index {
	uint16_t opcode;
	uint16_t index;
};

static bool match_request_opcode_index(const void *a, const void *b)
{
	const struct mgmt_request *request = a;
	const struct opcode_index *match = b;

	return request->opcode == match->opcode &&
					request->index == match->index;
}

static void request_complete(struct mgmt *mgmt, uint8_t status,
					uint16_t opcode, uint16_t index,
					uint16_t length, const void *param)
{
	struct opcode_index match = { .opcode = opcode, .index = index };
	struct mgmt_request *request;

	request = queue_remove_if(mgmt->pending_list,
					match_request_opcode_index, &match);
	if (request) {
		if (request->callback)
			request->callback(status, length, param,
							request->user_data);

		destroy_request(request);
	}

	wakeup_writer(mgmt);
}

struct event_index {
	uint16_t event;
	uint16_t index;
	uint16_t length;
	const void *param;
};

static void notify_handler(void *data, void *user_data)
{
	struct mgmt_notify *notify = data;
	struct event_index *match = user_data;

	if (notify->removed)
		return;

	if (notify->event != match->event)
		return;

	if (notify->index != match->index && notify->index != MGMT_INDEX_NONE)
		return;

	if (notify->callback)
		notify->callback(match->index, match->length, match->param,
							notify->user_data);
}

static void process_notify(struct mgmt *mgmt, uint16_t event, uint16_t index,
					uint16_t length, const void *param)
{
	struct event_index match = { .event = event, .index = index,
					.length = length, .param = param };

	mgmt->in_notify = true;

	queue_foreach(mgmt->notify_list, notify_handler, &match);

	mgmt->in_notify = false;

	if (mgmt->need_notify_cleanup) {
		queue_remove_all(mgmt->notify_list, match_notify_removed,
							NULL, destroy_notify);
		mgmt->need_notify_cleanup = false;
	}
}

static bool can_read_data(struct io *io, void *user_data)
{
	struct mgmt *mgmt = user_data;
	struct mgmt_hdr *hdr;
	struct mgmt_ev_cmd_complete *cc;
	struct mgmt_ev_cmd_status *cs;
	ssize_t bytes_read;
	uint16_t opcode, event, index, length;

	bytes_read = read(mgmt->fd, mgmt->buf, mgmt->len);
	if (bytes_read < 0)
		return false;

	util_hexdump('>', mgmt->buf, bytes_read,
				mgmt->debug_callback, mgmt->debug_data);

	if (bytes_read < MGMT_HDR_SIZE)
		return true;

	hdr = mgmt->buf;
	event = btohs(hdr->opcode);
	index = btohs(hdr->index);
	length = btohs(hdr->len);

	if (bytes_read < length + MGMT_HDR_SIZE)
		return true;

	mgmt_ref(mgmt);

	switch (event) {
	case MGMT_EV_CMD_COMPLETE:
		cc = mgmt->buf + MGMT_HDR_SIZE;
		opcode = btohs(cc->opcode);

		util_debug(mgmt->debug_callback, mgmt->debug_data,
				"[0x%04x] command 0x%04x complete: 0x%02x",
						index, opcode, cc->status);

		request_complete(mgmt, cc->status, opcode, index, length - 3,
						mgmt->buf + MGMT_HDR_SIZE + 3);
		break;
	case MGMT_EV_CMD_STATUS:
		cs = mgmt->buf + MGMT_HDR_SIZE;
		opcode = btohs(cs->opcode);

		util_debug(mgmt->debug_callback, mgmt->debug_data,
				"[0x%04x] command 0x%02x status: 0x%02x",
						index, opcode, cs->status);

		request_complete(mgmt, cs->status, opcode, index, 0, NULL);
		break;
	default:
		util_debug(mgmt->debug_callback, mgmt->debug_data,
				"[0x%04x] event 0x%04x", index, event);

		process_notify(mgmt, event, index, length,
						mgmt->buf + MGMT_HDR_SIZE);
		break;
	}

	mgmt_unref(mgmt);

	return true;
}

struct mgmt *mgmt_new(int fd)
{
	struct mgmt *mgmt;

	if (fd < 0)
		return NULL;

	mgmt = new0(struct mgmt, 1);
	mgmt->fd = fd;
	mgmt->close_on_unref = false;

	mgmt->len = 512;
	mgmt->buf = malloc(mgmt->len);
	if (!mgmt->buf) {
		free(mgmt);
		return NULL;
	}

	mgmt->io = io_new(fd);
	if (!mgmt->io) {
		free(mgmt->buf);
		free(mgmt);
		return NULL;
	}

	mgmt->request_queue = queue_new();
	mgmt->reply_queue = queue_new();
	mgmt->pending_list = queue_new();
	mgmt->notify_list = queue_new();

	if (!io_set_read_handler(mgmt->io, can_read_data, mgmt, NULL)) {
		queue_destroy(mgmt->notify_list, NULL);
		queue_destroy(mgmt->pending_list, NULL);
		queue_destroy(mgmt->reply_queue, NULL);
		queue_destroy(mgmt->request_queue, NULL);
		io_destroy(mgmt->io);
		free(mgmt->buf);
		free(mgmt);
		return NULL;
	}

	mgmt->writer_active = false;

	return mgmt_ref(mgmt);
}

struct mgmt *mgmt_new_default(void)
{
	struct mgmt *mgmt;
	union {
		struct sockaddr common;
		struct sockaddr_hci hci;
	} addr;
	int fd;

	fd = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK,
								BTPROTO_HCI);
	if (fd < 0)
		return NULL;

	memset(&addr, 0, sizeof(addr));
	addr.hci.hci_family = AF_BLUETOOTH;
	addr.hci.hci_dev = HCI_DEV_NONE;
	addr.hci.hci_channel = HCI_CHANNEL_CONTROL;

	if (bind(fd, &addr.common, sizeof(addr.hci)) < 0) {
		close(fd);
		return NULL;
	}

	mgmt = mgmt_new(fd);
	if (!mgmt) {
		close(fd);
		return NULL;
	}

	mgmt->close_on_unref = true;

	return mgmt;
}

struct mgmt *mgmt_ref(struct mgmt *mgmt)
{
	if (!mgmt)
		return NULL;

	__sync_fetch_and_add(&mgmt->ref_count, 1);

	return mgmt;
}

void mgmt_unref(struct mgmt *mgmt)
{
	if (!mgmt)
		return;

	if (__sync_sub_and_fetch(&mgmt->ref_count, 1))
		return;

	mgmt_unregister_all(mgmt);
	mgmt_cancel_all(mgmt);

	queue_destroy(mgmt->reply_queue, NULL);
	queue_destroy(mgmt->request_queue, NULL);

	io_set_write_handler(mgmt->io, NULL, NULL, NULL);
	io_set_read_handler(mgmt->io, NULL, NULL, NULL);

	io_destroy(mgmt->io);
	mgmt->io = NULL;

	if (mgmt->close_on_unref)
		close(mgmt->fd);

	if (mgmt->debug_destroy)
		mgmt->debug_destroy(mgmt->debug_data);

	free(mgmt->buf);
	mgmt->buf = NULL;

	if (!mgmt->in_notify) {
		queue_destroy(mgmt->notify_list, NULL);
		queue_destroy(mgmt->pending_list, NULL);
		free(mgmt);
		return;
	}
}

bool mgmt_set_debug(struct mgmt *mgmt, mgmt_debug_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	if (!mgmt)
		return false;

	if (mgmt->debug_destroy)
		mgmt->debug_destroy(mgmt->debug_data);

	mgmt->debug_callback = callback;
	mgmt->debug_destroy = destroy;
	mgmt->debug_data = user_data;

	return true;
}

bool mgmt_set_close_on_unref(struct mgmt *mgmt, bool do_close)
{
	if (!mgmt)
		return false;

	mgmt->close_on_unref = do_close;

	return true;
}

static struct mgmt_request *create_request(uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	struct mgmt_request *request;
	struct mgmt_hdr *hdr;

	if (!opcode)
		return NULL;

	if (length > 0 && !param)
		return NULL;

	request = new0(struct mgmt_request, 1);
	request->len = length + MGMT_HDR_SIZE;
	request->buf = malloc(request->len);
	if (!request->buf) {
		free(request);
		return NULL;
	}

	if (length > 0)
		memcpy(request->buf + MGMT_HDR_SIZE, param, length);

	hdr = request->buf;
	hdr->opcode = htobs(opcode);
	hdr->index = htobs(index);
	hdr->len = htobs(length);

	request->opcode = opcode;
	request->index = index;

	request->callback = callback;
	request->destroy = destroy;
	request->user_data = user_data;

	return request;
}

unsigned int mgmt_send(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	struct mgmt_request *request;

	if (!mgmt)
		return 0;

	request = create_request(opcode, index, length, param,
					callback, user_data, destroy);
	if (!request)
		return 0;

	if (mgmt->next_request_id < 1)
		mgmt->next_request_id = 1;

	request->id = mgmt->next_request_id++;

	if (!queue_push_tail(mgmt->request_queue, request)) {
		free(request->buf);
		free(request);
		return 0;
	}

	wakeup_writer(mgmt);

	return request->id;
}

unsigned int mgmt_send_nowait(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	struct mgmt_request *request;

	if (!mgmt)
		return 0;

	request = create_request(opcode, index, length, param,
					callback, user_data, destroy);
	if (!request)
		return 0;

	if (mgmt->next_request_id < 1)
		mgmt->next_request_id = 1;

	request->id = mgmt->next_request_id++;

	if (!send_request(mgmt, request))
		return 0;

	return request->id;
}

unsigned int mgmt_reply(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	struct mgmt_request *request;

	if (!mgmt)
		return 0;

	request = create_request(opcode, index, length, param,
					callback, user_data, destroy);
	if (!request)
		return 0;

	if (mgmt->next_request_id < 1)
		mgmt->next_request_id = 1;

	request->id = mgmt->next_request_id++;

	if (!queue_push_tail(mgmt->reply_queue, request)) {
		free(request->buf);
		free(request);
		return 0;
	}

	wakeup_writer(mgmt);

	return request->id;
}

bool mgmt_cancel(struct mgmt *mgmt, unsigned int id)
{
	struct mgmt_request *request;

	if (!mgmt || !id)
		return false;

	request = queue_remove_if(mgmt->request_queue, match_request_id,
							UINT_TO_PTR(id));
	if (request)
		goto done;

	request = queue_remove_if(mgmt->reply_queue, match_request_id,
							UINT_TO_PTR(id));
	if (request)
		goto done;

	request = queue_remove_if(mgmt->pending_list, match_request_id,
							UINT_TO_PTR(id));
	if (!request)
		return false;

done:
	destroy_request(request);

	wakeup_writer(mgmt);

	return true;
}

bool mgmt_cancel_index(struct mgmt *mgmt, uint16_t index)
{
	if (!mgmt)
		return false;

	queue_remove_all(mgmt->request_queue, match_request_index,
					UINT_TO_PTR(index), destroy_request);
	queue_remove_all(mgmt->reply_queue, match_request_index,
					UINT_TO_PTR(index), destroy_request);
	queue_remove_all(mgmt->pending_list, match_request_index,
					UINT_TO_PTR(index), destroy_request);

	return true;
}

bool mgmt_cancel_all(struct mgmt *mgmt)
{
	if (!mgmt)
		return false;

	queue_remove_all(mgmt->pending_list, NULL, NULL, destroy_request);
	queue_remove_all(mgmt->reply_queue, NULL, NULL, destroy_request);
	queue_remove_all(mgmt->request_queue, NULL, NULL, destroy_request);

	return true;
}

unsigned int mgmt_register(struct mgmt *mgmt, uint16_t event, uint16_t index,
				mgmt_notify_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy)
{
	struct mgmt_notify *notify;

	if (!mgmt || !event)
		return 0;

	notify = new0(struct mgmt_notify, 1);
	notify->event = event;
	notify->index = index;

	notify->callback = callback;
	notify->destroy = destroy;
	notify->user_data = user_data;

	if (mgmt->next_notify_id < 1)
		mgmt->next_notify_id = 1;

	notify->id = mgmt->next_notify_id++;

	if (!queue_push_tail(mgmt->notify_list, notify)) {
		free(notify);
		return 0;
	}

	return notify->id;
}

bool mgmt_unregister(struct mgmt *mgmt, unsigned int id)
{
	struct mgmt_notify *notify;

	if (!mgmt || !id)
		return false;

	notify = queue_remove_if(mgmt->notify_list, match_notify_id,
							UINT_TO_PTR(id));
	if (!notify)
		return false;

	if (!mgmt->in_notify) {
		destroy_notify(notify);
		return true;
	}

	notify->removed = true;
	mgmt->need_notify_cleanup = true;

	return true;
}

bool mgmt_unregister_index(struct mgmt *mgmt, uint16_t index)
{
	if (!mgmt)
		return false;

	if (mgmt->in_notify) {
		queue_foreach(mgmt->notify_list, mark_notify_removed,
							UINT_TO_PTR(index));
		mgmt->need_notify_cleanup = true;
	} else
		queue_remove_all(mgmt->notify_list, match_notify_index,
					UINT_TO_PTR(index), destroy_notify);

	return true;
}

bool mgmt_unregister_all(struct mgmt *mgmt)
{
	if (!mgmt)
		return false;

	if (mgmt->in_notify) {
		queue_foreach(mgmt->notify_list, mark_notify_removed,
						UINT_TO_PTR(MGMT_INDEX_NONE));
		mgmt->need_notify_cleanup = true;
	} else
		queue_remove_all(mgmt->notify_list, NULL, NULL, destroy_notify);

	return true;
}
