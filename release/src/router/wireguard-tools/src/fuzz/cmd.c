// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const char *__asan_default_options()
{
	return "verbosity=1";
}

int wg_main(int argc, char *argv[]);

static FILE *devnull;

int LLVMFuzzerTestOneInput(const char *data, size_t data_len)
{
	char *argv[8192] = { 0 }, *args;
	size_t argc = 0;
	FILE *fake_stdin = NULL;

	if (!devnull) {
		assert((devnull = fopen("/dev/null", "r+")));
		stdin = stdout = stderr = devnull;
	}

	assert((args = malloc(data_len)));
	memcpy(args, data, data_len);
	if (data_len)
		args[data_len - 1] = '\0';

	for (const char *arg = args; argc < 8192 && arg - args < data_len; arg += strlen(arg) + 1) {
		if (arg[0])
			assert((argv[argc++] = strdup(arg)));
	}
	if (!argc)
		assert((argv[argc++] = strdup("no argv[0]!")));
	if (argc > 2 && (!strcmp(argv[1], "show") || !strcmp(argv[1], "showconf") || !strcmp(argv[1], "set") || !strcmp(argv[1], "setconf") || !strcmp(argv[1], "addconf") || !strcmp(argv[1], "syncconf"))) {
		free(argv[2]);
		assert((argv[2] = strdup("wg0")));
	}
	if (argc >= 2 && !strcmp(argv[1], "pubkey")) {
		char *arg;
		size_t len;

		for (size_t i = 2; i < argc; ++i)
			free(argv[i]);
		argc = 2;
		arg = args;
		for (; !arg[0]; ++arg);
		arg += strlen(arg) + 1;
		for (; !arg[0]; ++arg);
		arg += strlen(arg) + 1;
		len = data_len - (arg - args);
		if (len <= 1)
			goto done;
		assert((fake_stdin = fmemopen(arg, len - 1, "r")));
		stdin = fake_stdin;
	}
	wg_main(argc, argv);
done:
	for (size_t i = 0; i < argc; ++i)
		free(argv[i]);
	free(args);
	if (fake_stdin)
		fclose(fake_stdin);
	return 0;
}
