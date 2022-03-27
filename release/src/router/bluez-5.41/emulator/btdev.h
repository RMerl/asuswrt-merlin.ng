/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <stdint.h>
#include <stdbool.h>

#define BTDEV_RESPONSE_DEFAULT		0
#define BTDEV_RESPONSE_COMMAND_STATUS	1
#define BTDEV_RESPONSE_COMMAND_COMPLETE	2

typedef struct btdev_callback * btdev_callback;

void btdev_command_response(btdev_callback callback, uint8_t response,
				uint8_t status, const void *data, uint8_t len);

#define btdev_command_default(callback) \
		btdev_command_response(callback, \
			BTDEV_RESPONSE_DEFAULT, 0x00, NULL, 0);

#define btdev_command_status(callback, status) \
		btdev_command_response(callback, \
			BTDEV_RESPONSE_COMMAND_STATUS, status, NULL, 0);

#define btdev_command_complete(callback, data, len) \
		 btdev_command_response(callback, \
			BTDEV_RESPONSE_COMMAND_COMPLETE, 0x00, data, len);


typedef void (*btdev_command_func) (uint16_t opcode,
				const void *data, uint8_t len,
				btdev_callback callback, void *user_data);

typedef void (*btdev_send_func) (const struct iovec *iov, int iovlen,
							void *user_data);

typedef bool (*btdev_hook_func) (const void *data, uint16_t len,
							void *user_data);

enum btdev_type {
	BTDEV_TYPE_BREDRLE,
	BTDEV_TYPE_BREDR,
	BTDEV_TYPE_LE,
	BTDEV_TYPE_AMP,
	BTDEV_TYPE_BREDR20,
};

enum btdev_hook_type {
	BTDEV_HOOK_PRE_CMD,
	BTDEV_HOOK_POST_CMD,
	BTDEV_HOOK_PRE_EVT,
	BTDEV_HOOK_POST_EVT,
};

struct btdev;

struct btdev *btdev_create(enum btdev_type type, uint16_t id);
void btdev_destroy(struct btdev *btdev);

const uint8_t *btdev_get_bdaddr(struct btdev *btdev);
uint8_t *btdev_get_features(struct btdev *btdev);

uint8_t btdev_get_scan_enable(struct btdev *btdev);

uint8_t btdev_get_le_scan_enable(struct btdev *btdev);

void btdev_set_command_handler(struct btdev *btdev, btdev_command_func handler,
							void *user_data);

void btdev_set_send_handler(struct btdev *btdev, btdev_send_func handler,
							void *user_data);

void btdev_receive_h4(struct btdev *btdev, const void *data, uint16_t len);

int btdev_add_hook(struct btdev *btdev, enum btdev_hook_type type,
				uint16_t opcode, btdev_hook_func handler,
				void *user_data);

bool btdev_del_hook(struct btdev *btdev, enum btdev_hook_type type,
							uint16_t opcode);
