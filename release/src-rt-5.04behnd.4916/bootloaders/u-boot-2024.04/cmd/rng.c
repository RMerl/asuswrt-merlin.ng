// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'rng' command prints bytes from the hardware random number generator.
 *
 * Copyright (c) 2019, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <hexdump.h>
#include <malloc.h>
#include <rng.h>

static int do_rng(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	size_t n;
	u8 buf[64];
	int devnum;
	struct udevice *dev;
	int ret = CMD_RET_SUCCESS;

	switch (argc) {
	case 1:
		devnum = 0;
		n = 0x40;
		break;
	case 2:
		devnum = hextoul(argv[1], NULL);
		n = 0x40;
		break;
	case 3:
		devnum = hextoul(argv[1], NULL);
		n = hextoul(argv[2], NULL);
		break;
	default:
		return CMD_RET_USAGE;
	}

	if (uclass_get_device_by_seq(UCLASS_RNG, devnum, &dev) || !dev) {
		printf("No RNG device\n");
		return CMD_RET_FAILURE;
	}

	if (!n)
		return 0;

	n = min(n, sizeof(buf));

	if (dm_rng_read(dev, buf, n)) {
		printf("Reading RNG failed\n");
		ret = CMD_RET_FAILURE;
	} else {
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf, n);
	}

	return ret;
}

U_BOOT_LONGHELP(rng,
	"[dev [n]]\n"
	"  - print n random bytes(max 64) read from dev\n");

U_BOOT_CMD(
	rng, 3, 0, do_rng,
	"print bytes from the hardware random number generator",
	rng_help_text
);
