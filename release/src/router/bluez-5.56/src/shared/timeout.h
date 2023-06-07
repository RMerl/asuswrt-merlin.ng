/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>

typedef bool (*timeout_func_t)(void *user_data);
typedef void (*timeout_destroy_func_t)(void *user_data);

unsigned int timeout_add(unsigned int timeout, timeout_func_t func,
			void *user_data, timeout_destroy_func_t destroy);
void timeout_remove(unsigned int id);
