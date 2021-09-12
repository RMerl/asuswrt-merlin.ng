// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#define RUNSTATEDIR "/var/empty"
#include "../curve25519.c"
#undef __linux__
#include "../ipc.c"
#include "../encoding.c"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const char *__asan_default_options()
{
	return "verbosity=1";
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t data_len)
{
	struct string_list list = { 0 };
	char *interfaces;

	if (!data_len)
		return 0;

	interfaces = malloc(data_len);
	assert(interfaces);
	memcpy(interfaces, data, data_len);
	interfaces[data_len - 1] = '\0';

	for (char *interface = interfaces; interface - interfaces < data_len; interface += strlen(interface) + 1)
		assert(string_list_add(&list, interface) == 0);

	for (char *interface = interfaces, *interface2 = list.buffer;;) {
		size_t len;

		if (interface - interfaces >= data_len) {
			assert(!interface2 || !strlen(interface2));
			break;
		}
		len = strlen(interface);
		if (!len) {
			++interface;
			continue;
		}
		assert(strlen(interface2) == len);
		assert(!memcmp(interface, interface2, len + 1));
		interface += len + 1;
		interface2 += len + 1;
	}
	free(list.buffer);
	free(interfaces);
	return 0;
}
