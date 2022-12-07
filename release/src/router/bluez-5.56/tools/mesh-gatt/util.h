/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>

struct mesh_publication;

void set_menu_prompt(const char *name, const char *id);
void print_byte_array(const char *prefix, const void *ptr, int len);
bool str2hex(const char *str, uint16_t in_len, uint8_t *out_buf,
		uint16_t out_len);
size_t hex2str(uint8_t *in, size_t in_len, char *out,
		size_t out_len);
uint16_t mesh_opcode_set(uint32_t opcode, uint8_t *buf);
bool mesh_opcode_get(const uint8_t *buf, uint16_t sz, uint32_t *opcode, int *n);
const char *mesh_status_str(uint8_t status);
void print_model_pub(uint16_t ele_addr, uint32_t mod_id,
						struct mesh_publication *pub);
void swap_u256_bytes(uint8_t *u256);
