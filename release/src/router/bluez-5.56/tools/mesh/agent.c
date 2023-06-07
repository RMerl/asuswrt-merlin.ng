// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include <lib/bluetooth.h>

#include "src/shared/shell.h"
#include "tools/mesh/agent.h"

#define AGENT_PROMPT	COLOR_BLUE "[mesh-agent]" COLOR_OFF "# "

struct input_request {
	oob_type_t type;
	uint16_t len;
	agent_input_cb cb;
	void *user_data;
};

static struct input_request pending_request = {NONE, 0, NULL, NULL};

bool agent_completion(void)
{
	if (pending_request.type == NONE)
		return false;

	return true;
}

static void reset_input_request(void)
{
	pending_request.type = NONE;
	pending_request.len = 0;
	pending_request.cb = NULL;
	pending_request.user_data = NULL;
}

static bool str2hex(const char *str, uint16_t in_len, uint8_t *out,
							uint16_t out_len)
{
	uint16_t i;

	if (in_len < out_len * 2)
		return false;

	for (i = 0; i < out_len; i++) {
		if (sscanf(&str[i * 2], "%02hhx", &out[i]) != 1)
			return false;
	}

	return true;
}

static void response_hexadecimal(const char *input, void *user_data)
{
	uint8_t buf[MAX_HEXADECIMAL_OOB_LEN];
	uint16_t len = pending_request.len;

	if (!str2hex(input, strlen(input), buf, pending_request.len) ) {
		bt_shell_printf("Incorrect input: expecting %d hex octets\n",
			  pending_request.len);
		len = 0;
	}

	if (pending_request.cb)
		pending_request.cb(HEXADECIMAL, buf, len,
					pending_request.user_data);

	reset_input_request();
}

static void response_decimal(const char *input, void *user_data)
{
	uint8_t buf[DECIMAL_OOB_LEN];
	uint16_t len = DECIMAL_OOB_LEN;

	if (strlen(input) > pending_request.len)
		len = 0;

	bt_put_be32(atoi(input), buf);

	if (pending_request.cb)
		pending_request.cb(DECIMAL, buf, len,
					pending_request.user_data);

	reset_input_request();
}

static void response_ascii(const char *input, void *user_data)
{
	if (pending_request.cb)
		pending_request.cb(ASCII, (uint8_t *) input, strlen(input),
					pending_request.user_data);

	reset_input_request();
}

static bool request_hexadecimal(uint16_t len)
{
	if (len > MAX_HEXADECIMAL_OOB_LEN)
		return false;

	bt_shell_printf("Request hexadecimal key (hex %d octets)\n", len);
	bt_shell_prompt_input(AGENT_PROMPT, "Enter key (hex number):",
						response_hexadecimal, NULL);

	return true;
}

static uint32_t power_ten(uint8_t power)
{
	uint32_t ret = 1;

	while (power--)
		ret *= 10;

	return ret;
}

static bool request_decimal(const char *desc, uint16_t len)
{
	if (!desc)
		bt_shell_printf("Request decimal key (0 - %d)\n",
				power_ten(len) - 1);
	else
		bt_shell_printf("%s (0 - %d)\n", desc, power_ten(len) - 1);

	bt_shell_prompt_input(AGENT_PROMPT, "Enter decimal number:",
							response_decimal, NULL);

	return true;
}

static bool request_ascii(uint16_t len)
{
	if (len > MAX_ASCII_OOB_LEN)
		return false;

	bt_shell_printf("Request ASCII key (max characters %d)\n", len);
	bt_shell_prompt_input("mesh", "Enter key (ascii string):",
							response_ascii, NULL);

	return true;
}

bool agent_input_request(oob_type_t type, uint16_t max_len, const char *desc,
					agent_input_cb cb, void *user_data)
{
	bool result;

	if (pending_request.type != NONE)
		return false;

	switch (type) {
	case HEXADECIMAL:
		result = request_hexadecimal(max_len);
		break;
	case DECIMAL:
		result = request_decimal(desc, max_len);
		break;
	case ASCII:
		result = request_ascii(max_len);
		break;
	case NONE:
	case OUTPUT:
	default:
		return false;
	};

	if (result) {
		pending_request.type = type;
		pending_request.len = max_len;
		pending_request.cb = cb;
		pending_request.user_data = user_data;

		return true;
	}

	return false;
}

static void response_output(const char *input, void *user_data)
{
	reset_input_request();
}

bool agent_output_request(const char* str)
{
	if (pending_request.type != NONE)
		return false;

	pending_request.type = OUTPUT;
	bt_shell_prompt_input("mesh", str, response_output, NULL);
	return true;
}

void agent_output_request_cancel(void)
{
	if (pending_request.type != OUTPUT)
		return;
	pending_request.type = NONE;
	bt_shell_release_prompt("");
}
