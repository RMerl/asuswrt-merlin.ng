// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020
 * FUJITSU COMPUTERTECHNOLOGIES LIMITED. All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <u-boot/lz4.h>

static int do_unlz4(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	unsigned long src, dst;
	size_t src_len = ~0UL, dst_len = ~0UL;
	int ret;

	switch (argc) {
	case 4:
		src = hextoul(argv[1], NULL);
		dst = hextoul(argv[2], NULL);
		dst_len = hextoul(argv[3], NULL);
		break;
	default:
		return CMD_RET_USAGE;
	}

	ret = ulz4fn((void *)src, src_len, (void *)dst, &dst_len);
	if (ret) {
		printf("Uncompressed err :%d\n", ret);
		return 1;
	}

	printf("Uncompressed size: %zd = 0x%zX\n", dst_len, dst_len);
	env_set_hex("filesize", dst_len);

	return 0;
}

U_BOOT_CMD(unlz4, 4, 1, do_unlz4,
	   "lz4 uncompress a memory region",
	   "srcaddr dstaddr dstsize\n"
	   "NOTE: Specify the destination size that is sufficiently larger\n"
	   " than the source size.\n"
);
