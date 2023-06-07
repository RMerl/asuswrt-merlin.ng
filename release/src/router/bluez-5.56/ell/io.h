/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
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

#ifndef __ELL_IO_H
#define __ELL_IO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_io;

typedef void (*l_io_debug_cb_t) (const char *str, void *user_data);

typedef bool (*l_io_read_cb_t) (struct l_io *io, void *user_data);
typedef bool (*l_io_write_cb_t) (struct l_io *io, void *user_data);
typedef void (*l_io_disconnect_cb_t) (struct l_io *io, void *user_data);
typedef void (*l_io_destroy_cb_t) (void *user_data);

struct l_io *l_io_new(int fd);
void l_io_destroy(struct l_io *io);

int l_io_get_fd(struct l_io *io);
bool l_io_set_close_on_destroy(struct l_io *io, bool do_close);

bool l_io_set_read_handler(struct l_io *io, l_io_read_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);
bool l_io_set_write_handler(struct l_io *io, l_io_write_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);
bool l_io_set_disconnect_handler(struct l_io *io,
				l_io_disconnect_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);

bool l_io_set_debug(struct l_io *io, l_io_debug_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_IO_H */
