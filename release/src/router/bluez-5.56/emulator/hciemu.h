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
#include <stdint.h>

struct hciemu;
struct hciemu_client;

enum hciemu_type {
	HCIEMU_TYPE_BREDRLE,
	HCIEMU_TYPE_BREDR,
	HCIEMU_TYPE_LE,
	HCIEMU_TYPE_LEGACY,
	HCIEMU_TYPE_BREDRLE50,
	HCIEMU_TYPE_BREDRLE52,
};

enum hciemu_hook_type {
	HCIEMU_HOOK_PRE_CMD,
	HCIEMU_HOOK_POST_CMD,
	HCIEMU_HOOK_PRE_EVT,
	HCIEMU_HOOK_POST_EVT,
};

struct hciemu *hciemu_new(enum hciemu_type type);
struct hciemu *hciemu_new_num(enum hciemu_type type, uint8_t num);

struct hciemu *hciemu_ref(struct hciemu *hciemu);
void hciemu_unref(struct hciemu *hciemu);

struct hciemu_client *hciemu_get_client(struct hciemu *hciemu, int num);
struct bthost *hciemu_client_host(struct hciemu_client *client);
const uint8_t *hciemu_client_bdaddr(struct hciemu_client *client);

typedef void (*hciemu_debug_func_t)(const char *str, void *user_data);
typedef void (*hciemu_destroy_func_t)(void *user_data);
bool hciemu_set_debug(struct hciemu *hciemu, hciemu_debug_func_t callback,
			void *user_data, hciemu_destroy_func_t destroy);

struct bthost *hciemu_client_get_host(struct hciemu *hciemu);

const char *hciemu_get_address(struct hciemu *hciemu);
uint8_t *hciemu_get_features(struct hciemu *hciemu);

const uint8_t *hciemu_get_master_bdaddr(struct hciemu *hciemu);
const uint8_t *hciemu_get_client_bdaddr(struct hciemu *hciemu);

uint8_t hciemu_get_master_scan_enable(struct hciemu *hciemu);

uint8_t hciemu_get_master_le_scan_enable(struct hciemu *hciemu);

void hciemu_set_master_le_states(struct hciemu *hciemu,
						const uint8_t *le_states);

typedef void (*hciemu_command_func_t)(uint16_t opcode, const void *data,
						uint8_t len, void *user_data);

typedef bool (*hciemu_hook_func_t)(const void *data, uint16_t len,
							void *user_data);

bool hciemu_add_master_post_command_hook(struct hciemu *hciemu,
			hciemu_command_func_t function, void *user_data);

bool hciemu_clear_master_post_command_hooks(struct hciemu *hciemu);

int hciemu_add_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
				uint16_t opcode, hciemu_hook_func_t function,
				void *user_data);

bool hciemu_del_hook(struct hciemu *hciemu, enum hciemu_hook_type type,
							uint16_t opcode);
