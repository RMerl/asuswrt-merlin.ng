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

struct hciemu;

enum hciemu_type {
	HCIEMU_TYPE_BREDRLE,
	HCIEMU_TYPE_BREDR,
	HCIEMU_TYPE_LE,
	HCIEMU_TYPE_LEGACY,
};

enum hciemu_hook_type {
	HCIEMU_HOOK_PRE_CMD,
	HCIEMU_HOOK_POST_CMD,
	HCIEMU_HOOK_PRE_EVT,
	HCIEMU_HOOK_POST_EVT,
};

struct hciemu *hciemu_new(enum hciemu_type type);

struct hciemu *hciemu_ref(struct hciemu *hciemu);
void hciemu_unref(struct hciemu *hciemu);

struct bthost *hciemu_client_get_host(struct hciemu *hciemu);

const char *hciemu_get_address(struct hciemu *hciemu);
uint8_t *hciemu_get_features(struct hciemu *hciemu);

const uint8_t *hciemu_get_master_bdaddr(struct hciemu *hciemu);
const uint8_t *hciemu_get_client_bdaddr(struct hciemu *hciemu);

uint8_t hciemu_get_master_scan_enable(struct hciemu *hciemu);

uint8_t hciemu_get_master_le_scan_enable(struct hciemu *hciemu);

typedef void (*hciemu_command_func_t)(uint16_t opcode, const void *data,
						uint8_t len, void *user_data);

typedef bool (*hciemu_hook_func_t)(const void *data, uint16_t len,
							void *user_data);

bool hciemu_add_master_post_command_hook(struct hciemu *hciemu,
			hciemu_command_func_t function, void *user_data);

int hciemu_add_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
				uint16_t opcode, hciemu_hook_func_t function,
				void *user_data);

bool hciemu_del_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
							uint16_t opcode);
