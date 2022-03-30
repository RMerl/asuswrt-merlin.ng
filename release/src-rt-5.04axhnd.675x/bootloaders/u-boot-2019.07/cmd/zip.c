// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Lei Wen <leiwen@marvell.com>, Marvell Inc.
 */

#include <common.h>
#include <command.h>

static int do_zip(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long src, dst;
	unsigned long src_len, dst_len = ~0UL;

	switch (argc) {
		case 5:
			dst_len = simple_strtoul(argv[4], NULL, 16);
			/* fall through */
		case 4:
			src = simple_strtoul(argv[1], NULL, 16);
			src_len = simple_strtoul(argv[2], NULL, 16);
			dst = simple_strtoul(argv[3], NULL, 16);
			break;
		default:
			return cmd_usage(cmdtp);
	}

	if (gzip((void *) dst, &dst_len, (void *) src, src_len) != 0)
		return 1;

	printf("Compressed size: %lu = 0x%lX\n", dst_len, dst_len);
	env_set_hex("filesize", dst_len);

	return 0;
}

U_BOOT_CMD(
	zip,	5,	1,	do_zip,
	"zip a memory region",
	"srcaddr srcsize dstaddr [dstsize]"
);
