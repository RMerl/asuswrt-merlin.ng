// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'kaslrseed' command takes bytes from the hardware random number
 * generator and uses them to set the kaslr-seed value in the chosen node.
 *
 * Copyright (c) 2021, Chris Morgan <macromorgan@hotmail.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <hexdump.h>
#include <malloc.h>
#include <rng.h>
#include <fdt_support.h>

static int do_kaslr_seed(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	size_t n = 0x8;
	struct udevice *dev;
	u64 *buf;
	int nodeoffset;
	int ret = CMD_RET_SUCCESS;

	if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
		printf("No RNG device\n");
		return CMD_RET_FAILURE;
	}

	buf = malloc(n);
	if (!buf) {
		printf("Out of memory\n");
		return CMD_RET_FAILURE;
	}

	if (dm_rng_read(dev, buf, n)) {
		printf("Reading RNG failed\n");
		return CMD_RET_FAILURE;
	}

	if (!working_fdt) {
		printf("No FDT memory address configured. Please configure\n"
		       "the FDT address via \"fdt addr <address>\" command.\n"
		       "Aborting!\n");
		return CMD_RET_FAILURE;
	}

	ret = fdt_check_header(working_fdt);
	if (ret < 0) {
		printf("fdt_chosen: %s\n", fdt_strerror(ret));
		return CMD_RET_FAILURE;
	}

	nodeoffset = fdt_find_or_add_subnode(working_fdt, 0, "chosen");
	if (nodeoffset < 0) {
		printf("Reading chosen node failed\n");
		return CMD_RET_FAILURE;
	}

	ret = fdt_setprop(working_fdt, nodeoffset, "kaslr-seed", buf, sizeof(buf));
	if (ret < 0) {
		printf("Unable to set kaslr-seed on chosen node: %s\n", fdt_strerror(ret));
		return CMD_RET_FAILURE;
	}

	free(buf);

	return ret;
}

U_BOOT_LONGHELP(kaslrseed,
	"[n]\n"
	"  - append random bytes to chosen kaslr-seed node\n");

U_BOOT_CMD(
	kaslrseed, 1, 0, do_kaslr_seed,
	"feed bytes from the hardware random number generator to the kaslr-seed",
	kaslrseed_help_text
);
