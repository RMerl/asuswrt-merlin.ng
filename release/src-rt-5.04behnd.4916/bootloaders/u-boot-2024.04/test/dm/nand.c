// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <nand.h>
#include <part.h>
#include <rand.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>

static int dm_test_nand(struct unit_test_state *uts, int dev, bool end)
{
	nand_erase_options_t opts = { };
	struct mtd_info *mtd;
	size_t length;
	loff_t size;
	char *buf;
	int *gold;
	u8 oob[NAND_MAX_OOBSIZE];
	int i;
	loff_t off = 0;
	mtd_oob_ops_t ops = { };

	/* Seed RNG for bit errors */
	srand((off >> 32) ^ off ^ ~dev);

	mtd = get_nand_dev_by_index(dev);
	ut_assertnonnull(mtd);
	size = mtd->erasesize * 4;
	length = size;

	buf = malloc(size);
	ut_assertnonnull(buf);
	gold = malloc(size);
	ut_assertnonnull(gold);

	/* Mark a block as bad */
	ut_assertok(mtd_block_markbad(mtd, off + mtd->erasesize));

	/* Erase some stuff */
	if (end)
		off = mtd->size - size - mtd->erasesize;
	opts.offset = off;
	opts.length = size;
	opts.spread = 1;
	opts.lim = U32_MAX;
	ut_assertok(nand_erase_opts(mtd, &opts));

	/* Make sure everything is erased */
	memset(gold, 0xff, size);
	ut_assertok(nand_read_skip_bad(mtd, off, &length, NULL, U64_MAX, buf));
	ut_asserteq(size, length);
	ut_asserteq_mem(gold, buf, size);

	/* ...but our bad block marker is still there */
	ops.oobbuf = oob;
	ops.ooblen = mtd->oobsize;
	ut_assertok(mtd_read_oob(mtd, mtd->erasesize, &ops));
	ut_asserteq(0, oob[mtd_to_nand(mtd)->badblockpos]);

	/* Generate some data and write it */
	for (i = 0; i < size / sizeof(int); i++)
		gold[i] = rand();
	ut_assertok(nand_write_skip_bad(mtd, off, &length, NULL, U64_MAX,
					(void *)gold, 0));
	ut_asserteq(size, length);

	/* Verify */
	ut_assertok(nand_read_skip_bad(mtd, off, &length, NULL, U64_MAX, buf));
	ut_asserteq(size, length);
	ut_asserteq_mem(gold, buf, size);

	/* Erase some blocks */
	memset(((char *)gold) + mtd->erasesize, 0xff, mtd->erasesize * 2);
	opts.offset = off + mtd->erasesize;
	opts.length = mtd->erasesize * 2;
	ut_assertok(nand_erase_opts(mtd, &opts));

	/* Verify */
	ut_assertok(nand_read_skip_bad(mtd, off, &length, NULL, U64_MAX, buf));
	ut_asserteq(size, length);
	ut_asserteq_mem(gold, buf, size);

	return 0;
}

#define DM_NAND_TEST(dev) \
static int dm_test_nand##dev##_start(struct unit_test_state *uts) \
{ \
	return dm_test_nand(uts, dev, false); \
} \
DM_TEST(dm_test_nand##dev##_start, UT_TESTF_SCAN_FDT); \
static int dm_test_nand##dev##_end(struct unit_test_state *uts) \
{ \
	return dm_test_nand(uts, dev, true); \
} \
DM_TEST(dm_test_nand##dev##_end, UT_TESTF_SCAN_FDT)

DM_NAND_TEST(0);
DM_NAND_TEST(1);
