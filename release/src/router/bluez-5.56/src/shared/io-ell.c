// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include <ell/ell.h>

#include "src/shared/io.h"

struct io_watch {
	struct io *io;
	io_callback_func_t cb;
	io_destroy_func_t destroy;
	void *user_data;
};

struct io {
	int ref_count;
	struct l_io *l_io;
	struct io_watch *read_watch;
	struct io_watch *write_watch;
	struct io_watch *disc_watch;
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

	l_free(io);
}

static void watch_destroy(void *user_data)
{
	struct io_watch *watch = user_data;
	struct io *io;

	if (!watch)
		return;

	io = watch->io;

	if (watch == io->read_watch)
		io->read_watch = NULL;
	else if (watch == io->write_watch)
		io->write_watch = NULL;
	else if (watch == io->disc_watch)
		io->disc_watch = NULL;

	if (watch->destroy)
		watch->destroy(watch->user_data);

	io_unref(watch->io);
	l_free(watch);
}

static struct io_watch *watch_new(struct io *io, io_callback_func_t cb,
				void *user_data, io_destroy_func_t destroy)
{
	struct io_watch *watch;

	watch = l_new(struct io_watch, 1);
	watch->io = io_ref(io);
	watch->cb = cb;
	watch->user_data = user_data;
	watch->destroy = destroy;

	return watch;
}

static bool watch_callback(struct l_io *l_io, void *user_data)
{
	struct io_watch *watch = user_data;

	if (!watch->cb)
		return false;

	return watch->cb(watch->io, watch->user_data);
}

static void disc_callback(struct l_io *l_io, void *user_data)
{
	struct io_watch *watch = user_data;

	if (watch->cb)
		watch->cb(watch->io, watch->user_data);
}

struct io *io_new(int fd)
{
	struct io *io;
	struct l_io *l_io;

	if (fd < 0)
		return NULL;

	io = l_new(struct io, 1);
	if (!io)
		return NULL;

	l_io = l_io_new(fd);
	if (!l_io) {
		l_free(io);
		return NULL;
	}

	io->l_io = l_io;

	return io_ref(io);
}

void io_destroy(struct io *io)
{
	if (!io)
		return;

	l_io_set_read_handler(io->l_io, NULL, NULL, NULL);
	watch_destroy(io->read_watch);
	io->read_watch = NULL;

	l_io_set_write_handler(io->l_io, NULL, NULL, NULL);
	watch_destroy(io->write_watch);
	io->write_watch = NULL;

	l_io_set_disconnect_handler(io->l_io, NULL, NULL, NULL);
	watch_destroy(io->disc_watch);
	io->disc_watch = NULL;

	l_io_destroy(io->l_io);
	io->l_io = NULL;

	io_unref(io);
}

int io_get_fd(struct io *io)
{
	if (!io || !io->l_io)
		return -ENOTCONN;

	return l_io_get_fd(io->l_io);
}

bool io_set_close_on_destroy(struct io *io, bool do_close)
{
	if (!io || !io->l_io)
		return false;

	return l_io_set_close_on_destroy(io->l_io, do_close);
}

bool io_set_read_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	bool result;

	if (!io || !io->l_io)
		return false;

	if (io->read_watch) {
		l_io_set_read_handler(io->l_io, NULL, NULL, NULL);

		if (!callback) {
			watch_destroy(io->read_watch);
			io->read_watch = NULL;
			return true;
		}
	}

	io->read_watch = watch_new(io, callback, user_data, destroy);

	result = l_io_set_read_handler(io->l_io, watch_callback, io->read_watch,
								watch_destroy);

	if (!result) {
		watch_destroy(io->read_watch);
		io->read_watch = NULL;
	}

	return result;
}

bool io_set_write_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	bool result;

	if (!io || !io->l_io)
		return false;

	if (io->write_watch) {
		l_io_set_write_handler(io->l_io, NULL, NULL, NULL);

		if (!callback) {
			watch_destroy(io->write_watch);
			io->write_watch = NULL;
			return true;
		}
	}

	io->write_watch = watch_new(io, callback, user_data, destroy);

	result = l_io_set_write_handler(io->l_io, watch_callback,
						io->write_watch, watch_destroy);

	if (!result) {
		watch_destroy(io->write_watch);
		io->write_watch = NULL;
	}

	return result;
}

bool io_set_disconnect_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy)
{
	bool result;

	if (!io || !io->l_io)
		return false;

	if (io->disc_watch) {
		l_io_set_disconnect_handler(io->l_io, NULL, NULL, NULL);

		if (!callback) {
			watch_destroy(io->disc_watch);
			io->disc_watch = NULL;
			return true;
		}
	}

	io->disc_watch = watch_new(io, callback, user_data, destroy);

	result = l_io_set_disconnect_handler(io->l_io, disc_callback,
						io->disc_watch, watch_destroy);

	if (!result) {
		watch_destroy(io->disc_watch);
		io->disc_watch = NULL;
	}

	return result;
}

ssize_t io_send(struct io *io, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;
	int fd;

	if (!io || !io->l_io)
		return -ENOTCONN;

	fd = l_io_get_fd(io->l_io);
	if (fd < 0)
		return -ENOTCONN;

	do {
		ret = writev(fd, iov, iovcnt);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return -errno;

	return ret;
}

bool io_shutdown(struct io *io)
{
	int fd;

	if (!io || !io->l_io)
		return false;

	fd = l_io_get_fd(io->l_io);
	if (fd < 0)
		return false;

	return shutdown(fd, SHUT_RDWR) == 0;
}
