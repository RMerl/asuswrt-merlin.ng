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

#include <stdint.h>
#include <errno.h>

#include <ell/util.h>

#define uninitialized_var(x) x = x

#define align_len(len, boundary) (((len)+(boundary)-1) & ~((boundary)-1))

#define LIB_EXPORT __attribute__ ((visibility("default")))

struct l_debug_desc;

void debug_enable(struct l_debug_desc *start, struct l_debug_desc *stop);
void debug_disable(struct l_debug_desc *start, struct l_debug_desc *stop);

void plugin_update_debug(void);

typedef void (*watch_event_cb_t) (int fd, uint32_t events, void *user_data);
typedef void (*watch_destroy_cb_t) (void *user_data);

typedef void (*idle_event_cb_t) (void *user_data);
typedef void (*idle_destroy_cb_t) (void *user_data);

int watch_add(int fd, uint32_t events, watch_event_cb_t callback,
				void *user_data, watch_destroy_cb_t destroy);
int watch_modify(int fd, uint32_t events, bool force);
int watch_remove(int fd, bool epoll_del);
int watch_clear(int fd);

#define IDLE_FLAG_NO_WARN_DANGLING 0x10000000
int idle_add(idle_event_cb_t callback, void *user_data, uint32_t flags,
		idle_destroy_cb_t destroy);
void idle_remove(int id);
