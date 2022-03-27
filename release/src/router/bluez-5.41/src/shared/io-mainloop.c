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

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/io.h"

struct io {
	int ref_count;
	int fd;
	uint32_t events;
	bool close_on_destroy;
	io_callback_func_t read_callback;
	io_destroy_func_t read_destroy;
	void *read_data;
	io_callback_func_t write_callback;
	io_destroy_func_t write_destroy;
	void *write_data;
	io_callback_func_t disconnect_callback;
	io_destroy_func_t disconnect_destroy;
	void *disconnect_data;
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

	free(io);
}

static void io_cleanup(void *user_data)
{
	struct io *io = user_data;

	if (io->write_destroy)
		io->write_destroy(io->write_data);

	if (io->read_destroy)
		io->read_destroy(io->read_data);

	if (io->disconnect_destroy)
		io->disconnect_destroy(io->disconnect_data);

	if (io->close_on_destroy)
		close(io->fd);

	io->fd = -1;
}

static void io_callback(int fd, uint32_t events, void *user_data)
{
	struct io *io = user_data;

	io_ref(io);

	if ((events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))) {
		io->read_callback = NULL;
		io->write_callback = NULL;

		if (!io->disconnect_callback) {
			mainloop_remove_fd(io->fd);
			io_unref(io);
			return;
		}

		if (!io->disconnect_callback(io, io->disconnect_data)) {
			if (io->disconnect_destroy)
				io->disconnect_destroy(io->disconnect_data);

			io->disconnect_callback = NULL;
			io->disconnect_destroy = NULL;
			io->disconnect_data = NULL;

			io->events &= ~EPOLLRDHUP;

			mainloop_modify_fd(io->fd, io->events);
		}
	}

	if ((events & EPOLLIN) && io->read_callback) {
		if (!io->read_callback(io, io->read_data)) {
			if (io->read_destroy)
				io->read_destroy(io->read_data);

			io->read_callback = NULL;
			io->read_destroy = NULL;
			io->read_data = NULL;

			io->events &= ~EPOLLIN;

			mainloop_modify_fd(io->fd, io->events);
		}
	}

	if ((events & EPOLLOUT) && io->write_callback) {
		if (!io->write_callback(io, io->write_data)) {
			if (io->write_destroy)
				io->write_destroy(io->write_data);

			io->write_callback = NULL;
			io->write_destroy = NULL;
			io->write_data = NULL;

			io->events &= ~EPOLLOUT;

			mainloop_modify_fd(io->fd, io->events);
		}
	}

	io_unref(io);
}

struct io *io_new(int fd)
{
	struct io *io;

	if (fd < 0)
		return NULL;

	io = new0(struct io, 1);
	io->fd = fd;
	io->events = 0;
	io->close_on_destroy = false;

	if (mainloop_add_fd(io->fd, io->events, io_callback,
						io, io_cleanup) < 0) {
		free(io);
		return NULL;
	}

	return io_ref(io);
}

void io_destroy(struct io *io)
{
	if (!io)
		return;

	io->read_callback = NULL;
	io->write_callback = NULL;
	io->disconnect_callback = NULL;

	mainloop_remove_fd(io->fd);

	io_unref(io);
}

int io_get_fd(struct io *io)
{
	if (!io)
		return -ENOTCONN;

	return io->fd;
}

bool io_set_close_on_destroy(struct io *io, bool do_close)
{
	if (!io)
		return false;

	io->close_on_destroy = do_close;

	return true;
}

bool io_set_read_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	uint32_t events;

	if (!io || io->fd < 0)
		return false;

	if (io->read_destroy)
		io->read_destroy(io->read_data);

	if (callback)
		events = io->events | EPOLLIN;
	else
		events = io->events & ~EPOLLIN;

	io->read_callback = callback;
	io->read_destroy = destroy;
	io->read_data = user_data;

	if (events == io->events)
		return true;

	if (mainloop_modify_fd(io->fd, events) < 0)
		return false;

	io->events = events;

	return true;
}

bool io_set_write_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	uint32_t events;

	if (!io || io->fd < 0)
		return false;

	if (io->write_destroy)
		io->write_destroy(io->write_data);

	if (callback)
		events = io->events | EPOLLOUT;
	else
		events = io->events & ~EPOLLOUT;

	io->write_callback = callback;
	io->write_destroy = destroy;
	io->write_data = user_data;

	if (events == io->events)
		return true;

	if (mainloop_modify_fd(io->fd, events) < 0)
		return false;

	io->events = events;

	return true;
}

bool io_set_disconnect_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	uint32_t events;

	if (!io || io->fd < 0)
		return false;

	if (io->disconnect_destroy)
		io->disconnect_destroy(io->disconnect_data);

	if (callback)
		events = io->events | EPOLLRDHUP;
	else
		events = io->events & ~EPOLLRDHUP;

	io->disconnect_callback = callback;
	io->disconnect_destroy = destroy;
	io->disconnect_data = user_data;

	if (events == io->events)
		return true;

	if (mainloop_modify_fd(io->fd, events) < 0)
		return false;

	io->events = events;

	return true;
}

ssize_t io_send(struct io *io, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;

	if (!io || io->fd < 0)
		return -ENOTCONN;

	do {
		ret = writev(io->fd, iov, iovcnt);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return -errno;

	return ret;
}

bool io_shutdown(struct io *io)
{
	if (!io || io->fd < 0)
		return false;

	return shutdown(io->fd, SHUT_RDWR) == 0;
}
