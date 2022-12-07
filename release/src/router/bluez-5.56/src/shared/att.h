/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "src/shared/att-types.h"

#define BT_ATT_DEBUG		0x00
#define BT_ATT_DEBUG_VERBOSE	0x01
#define BT_ATT_DEBUG_HEXDUMP	0x02

struct bt_att;
struct bt_att_chan;

struct bt_att *bt_att_new(int fd, bool ext_signed);

struct bt_att *bt_att_ref(struct bt_att *att);
void bt_att_unref(struct bt_att *att);

bool bt_att_set_close_on_unref(struct bt_att *att, bool do_close);

int bt_att_get_fd(struct bt_att *att);

int bt_att_attach_fd(struct bt_att *att, int fd);

int bt_att_get_channels(struct bt_att *att);

typedef void (*bt_att_response_func_t)(uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data);
typedef void (*bt_att_notify_func_t)(struct bt_att_chan *chan,
					uint8_t opcode, const void *pdu,
					uint16_t length, void *user_data);
typedef void (*bt_att_destroy_func_t)(void *user_data);
typedef void (*bt_att_debug_func_t)(const char *str, void *user_data);
typedef void (*bt_att_timeout_func_t)(unsigned int id, uint8_t opcode,
							void *user_data);
typedef void (*bt_att_disconnect_func_t)(int err, void *user_data);
typedef bool (*bt_att_counter_func_t)(uint32_t *sign_cnt, void *user_data);

bool bt_att_set_debug(struct bt_att *att, uint8_t level,
			bt_att_debug_func_t callback, void *user_data,
			bt_att_destroy_func_t destroy);

uint16_t bt_att_get_mtu(struct bt_att *att);
bool bt_att_set_mtu(struct bt_att *att, uint16_t mtu);
uint8_t bt_att_get_link_type(struct bt_att *att);

bool bt_att_set_timeout_cb(struct bt_att *att, bt_att_timeout_func_t callback,
						void *user_data,
						bt_att_destroy_func_t destroy);

unsigned int bt_att_send(struct bt_att *att, uint8_t opcode,
					const void *pdu, uint16_t length,
					bt_att_response_func_t callback,
					void *user_data,
					bt_att_destroy_func_t destroy);
unsigned int bt_att_chan_send(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t len,
					bt_att_response_func_t callback,
					void *user_data,
					bt_att_destroy_func_t destroy);
#define bt_att_chan_send_rsp(chan, opcode, pdu, len) \
	bt_att_chan_send(chan, opcode, pdu, len, NULL, NULL, NULL)
bool bt_att_chan_cancel(struct bt_att_chan *chan, unsigned int id);
bool bt_att_cancel(struct bt_att *att, unsigned int id);
bool bt_att_cancel_all(struct bt_att *att);

int bt_att_chan_send_error_rsp(struct bt_att_chan *chan, uint8_t opcode,
						uint16_t handle, int error);

unsigned int bt_att_register(struct bt_att *att, uint8_t opcode,
						bt_att_notify_func_t callback,
						void *user_data,
						bt_att_destroy_func_t destroy);
bool bt_att_unregister(struct bt_att *att, unsigned int id);

unsigned int bt_att_register_disconnect(struct bt_att *att,
					bt_att_disconnect_func_t callback,
					void *user_data,
					bt_att_destroy_func_t destroy);
bool bt_att_unregister_disconnect(struct bt_att *att, unsigned int id);

bool bt_att_unregister_all(struct bt_att *att);

int bt_att_get_security(struct bt_att *att, uint8_t *enc_size);
bool bt_att_set_security(struct bt_att *att, int level);
void bt_att_set_enc_key_size(struct bt_att *att, uint8_t enc_size);

bool bt_att_set_local_key(struct bt_att *att, uint8_t sign_key[16],
			bt_att_counter_func_t func, void *user_data);
bool bt_att_set_remote_key(struct bt_att *att, uint8_t sign_key[16],
			bt_att_counter_func_t func, void *user_data);
bool bt_att_has_crypto(struct bt_att *att);
