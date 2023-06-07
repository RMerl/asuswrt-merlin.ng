/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#define MAX_HEXADECIMAL_OOB_LEN	128
#define DECIMAL_OOB_LEN		4
#define MAX_ASCII_OOB_LEN		16

typedef enum {
	NONE,
	HEXADECIMAL,
	DECIMAL,
	ASCII,
	OUTPUT,
} oob_type_t;

typedef void (*agent_input_cb)(oob_type_t type, void *input, uint16_t len,
					void *user_data);
bool agent_input_request(oob_type_t type, uint16_t max_len, const char *desc,
					agent_input_cb cb, void *user_data);

bool agent_output_request(const char* str);
void agent_output_request_cancel(void);
bool agent_completion(void);
bool agent_input(const char *input);
void agent_release(void);
