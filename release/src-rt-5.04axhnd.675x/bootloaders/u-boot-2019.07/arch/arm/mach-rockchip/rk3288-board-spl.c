// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <led.h>
#include <malloc.h>
#include <ram.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/sdram.h>
#include <asm/arch-rockchip/sdram_common.h>
#include <asm/arch-rockchip/sys_proto.h>
#include <asm/arch-rockchip/timer.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <power/regulator.h>
#include <power/rk8xx_pmic.h>

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
#elif defined(CONFIG_TARGET_CHROMEBOOK_JERRY) || \
		defined(CONFIG_TARGET_CHROMEBIT_MICKEY) || \
		defined(CONFIG_TARGET_CHROMEBOOK_MINNIE) || \
		defined(CONFIG_TARGET_CHROMEBOOK_SPEEDY)
	return BOOT_DEVICE_SPI;
#endif
	return BOOT_DEVICE_MMC1;
}

#if !defined(CONFIG_SPL_OF_PLATDATA)
static int phycore_init(void)
{
	struct udevice *pmic;
	int ret;

	ret = uclass_first_device_err(UCLASS_PMIC, &pmic);
	if (ret)
		return ret;

#if defined(CONFIG_SPL_POWER_SUPPORT)
	/* Increase USB input current to 2A */
	ret = rk818_spl_configure_usb_input_current(pmic, 2000);
	if (ret)
		return ret;

	/* Close charger when USB lower then 3.26V */
	ret = rk818_spl_configure_usb_chrg_shutdown(pmic, 3260000);
	if (ret)
		return ret;
#endif

	return 0;
}
#endif

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
	debug("\nspl:debug uart enabled in %s\n", __func__);
#endif
	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	rockchip_timer_init();
	configure_l2ctlr();

	ret = rockchip_get_clk(&dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return;
	}

#if !defined(CONFIG_SPL_OF_PLATDATA)
	if (of_machine_is_compatible("phytec,rk3288-phycore-som")) {
		ret = phycore_init();
		if (ret) {
			debug("Failed to set up phycore power settings: %d\n",
			      ret);
			return;
		}
	}
#endif

#if !defined(CONFIG_SUPPORT_TPL)
	debug("\nspl:init dram\n");
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
#endif

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

#ifdef CONFIG_SPL_OS_BOOT

#define PMU_BASE		0xff730000
int dram_init_banksize(void)
{
	struct rk3288_pmu *const pmu = (void *)PMU_BASE;
	size_t size = rockchip_sdram_size((phys_addr_t)&pmu->sys_reg[2]);

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = size;

	return 0;
}
#endif
