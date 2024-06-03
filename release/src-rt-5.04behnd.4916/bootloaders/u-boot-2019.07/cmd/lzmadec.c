// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Patrice Bouchand <pbfwdlist_gmail_com>
 * lzma uncompress command in Uboot
 *
 * made from existing cmd_unzip.c file of Uboot
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <mapmem.h>
#include <asm/io.h>

#include <lzma/LzmaTools.h>

static int do_lzmadec(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long src, dst;
	SizeT src_len = ~0UL, dst_len = ~0UL;
	int ret;

	switch (argc) {
	case 4:
		dst_len = simple_strtoul(argv[3], NULL, 16);
		/* fall through */
	case 3:
		src = simple_strtoul(argv[1], NULL, 16);
		dst = simple_strtoul(argv[2], NULL, 16);
		break;
	default:
		return CMD_RET_USAGE;
	}

	ret = lzmaBuffToBuffDecompress(map_sysmem(dst, dst_len), &src_len,
				       map_sysmem(src, 0), dst_len);

	if (ret != SZ_OK)
		return 1;
	printf("Uncompressed size: %ld = %#lX\n", (ulong)src_len,
	       (ulong)src_len);
	env_set_hex("filesize", src_len);

	return 0;
}

U_BOOT_CMD(
	lzmadec,    4,    1,    do_lzmadec,
	"lzma uncompress a memory region",
	"srcaddr dstaddr [dstsize]"
);
