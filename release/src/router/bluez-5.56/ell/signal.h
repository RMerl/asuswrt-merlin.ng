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

#ifndef __ELL_SIGNAL_H
#define __ELL_SIGNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_signal;

typedef void (*l_signal_notify_cb_t) (void *user_data);
typedef void (*l_signal_destroy_cb_t) (void *user_data);

struct l_signal *l_signal_create(uint32_t signo, l_signal_notify_cb_t callback,
				void *user_data, l_signal_destroy_cb_t destroy);
void l_signal_remove(struct l_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_SIGNAL_H */
