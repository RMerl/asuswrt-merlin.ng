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

#include <stdbool.h>
#include <stdint.h>

#define MGMT_VERSION(v, r) (((v) << 16) + (r))

typedef void (*mgmt_destroy_func_t)(void *user_data);

struct mgmt;

struct mgmt *mgmt_new(int fd);
struct mgmt *mgmt_new_default(void);

struct mgmt *mgmt_ref(struct mgmt *mgmt);
void mgmt_unref(struct mgmt *mgmt);

typedef void (*mgmt_debug_func_t)(const char *str, void *user_data);

bool mgmt_set_debug(struct mgmt *mgmt, mgmt_debug_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy);

bool mgmt_set_close_on_unref(struct mgmt *mgmt, bool do_close);

typedef void (*mgmt_request_func_t)(uint8_t status, uint16_t length,
					const void *param, void *user_data);

unsigned int mgmt_send(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy);
unsigned int mgmt_send_nowait(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy);
unsigned int mgmt_reply(struct mgmt *mgmt, uint16_t opcode, uint16_t index,
				uint16_t length, const void *param,
				mgmt_request_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy);
bool mgmt_cancel(struct mgmt *mgmt, unsigned int id);
bool mgmt_cancel_index(struct mgmt *mgmt, uint16_t index);
bool mgmt_cancel_all(struct mgmt *mgmt);

typedef void (*mgmt_notify_func_t)(uint16_t index, uint16_t length,
					const void *param, void *user_data);

unsigned int mgmt_register(struct mgmt *mgmt, uint16_t event, uint16_t index,
				mgmt_notify_func_t callback,
				void *user_data, mgmt_destroy_func_t destroy);
bool mgmt_unregister(struct mgmt *mgmt, unsigned int id);
bool mgmt_unregister_index(struct mgmt *mgmt, uint16_t index);
bool mgmt_unregister_all(struct mgmt *mgmt);
