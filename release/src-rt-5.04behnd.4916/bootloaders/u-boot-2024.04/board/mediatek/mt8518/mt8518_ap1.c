// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	debug("gd->fdt_blob is %p\n", gd->fdt_blob);
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_USB_GADGET
	struct udevice *dev;
	int ret;
#endif

#ifdef CONFIG_USB_GADGET
	ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, 0, &dev);
	if (ret) {
		pr_err("%s: Cannot find USB device\n", __func__);
		return ret;
	}
#endif

	return 0;
}
