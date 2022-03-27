/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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
#include <fcntl.h>

#include "src/shared/io.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/uhid.h"

#define UHID_DEVICE_FILE "/dev/uhid"

struct bt_uhid {
	int ref_count;
	struct io *io;
	unsigned int notify_id;
	struct queue *notify_list;
};

struct uhid_notify {
	unsigned int id;
	uint32_t event;
	bt_uhid_callback_t func;
	void *user_data;
};

static void uhid_free(struct bt_uhid *uhid)
{
	if (uhid->io)
		io_destroy(uhid->io);

	if (uhid->notify_list)
		queue_destroy(uhid->notify_list, free);

	free(uhid);
}

static void notify_handler(void *data, void *user_data)
{
	struct uhid_notify *notify = data;
	struct uhid_event *ev = user_data;

	if (notify->event != ev->type)
		return;

	if (notify->func)
		notify->func(ev, notify->user_data);
}

static bool uhid_read_handler(struct io *io, void *user_data)
{
	struct bt_uhid *uhid = user_data;
	int fd;
	ssize_t len;
	struct uhid_event ev;

	fd = io_get_fd(io);
	if (fd < 0)
		return false;

	memset(&ev, 0, sizeof(ev));

	len = read(fd, &ev, sizeof(ev));
	if (len < 0)
		return false;

	if ((size_t) len < sizeof(ev.type))
		return false;

	queue_foreach(uhid->notify_list, notify_handler, &ev);

	return true;
}

struct bt_uhid *bt_uhid_new_default(void)
{
	struct bt_uhid *uhid;
	int fd;

	fd = open(UHID_DEVICE_FILE, O_RDWR | O_CLOEXEC);
	if (fd < 0)
		return NULL;

	uhid = bt_uhid_new(fd);
	if (!uhid) {
		close(fd);
		return NULL;
	}

	io_set_close_on_destroy(uhid->io, true);

	return uhid;
}

struct bt_uhid *bt_uhid_new(int fd)
{
	struct bt_uhid *uhid;

	uhid = new0(struct bt_uhid, 1);
	uhid->io = io_new(fd);
	if (!uhid->io)
		goto failed;

	uhid->notify_list = queue_new();

	if (!io_set_read_handler(uhid->io, uhid_read_handler, uhid, NULL))
		goto failed;

	return bt_uhid_ref(uhid);

failed:
	uhid_free(uhid);
	return NULL;
}

struct bt_uhid *bt_uhid_ref(struct bt_uhid *uhid)
{
	if (!uhid)
		return NULL;

	__sync_fetch_and_add(&uhid->ref_count, 1);

	return uhid;
}

void bt_uhid_unref(struct bt_uhid *uhid)
{
	if (!uhid)
		return;

	if (__sync_sub_and_fetch(&uhid->ref_count, 1))
		return;

	uhid_free(uhid);
}

bool bt_uhid_set_close_on_unref(struct bt_uhid *uhid, bool do_close)
{
	if (!uhid || !uhid->io)
		return false;

	io_set_close_on_destroy(uhid->io, do_close);

	return true;
}

unsigned int bt_uhid_register(struct bt_uhid *uhid, uint32_t event,
				bt_uhid_callback_t func, void *user_data)
{
	struct uhid_notify *notify;

	if (!uhid)
		return 0;

	notify = new0(struct uhid_notify, 1);
	notify->id = uhid->notify_id++;
	notify->event = event;
	notify->func = func;
	notify->user_data = user_data;

	if (!queue_push_tail(uhid->notify_list, notify)) {
		free(notify);
		return 0;
	}

	return notify->id;
}

static bool match_notify_id(const void *a, const void *b)
{
	const struct uhid_notify *notify = a;
	unsigned int id = PTR_TO_UINT(b);

	return notify->id == id;
}

bool bt_uhid_unregister(struct bt_uhid *uhid, unsigned int id)
{
	struct uhid_notify *notify;

	if (!uhid || !id)
		return false;

	notify = queue_remove_if(uhid->notify_list, match_notify_id,
							UINT_TO_PTR(id));
	if (!notify)
		return false;

	free(notify);
	return true;
}

int bt_uhid_send(struct bt_uhid *uhid, const struct uhid_event *ev)
{
	ssize_t len;
	struct iovec iov;

	if (!uhid->io)
		return -ENOTCONN;

	iov.iov_base = (void *) ev;
	iov.iov_len = sizeof(*ev);

	len = io_send(uhid->io, &iov, 1);
	if (len < 0)
		return -errno;

	/* uHID kernel driver does not handle partial writes */
	return len != sizeof(*ev) ? -EIO : 0;
}
