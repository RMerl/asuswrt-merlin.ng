// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <spl.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);

	return 0;
}

__weak const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
};

const char *board_spl_was_booted_from(void)
{
	u32  bootdevice_brom_id = readl(BROM_BOOTSOURCE_ID_ADDR);
	const char *bootdevice_ofpath = NULL;

	if (bootdevice_brom_id < ARRAY_SIZE(boot_devices))
		bootdevice_ofpath = boot_devices[bootdevice_brom_id];

	if (bootdevice_ofpath)
		debug("%s: brom_bootdevice_id %x maps to '%s'\n",
		      __func__, bootdevice_brom_id, bootdevice_ofpath);
	else
		debug("%s: failed to resolve brom_bootdevice_id %x\n",
		      __func__, bootdevice_brom_id);

	return bootdevice_ofpath;
}

u32 spl_boot_device(void)
{
	u32 boot_device = BOOT_DEVICE_MMC1;

#if defined(CONFIG_TARGET_CHROMEBOOK_JERRY) || \
		defined(CONFIG_TARGET_CHROMEBIT_MICKEY) || \
		defined(CONFIG_TARGET_CHROMEBOOK_MINNIE) || \
		defined(CONFIG_TARGET_CHROMEBOOK_SPEEDY) || \
		defined(CONFIG_TARGET_CHROMEBOOK_BOB) || \
		defined(CONFIG_TARGET_CHROMEBOOK_KEVIN)
	return BOOT_DEVICE_SPI;
#endif
	if (CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM))
		return BOOT_DEVICE_BOOTROM;

	return boot_device;
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

#define TIMER_LOAD_COUNT_L	0x00
#define TIMER_LOAD_COUNT_H	0x04
#define TIMER_CONTROL_REG	0x10
#define TIMER_EN	0x1
#define	TIMER_FMODE	BIT(0)
#define	TIMER_RMODE	BIT(1)

__weak void rockchip_stimer_init(void)
{
#if defined(CONFIG_ROCKCHIP_STIMER_BASE)
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);

	if (reg & TIMER_EN)
		return;
#ifndef CONFIG_ARM64
	asm volatile("mcr p15, 0, %0, c14, c0, 0"
		     : : "r"(CONFIG_COUNTER_FREQUENCY));
#endif
	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 4);
	writel(TIMER_EN | TIMER_FMODE, CONFIG_ROCKCHIP_STIMER_BASE +
	       TIMER_CONTROL_REG);
#endif
}

__weak int board_early_init_f(void)
{
	return 0;
}

__weak int arch_cpu_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	board_early_init_f();

	ret = spl_early_init();
	if (ret) {
		printf("spl_early_init() failed: %d\n", ret);
		hang();
	}
	arch_cpu_init();

	rockchip_stimer_init();

#ifdef CONFIG_SYS_ARCH_TIMER
	/* Init ARM arch timer in arch/arm/cpu/armv7/arch_timer.c */
	timer_init();
#endif
#if !defined(CONFIG_TPL) || defined(CONFIG_SPL_RAM)
	debug("\nspl:init dram\n");
	ret = dram_init();
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return;
	}
	gd->ram_top = gd->ram_base + get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->ram_size);
#endif
	preloader_console_init();
}
