// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <clk.h>
#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <fdtdec.h>
#include <led.h>
#include <malloc.h>
#include <ram.h>
#include <spl.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3188.h>
#include <asm/arch-rockchip/sdram.h>
#include <asm/arch-rockchip/timer.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	const char *bootdev;
	int node;
	int ret;

	bootdev = fdtdec_get_config_string(blob, "u-boot,boot0");
	debug("Boot device %s\n", bootdev);
	if (!bootdev)
		goto fallback;

	node = fdt_path_offset(blob, bootdev);
	if (node < 0) {
		debug("node=%d\n", node);
		goto fallback;
	}
	ret = device_get_global_by_ofnode(offset_to_ofnode(node), &dev);
	if (ret) {
		debug("device at node %s/%d not found: %d\n", bootdev, node,
		      ret);
		goto fallback;
	}
	debug("Found device %s\n", dev->name);
	switch (device_get_uclass_id(dev)) {
	case UCLASS_SPI_FLASH:
		return BOOT_DEVICE_SPI;
	case UCLASS_MMC:
		return BOOT_DEVICE_MMC1;
	default:
		debug("Booting from device uclass '%s' not supported\n",
		      dev_get_uclass_name(dev));
	}

fallback:
#endif
	return BOOT_DEVICE_MMC1;
}

static int setup_arm_clock(void)
{
	struct udevice *dev;
	struct clk clk;
	int ret;

	ret = rockchip_get_clk(&dev);
	if (ret)
		return ret;

	clk.id = CLK_ARM;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return ret;

	ret = clk_set_rate(&clk, 600000000);

	clk_free(&clk);
	return ret;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#ifdef CONFIG_DEBUG_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
	printascii("U-Boot SPL board init");
#endif

#ifdef CONFIG_ROCKCHIP_USB_UART
	rk_clrsetreg(&grf->uoc0_con[0],
		     SIDDQ_MASK | UOC_DISABLE_MASK | COMMON_ON_N_MASK,
		     1 << SIDDQ_SHIFT | 1 << UOC_DISABLE_SHIFT |
		     1 << COMMON_ON_N_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[2],
		     SOFT_CON_SEL_MASK, 1 << SOFT_CON_SEL_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[3],
		     OPMODE_MASK | XCVRSELECT_MASK |
		     TERMSEL_FULLSPEED_MASK | SUSPENDN_MASK,
		     OPMODE_NODRIVING << OPMODE_SHIFT |
		     XCVRSELECT_FSTRANSC << XCVRSELECT_SHIFT |
		     1 << TERMSEL_FULLSPEED_SHIFT |
		     1 << SUSPENDN_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[0],
		     BYPASSSEL_MASK | BYPASSDMEN_MASK,
		     1 << BYPASSSEL_SHIFT | 1 << BYPASSDMEN_SHIFT);
#endif

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = rockchip_get_clk(&dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}

	setup_arm_clock();
#if CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM) && !defined(CONFIG_SPL_BOARD_INIT)
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
#endif
}

static int setup_led(void)
{
#ifdef CONFIG_SPL_LED
	struct udevice *dev;
	char *led_name;
	int ret;

	led_name = fdtdec_get_config_string(gd->fdt_blob, "u-boot,boot-led");
	if (!led_name)
		return 0;
	ret = led_get_by_label(led_name, &dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}
	ret = led_set_on(dev, 1);
	if (ret)
		return ret;
#endif

	return 0;
}

void spl_board_init(void)
{
	int ret;

	ret = setup_led();
	if (ret) {
		debug("LED ret=%d\n", ret);
		hang();
	}

	preloader_console_init();
#if CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
#endif
	return;
}
