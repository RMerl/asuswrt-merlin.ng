// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <nand.h>
#include <spl.h>
#include <test/spl.h>
#include <test/ut.h>

uint32_t spl_nand_get_uboot_raw_page(void);

static int spl_test_nand_write_image(struct unit_test_state *uts, void *img,
				     size_t img_size)
{
	uint32_t off = spl_nand_get_uboot_raw_page();
	struct mtd_info *mtd;
	struct erase_info erase = { };
	size_t length;

	nand_reinit();
	mtd = get_nand_dev_by_index(0);
	ut_assertnonnull(mtd);

	/* Mark the first block as bad to test that it gets skipped */
	ut_assertok(mtd_block_markbad(mtd, off & ~mtd->erasesize_mask));
	off += mtd->erasesize;

	erase.mtd = mtd;
	erase.len = img_size + (off & mtd->erasesize_mask);
	erase.len += mtd->erasesize_mask;
	erase.len &= ~mtd->erasesize_mask;
	erase.addr = off & ~mtd->erasesize_mask;
	erase.scrub = 1;
	ut_assertok(mtd_erase(mtd, &erase));

	ut_assertok(mtd_write(mtd, off, img_size, &length, img));

	return 0;
}

static int spl_test_nand(struct unit_test_state *uts, const char *test_name,
			 enum spl_test_image type)
{
	return do_spl_test_load(uts, test_name, type,
				SPL_LOAD_IMAGE_GET(1, BOOT_DEVICE_NAND,
						   spl_nand_load_image),
				spl_test_nand_write_image);
}
SPL_IMG_TEST(spl_test_nand, LEGACY, DM_FLAGS);
SPL_IMG_TEST(spl_test_nand, LEGACY_LZMA, DM_FLAGS);
SPL_IMG_TEST(spl_test_nand, IMX8, DM_FLAGS);
SPL_IMG_TEST(spl_test_nand, FIT_INTERNAL, DM_FLAGS);
#if !IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)
SPL_IMG_TEST(spl_test_nand, FIT_EXTERNAL, DM_FLAGS);
#endif
