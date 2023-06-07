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

#ifndef __ELL_IDLE_H
#define __ELL_IDLE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_idle;

typedef void (*l_idle_notify_cb_t) (struct l_idle *idle, void *user_data);
typedef void (*l_idle_oneshot_cb_t) (void *user_data);
typedef void (*l_idle_destroy_cb_t) (void *user_data);

struct l_idle *l_idle_create(l_idle_notify_cb_t callback,
			void *user_data, l_idle_destroy_cb_t destroy);
void l_idle_remove(struct l_idle *idle);

bool l_idle_oneshot(l_idle_oneshot_cb_t callback, void *user_data,
			l_idle_destroy_cb_t destroy);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_IDLE_H */
