/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <fuzzing_engine.h>
#include <test/fuzz.h>

static struct fuzz_test *find_fuzz_test(const char *name)
{
	struct fuzz_test *fuzzer = FUZZ_TEST_START();
	size_t count = FUZZ_TEST_COUNT();
	size_t i;

	for (i = 0; i < count; ++i) {
		if (strcmp(name, fuzzer->name) == 0)
			return fuzzer;
		++fuzzer;
	}

	return NULL;
}

static struct udevice *find_fuzzing_engine(void)
{
	struct udevice *dev;

	if (uclass_first_device_err(UCLASS_FUZZING_ENGINE, &dev))
		return NULL;

	return dev;
}

static int do_fuzz(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct fuzz_test *fuzzer;
	struct udevice *dev;

	if (argc != 2)
		return CMD_RET_USAGE;

	fuzzer = find_fuzz_test(argv[1]);
	if (!fuzzer) {
		printf("Could not find fuzzer: %s\n", argv[1]);
		return 1;
	}

	dev = find_fuzzing_engine();
	if (!dev) {
		puts("No fuzzing engine available\n");
		return 1;
	}

	while (1) {
		const uint8_t *data;
		size_t size;

		if (dm_fuzzing_engine_get_input(dev, &data, &size)) {
			puts("Fuzzing engine failed\n");
			return 1;
		}

		fuzzer->func(data, size);
	}

	return 1;
}

U_BOOT_LONGHELP(fuzz,
	"[fuzz-test-name] - execute the named fuzz test\n");

U_BOOT_CMD(
	fuzz, CONFIG_SYS_MAXARGS, 1, do_fuzz,
	"fuzz tests", fuzz_help_text
);
