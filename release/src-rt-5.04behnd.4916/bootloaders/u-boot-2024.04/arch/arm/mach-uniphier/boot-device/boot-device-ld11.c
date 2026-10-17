// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <spl.h>
#include <linux/io.h>
#include <linux/kernel.h>

#include "boot-device.h"

const struct uniphier_boot_device uniphier_ld11_boot_device_table[] = {
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, ONFI             Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, ONFI             Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, ONFI             Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, ONFI             Addr 5)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         4bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         4bit, 1.8V, Training On)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         8bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         8bit, 1.8V, Training On)"},
	{BOOT_DEVICE_MMC1, "eMMC (High Speed SDR, 8bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (High Speed SDR, 8bit, 1.8V, Training On)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         4bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_NOR,  "NOR  (XECS1)"},
};

const unsigned uniphier_ld11_boot_device_count =
				ARRAY_SIZE(uniphier_ld11_boot_device_table);

int uniphier_ld11_boot_device_is_usb(u32 pinmon)
{
	return !!(~pinmon & 0x00000080);
}

int uniphier_ld20_boot_device_is_usb(u32 pinmon)
{
	return !!(~pinmon & 0x00000780);
}
