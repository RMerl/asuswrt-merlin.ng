// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <spl.h>
#include <linux/libfdt.h>
#include <nand.h>
#include <stdio.h>
#include <linux/io.h>
#include <linux/printk.h>
#include <../drivers/mtd/nand/raw/denali.h>

#include "init.h"

static void nand_denali_wp_disable(void)
{
#ifdef CONFIG_NAND_DENALI
	/*
	 * Since the boot rom enables the write protection for NAND boot mode,
	 * it must be disabled somewhere for "nand write", "nand erase", etc.
	 * The workaround is here to not disturb the Denali NAND controller
	 * driver just for a really SoC-specific thing.
	 */
	void __iomem *denali_reg = (void __iomem *)CONFIG_SYS_NAND_REGS_BASE;

	writel(WRITE_PROTECT__FLAG, denali_reg + WRITE_PROTECT);
#endif
}

static int uniphier_set_fdt_file(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const char *compat;
	char dtb_name[256];
	int buf_len = sizeof(dtb_name);

	if (env_get("fdtfile"))
		return 0;	/* do nothing if it is already set */

	compat = fdt_stringlist_get(gd->fdt_blob, 0, "compatible", 0, NULL);
	if (!compat)
		return -EINVAL;

	/* rip off the vendor prefix "socionext,"  */
	compat = strchr(compat, ',');
	if (!compat)
		return -EINVAL;
	compat++;

	strncpy(dtb_name, compat, buf_len);
	buf_len -= strlen(compat);

	strncat(dtb_name, ".dtb", buf_len);

	return env_set("fdtfile", dtb_name);
}

int board_late_init(void)
{
	puts("MODE:  ");

	switch (uniphier_boot_device_raw()) {
	case BOOT_DEVICE_MMC1:
		printf("eMMC Boot");
		env_set("bootdev", "emmc");
		break;
	case BOOT_DEVICE_NAND:
		printf("NAND Boot");
		env_set("bootdev", "nand");
		nand_denali_wp_disable();
		break;
	case BOOT_DEVICE_NOR:
		printf("NOR Boot");
		env_set("bootdev", "nor");
		break;
	case BOOT_DEVICE_USB:
		printf("USB Boot");
		env_set("bootdev", "usb");
		break;
	default:
		printf("Unknown");
		break;
	}

	if (uniphier_have_internal_stm())
		printf(" (STM: %s)",
		       uniphier_boot_from_backend() ? "OFF" : "ON");

	printf("\n");

	if (uniphier_set_fdt_file())
		pr_warn("fdt_file environment was not set correctly\n");

	return 0;
}
