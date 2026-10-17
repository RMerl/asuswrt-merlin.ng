// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <env.h>
#include <init.h>
#include <spl.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <stdio.h>
#include <linux/printk.h>

#include "init.h"

static void uniphier_set_env_fdt_file(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const char *compat;
	char dtb_name[256];
	int buf_len = sizeof(dtb_name);
	int ret;

	if (env_get("fdtfile"))
		return;		/* do nothing if it is already set */

	compat = fdt_stringlist_get(gd->fdt_blob, 0, "compatible", 0, NULL);
	if (!compat)
		goto fail;

	/* rip off the vendor prefix "socionext,"  */
	compat = strchr(compat, ',');
	if (!compat)
		goto fail;
	compat++;

	strncpy(dtb_name, compat, buf_len);
	buf_len -= strlen(compat);

	strncat(dtb_name, ".dtb", buf_len);

	ret = env_set("fdtfile", dtb_name);
	if (ret)
		goto fail;

	return;
fail:
	pr_warn("\"fdt_file\" environment variable was not set correctly\n");
}

static void uniphier_set_env_addr(const char *env, const char *offset_env)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned long offset = 0;
	const char *str;
	char *end;
	int ret;

	if (env_get(env))
		return;		/* do nothing if it is already set */

	if (offset_env) {
		str = env_get(offset_env);
		if (!str)
			goto fail;

		offset = hextoul(str, &end);
		if (*end)
			goto fail;
	}

	ret = env_set_hex(env, gd->ram_base + offset);
	if (ret)
		goto fail;

	return;

fail:
	pr_warn("\"%s\" environment variable was not set correctly\n", env);
}

int board_late_init(void)
{
	puts("MODE:  ");

	switch (uniphier_boot_device_raw()) {
	case BOOT_DEVICE_MMC1:
		printf("eMMC Boot");
		env_set("bootdev", "emmc");
		break;
	case BOOT_DEVICE_MMC2:
		printf("SD Boot");
		env_set("bootdev", "sd");
		break;
	case BOOT_DEVICE_NAND:
		printf("NAND Boot");
		env_set("bootdev", "nand");
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

	uniphier_set_env_fdt_file();

	uniphier_set_env_addr("dram_base", NULL);

	uniphier_set_env_addr("loadaddr", "loadaddr_offset");

	uniphier_set_env_addr("kernel_addr_r", "kernel_addr_r_offset");
	uniphier_set_env_addr("ramdisk_addr_r", "ramdisk_addr_r_offset");
	uniphier_set_env_addr("fdt_addr_r", "fdt_addr_r_offset");

	return 0;
}
