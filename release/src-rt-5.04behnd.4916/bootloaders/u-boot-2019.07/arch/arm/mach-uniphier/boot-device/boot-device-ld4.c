// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>
#include <linux/kernel.h>

#include "boot-device.h"

const struct uniphier_boot_device uniphier_ld4_boot_device_table[] = {
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 24, EraseSize   1MB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 4, ECC 24, EraseSize   1MB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 128KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 256KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 24, EraseSize 512KB, Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 4)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 8, ECC 24, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 4, ECC 24, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC  8, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 16, ONFI,            Addr 5)"},
	{BOOT_DEVICE_NAND, "NAND (Mirror 1, ECC 24, ONFI,            Addr 5)"},
	{BOOT_DEVICE_MMC1, "eMMC (3.3V)"},
	{BOOT_DEVICE_MMC1, "eMMC (1.8V)"},
	{BOOT_DEVICE_NONE, "Reserved"},
	{BOOT_DEVICE_NONE, "Reserved"},
	{BOOT_DEVICE_NONE, "Reserved"},
	{BOOT_DEVICE_NONE, "Reserved"},
	{BOOT_DEVICE_NONE, "Reserved"},
	{BOOT_DEVICE_NOR,  "NOR  (XECS0)"},
};

const unsigned uniphier_ld4_boot_device_count =
				ARRAY_SIZE(uniphier_ld4_boot_device_table);
