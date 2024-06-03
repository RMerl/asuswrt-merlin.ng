// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>
#include <dm/uclass.h>

/*
 * Second Watchdog Timer by default is configured
 * to trigger secondary boot source.
 */
#define AST_2ND_BOOT_WDT		1

/*
 * Third Watchdog Timer by default is configured
 * to toggle Flash address mode switch before reset.
 */
#define AST_FLASH_ADDR_DETECT_WDT	2

DECLARE_GLOBAL_DATA_PTR;

void lowlevel_init(void)
{
	/*
	 * These two watchdogs need to be stopped as soon as possible,
	 * otherwise the board might hang. By default they are set to
	 * a very short timeout and even simple debug write to serial
	 * console early in the init process might cause them to fire.
	 */
	struct ast_wdt *flash_addr_wdt =
	    (struct ast_wdt *)(WDT_BASE +
			       sizeof(struct ast_wdt) *
			       AST_FLASH_ADDR_DETECT_WDT);

	clrbits_le32(&flash_addr_wdt->ctrl, WDT_CTRL_EN);

#ifndef CONFIG_FIRMWARE_2ND_BOOT
	struct ast_wdt *sec_boot_wdt =
	    (struct ast_wdt *)(WDT_BASE +
			       sizeof(struct ast_wdt) *
			       AST_2ND_BOOT_WDT);

	clrbits_le32(&sec_boot_wdt->ctrl, WDT_CTRL_EN);
#endif
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	struct udevice *dev;
	struct ram_info ram;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM FAIL1\r\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("DRAM FAIL2\r\n");
		return ret;
	}

	gd->ram_size = ram.size;

	return 0;
}
