// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/io.h>
#include <test/spl.h>
#include <test/ut.h>

static void *spl_test_nor_base;

unsigned long spl_nor_get_uboot_base(void)
{
	return virt_to_phys(spl_test_nor_base);
}

static int spl_test_nor_write_image(struct unit_test_state *uts, void *img,
				    size_t img_size)
{
	spl_test_nor_base = img;
	return 0;
}

static int spl_test_nor(struct unit_test_state *uts, const char *test_name,
			enum spl_test_image type)
{
	return do_spl_test_load(uts, test_name, type,
				SPL_LOAD_IMAGE_GET(0, BOOT_DEVICE_NOR,
						   spl_nor_load_image),
				spl_test_nor_write_image);
}
SPL_IMG_TEST(spl_test_nor, LEGACY, 0);
SPL_IMG_TEST(spl_test_nor, LEGACY_LZMA, 0);
SPL_IMG_TEST(spl_test_nor, IMX8, 0);
SPL_IMG_TEST(spl_test_nor, FIT_INTERNAL, 0);
#if !IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)
SPL_IMG_TEST(spl_test_nor, FIT_EXTERNAL, 0);
#endif
