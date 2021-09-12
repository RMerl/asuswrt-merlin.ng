// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#undef stderr
#define stderr stdin
#define RUNSTATEDIR "/var/empty"
#include "../curve25519.c"
#define parse_allowedips parse_allowedips_ipc
#include "../ipc.c"
#undef parse_allowedips
#include "../encoding.c"
static FILE *hacked_fopen(const char *pathname, const char *mode);
#define fopen hacked_fopen
#include "../config.c"
#include "../set.c"
#undef stderr

#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char *__asan_default_options()
{
	return "verbosity=1";
}

const char *PROG_NAME = "wg";

static FILE *hacked_fopen(const char *pathname, const char *mode)
{
	return fmemopen((char *)pathname, strlen(pathname), "r");
}

int LLVMFuzzerTestOneInput(const char *data, size_t data_len)
{
	char *argv[8192] = { "set", "wg0" }, *args;
	size_t argc = 2;

	if (!data_len)
		return 0;

	assert((args = malloc(data_len)));
	memcpy(args, data, data_len);
	args[data_len - 1] = '\0';

	for (char *arg = strtok(args, " \t\n\r"); arg && argc < 8192; arg = strtok(NULL, " \t\n\r")) {
		if (arg[0])
			argv[argc++] = arg;
	}
	set_main(argc, argv);
	free(args);
	return 0;
}
