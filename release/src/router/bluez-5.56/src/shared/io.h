/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>
#include <sys/uio.h>

typedef void (*io_destroy_func_t)(void *data);

struct io;

struct io *io_new(int fd);
void io_destroy(struct io *io);

int io_get_fd(struct io *io);
bool io_set_close_on_destroy(struct io *io, bool do_close);

ssize_t io_send(struct io *io, const struct iovec *iov, int iovcnt);
bool io_shutdown(struct io *io);

typedef bool (*io_callback_func_t)(struct io *io, void *user_data);

bool io_set_read_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy);
bool io_set_write_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy);
bool io_set_disconnect_handler(struct io *io, io_callback_func_t callback,
				void *user_data, io_destroy_func_t destroy);
