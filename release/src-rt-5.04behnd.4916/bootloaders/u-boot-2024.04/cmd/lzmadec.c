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
#include <env.h>
#include <mapmem.h>
#include <asm/io.h>

#include <lzma/LzmaTools.h>

static int do_lzmadec(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	unsigned long src, dst;
	SizeT src_len = ~0UL, dst_len = ~0UL;
	int ret;

	switch (argc) {
	case 4:
		dst_len = hextoul(argv[3], NULL);
		/* fall through */
	case 3:
		src = hextoul(argv[1], NULL);
		dst = hextoul(argv[2], NULL);
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
