// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <spl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>

#include "../sg-regs.h"
#include "boot-device.h"

const struct uniphier_boot_device uniphier_pxs3_boot_device_table[] = {
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         4bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         4bit, 1.8V, Training On)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         8bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (Legacy,         8bit, 1.8V, Training On)"},
	{BOOT_DEVICE_MMC1, "eMMC (High Speed SDR, 8bit, 1.8V, Training Off)"},
	{BOOT_DEVICE_MMC1, "eMMC (High Speed SDR, 8bit, 1.8V, Training On)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 5, BBM Last Page)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 5, BBM Last Page)"},
};

const unsigned uniphier_pxs3_boot_device_count =
				ARRAY_SIZE(uniphier_pxs3_boot_device_table);

int uniphier_pxs3_boot_device_is_usb(u32 pinmon)
{
	return !!(readl(sg_base + SG_PINMON2) & BIT(31));
}
