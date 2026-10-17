// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <spl.h>
#include <linux/io.h>
#include <linux/kernel.h>

#include "boot-device.h"

const struct uniphier_boot_device uniphier_pxs2_boot_device_table[] = {
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 4)"},
	{BOOT_DEVICE_MMC1, "eMMC (1.8V)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, ONFI,            Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, ONFI,            Addr 4)"},
	{BOOT_DEVICE_SPI,  "SPI  (3Byte CS0)"},
	{BOOT_DEVICE_SPI,  "SPI  (4Byte CS0)"},
	{BOOT_DEVICE_SPI,  "SPI  (3Byte CS1)"},
	{BOOT_DEVICE_SPI,  "SPI  (4Byte CS1)"},
	{BOOT_DEVICE_SPI,  "SPI  (4Byte CS0)"},
	{BOOT_DEVICE_SPI,  "SPI  (3Byte CS0)"},
	{BOOT_DEVICE_NONE, "Reserved"},
};

const unsigned uniphier_pxs2_boot_device_count =
				ARRAY_SIZE(uniphier_pxs2_boot_device_table);

int uniphier_pxs2_boot_device_is_usb(u32 pinmon)
{
	return !!(pinmon & 0x00000040);
}

unsigned int uniphier_pxs2_boot_device_fixup(unsigned int mode)
{
	if (mode == BOOT_DEVICE_USB)
		return BOOT_DEVICE_NOR;

	return mode;
}
