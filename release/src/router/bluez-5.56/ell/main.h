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

#ifndef __ELL_MAIN_H
#define __ELL_MAIN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool l_main_init(void);
int l_main_prepare(void);
void l_main_iterate(int timeout);
int l_main_run(void);
bool l_main_exit(void);

bool l_main_quit(void);

typedef void (*l_main_signal_cb_t) (uint32_t signo, void *user_data);

int l_main_run_with_signal(l_main_signal_cb_t callback, void *user_data);

int l_main_get_epoll_fd(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_MAIN_H */
