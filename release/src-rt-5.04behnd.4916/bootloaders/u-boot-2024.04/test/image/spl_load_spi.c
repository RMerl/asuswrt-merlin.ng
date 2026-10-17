// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <spi_flash.h>
#include <spl.h>
#include <test/spl.h>
#include <test/ut.h>

static int spl_test_spi_write_image(struct unit_test_state *uts, void *img,
				    size_t img_size)
{
	struct spi_flash *flash;

	flash = spi_flash_probe(spl_spi_boot_bus(), spl_spi_boot_cs(),
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ut_assertnonnull(flash);
	ut_assertok(spi_flash_write(flash, spl_spi_get_uboot_offs(flash),
				    img_size, img));

	return 0;
}

static int spl_test_spi(struct unit_test_state *uts, const char *test_name,
			enum spl_test_image type)
{
	return do_spl_test_load(uts, test_name, type,
				SPL_LOAD_IMAGE_GET(1, BOOT_DEVICE_SPI,
						   spl_spi_load_image),
				spl_test_spi_write_image);
}
SPL_IMG_TEST(spl_test_spi, LEGACY, DM_FLAGS);
SPL_IMG_TEST(spl_test_spi, LEGACY_LZMA, DM_FLAGS);
SPL_IMG_TEST(spl_test_spi, IMX8, DM_FLAGS);
SPL_IMG_TEST(spl_test_spi, FIT_INTERNAL, DM_FLAGS);
#if !IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)
SPL_IMG_TEST(spl_test_spi, FIT_EXTERNAL, DM_FLAGS);
#endif
