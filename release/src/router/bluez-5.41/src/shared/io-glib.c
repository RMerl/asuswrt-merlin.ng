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

#include <errno.h>

#include <glib.h>

#include "src/shared/io.h"

struct io_watch {
	struct io *io;
	guint id;
	io_callback_func_t callback;
	io_destroy_func_t destroy;
	void *user_data;
};

struct io {
	int ref_count;
	GIOChannel *channel;
	struct io_watch *read_watch;
	struct io_watch *write_watch;
	struct io_watch *disconnect_watch;
};

static struct io *io_ref(struct io *io)
{
	if (!io)
		return NULL;

	__sync_fetch_and_add(&io->ref_count, 1);

	return io;
}

static void io_unref(struct io *io)
{
	if (!io)
		return;

	if (__sync_sub_and_fetch(&io->ref_count, 1))
		return;

	g_free(io);
}

struct io *io_new(int fd)
{
	struct io *io;

	if (fd < 0)
		return NULL;

	io = g_try_new0(struct io, 1);
	if (!io)
		return NULL;

	io->channel = g_io_channel_unix_new(fd);

	g_io_channel_set_encoding(io->channel, NULL, NULL);
	g_io_channel_set_buffered(io->channel, FALSE);

	g_io_channel_set_close_on_unref(io->channel, FALSE);

	return io_ref(io);
}

static void watch_destroy(void *user_data)
{
	struct io_watch *watch = user_data;
	struct io *io = watch->io;

	if (watch == io->read_watch)
		io->read_watch = NULL;
	else if (watch == io->write_watch)
		io->write_watch = NULL;
	else if (watch == io->disconnect_watch)
		io->disconnect_watch = NULL;

	if (watch->destroy)
		watch->destroy(watch->user_data);

	io_unref(watch->io);
	g_free(watch);
}

void io_destroy(struct io *io)
{
	if (!io)
		return;

	if (io->read_watch) {
		g_source_remove(io->read_watch->id);
		io->read_watch = NULL;
	}

	if (io->write_watch) {
		g_source_remove(io->write_watch->id);
		io->write_watch = NULL;
	}

	if (io->disconnect_watch) {
		g_source_remove(io->disconnect_watch->id);
		io->disconnect_watch = NULL;
	}

	g_io_channel_unref(io->channel);
	io->channel = NULL;

	io_unref(io);
}

int io_get_fd(struct io *io)
{
	if (!io)
		return -ENOTCONN;

	return g_io_channel_unix_get_fd(io->channel);
}

bool io_set_close_on_destroy(struct io *io, bool do_close)
{
	if (!io)
		return false;

	if (do_close)
		g_io_channel_set_close_on_unref(io->channel, TRUE);
	else
		g_io_channel_set_close_on_unref(io->channel, FALSE);

	return true;
}

static gboolean watch_callback(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	struct io_watch *watch = user_data;
	bool result, destroy;

	destroy = watch == watch->io->disconnect_watch;

	if (!destroy && (cond & (G_IO_ERR | G_IO_NVAL)))
		return FALSE;

	if (watch->callback)
		result = watch->callback(watch->io, watch->user_data);
	else
		result = false;

	return result ? TRUE : FALSE;
}

static struct io_watch *watch_new(struct io *io, GIOCondition cond,
				io_callback_func_t callback, void *user_data,
				io_destroy_func_t destroy)
{
	struct io_watch *watch;

	watch = g_try_new0(struct io_watch, 1);
	if (!watch)
		return NULL;

	watch->io = io_ref(io);
	watch->callback = callback;
	watch->destroy = destroy;
	watch->user_data = user_data;

	watch->id = g_io_add_watch_full(io->channel, G_PRIORITY_DEFAULT,
						cond | G_IO_ERR | G_IO_NVAL,
						watch_callback, watch,
						watch_destroy);
	if (watch->id == 0) {
		watch_destroy(watch);
		return NULL;
	}

	return watch;
}

static bool io_set_handler(struct io *io, GIOCondition cond,
				io_callback_func_t callback, void *user_data,
				io_destroy_func_t destroy)
{
	struct io_watch **watch;

	if (!io)
		return false;

	switch (cond) {
	case G_IO_IN:
		watch = &io->read_watch;
		break;
	case G_IO_OUT:
		watch = &io->write_watch;
		break;
	case G_IO_HUP:
		watch = &io->disconnect_watch;
		break;
	case G_IO_PRI:
	case G_IO_ERR:
	case G_IO_NVAL:
	default:
		return false;
	}

	if (*watch) {
		g_source_remove((*watch)->id);
		*watch = NULL;
	}

	if (!callback)
		return true;

	*watch = watch_new(io, cond, callback, user_data, destroy);
	if (!*watch)
		return false;

	return true;
}

bool io_set_read_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	return io_set_handler(io, G_IO_IN, callback, user_data, destroy);
}

bool io_set_write_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	return io_set_handler(io, G_IO_OUT, callback, user_data, destroy);
}

bool io_set_disconnect_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	return io_set_handler(io, G_IO_HUP, callback, user_data, destroy);
}

ssize_t io_send(struct io *io, const struct iovec *iov, int iovcnt)
{
	int fd;
	ssize_t ret;

	if (!io || !io->channel)
		return -ENOTCONN;

	fd = io_get_fd(io);

	do {
		ret = writev(fd, iov, iovcnt);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return -errno;

	return ret;
}

bool io_shutdown(struct io *io)
{
	if (!io || !io->channel)
		return false;

	return g_io_channel_shutdown(io->channel, TRUE, NULL)
							== G_IO_STATUS_NORMAL;
}
