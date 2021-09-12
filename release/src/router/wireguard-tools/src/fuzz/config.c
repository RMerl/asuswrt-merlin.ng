// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#undef stderr
#define stderr stdin
#include "../config.c"
#include "../encoding.c"
#undef stderr

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../config.h"

const char *__asan_default_options()
{
	return "verbosity=1";
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t len)
{
	bool file;
	char *input;

	if (len < 2)
		return 0;
	file = !!(data[0] >> 7);
	input = malloc(len);
	if (!input)
		return 0;
	memcpy(input, data + 1, len - 1);
	input[len - 1] = '\0';

	if (file) {
		struct config_ctx ctx;
		char *saveptr;

		config_read_init(&ctx, false);
		for (char *line = strtok_r(input, "\n", &saveptr); line; line = strtok_r(NULL, "\n", &saveptr)) {
			if (!config_read_line(&ctx, line))
				config_read_init(&ctx, false);
		}
		free_wgdevice(config_read_finish(&ctx));
	} else {
		size_t spaces = 0;
		char **argv, *saveptr;

		for (char *c = input; *c; ++c) {
			if (*c == ' ')
				++spaces;
		}
		argv = calloc(spaces + 1, sizeof(char *));
		if (!argv)
			goto out;
		spaces = 0;
		for (char *token = strtok_r(input, " ", &saveptr); token; token = strtok_r(NULL, " ", &saveptr))
			argv[spaces++] = token;
		free_wgdevice(config_read_cmd(argv, spaces));
		free(argv);
	}

out:
	free(input);
	return 0;
}
