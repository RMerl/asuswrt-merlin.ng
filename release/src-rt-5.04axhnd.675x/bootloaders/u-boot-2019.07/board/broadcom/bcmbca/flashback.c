/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>

#include <asm/arch/ddr.h>
#include <linux/ctype.h>
#include <mtd.h>
#include <nand.h>
#include <stdlib.h>
#include <string.h>
#include <environment.h>
#include <cli.h>
#include <linux/bitops.h>
#include <linux/crc32.h>
#include <ubi_uboot.h>
#include "bca_common.h"
#include "bca_sdk.h"
#include "spl_env.h"

struct recovery_chunks {
	int flashpage;		// page in flash
	int size;		// number of bytes before fill
	int type;		// 0x0= fill with 0xff, 0x1 = fill with 0x00, 0x7fffffff = END
};

DECLARE_GLOBAL_DATA_PTR;

static int erase(struct mtd_info *mtd, int first, int blocks);

static int erase(struct mtd_info *mtd, int first, int blocks)
{
	struct erase_info erase_op = { };
	int ret;

	erase_op.mtd = mtd;
	erase_op.addr = first * mtd->erasesize;
	erase_op.len = blocks * mtd->erasesize;
	erase_op.scrub = 0;
	printf("Erasing %d blocks from block %d\n", blocks, first);

	while (erase_op.len) {
		ret = mtd_erase(mtd, &erase_op);

		/* Abort if its not a bad block error */
		if (ret != -EIO)
			break;

		printf("Skipping bad block at 0x%08llx\n", erase_op.fail_addr);

		/* Skip bad block and continue behind it */
		erase_op.len -= erase_op.fail_addr - erase_op.addr;
		erase_op.len -= mtd->erasesize;
		erase_op.addr = erase_op.fail_addr + mtd->erasesize;
	}

	if (ret && ret != -EIO)
		ret = -1;
	else
		ret = 0;
	return (ret);
};


static int do_flashback(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[]);
static int do_flashback(cmd_tbl_t * cmdtp, int flag, int argc,
			char *const argv[])
{
	struct mtd_info *mtd = NULL;
	struct recovery_chunks *recovery_chunks_list = CONFIG_SYS_LOAD_ADDR;
	long offset;
	int i;
	size_t sz = 0;
	int blocks, pages;
	int ret = 0;
	char *bp;
	int chunk;
	mtd = get_mtd_device_nm("nand0");
	blocks = (mtd->size / mtd->erasesize);
	pages = (mtd->erasesize / mtd->writesize);
	erase(mtd, 0, blocks);
	chunk = 0;
	bp = &recovery_chunks_list[blocks * pages + 1];
	while (recovery_chunks_list[chunk].type < 0x1000) {
		i = mtd_write(mtd,
			      recovery_chunks_list[chunk].flashpage *
			      mtd->writesize, mtd->writesize, &sz, bp);
		bp += recovery_chunks_list[chunk].size;
		chunk++;
		printf("%d ", chunk);
	}
	put_mtd_device(mtd);
	return(0);
}

static char usage[] = "line 1...\n" "line 2...\n";

U_BOOT_CMD_WITH_SUBCMDS(flashback_ops, "flashback commands", usage,
			U_BOOT_SUBCMD_MKENT(go_flashback, 1, 0, do_flashback)
					    );


