// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 SAMSUNG Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 */

#include <common.h>
#include <cros_ec.h>
#include <env.h>
#include <errno.h>
#include <fdtdec.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <spi.h>
#include <tmu.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/board.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/system.h>
#include <i2c.h>
#include <mmc.h>
#include <stdio_dev.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/delay.h>
#include <samsung/misc.h>
#include <dm/pinctrl.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int exynos_early_init_f(void)
{
	return 0;
}

__weak void exynos_init(void)
{
}

__weak int exynos_power_init(void)
{
	return 0;
}

/**
 * get_boot_mmc_dev() - read boot MMC device id from XOM[7:5] pins.
 */
static int get_boot_mmc_dev(void)
{
	u32 mode = readl(EXYNOS4_OP_MODE) & 0x1C;

	if (mode == 0x04)
		return 2; /* MMC2: SD */

	/* MMC0: eMMC or unknown */
	return 0;
}

#if defined CONFIG_EXYNOS_TMU
/* Boot Time Thermal Analysis for SoC temperature threshold breach */
static void boot_temp_check(void)
{
	int temp;

	switch (tmu_monitor(&temp)) {
	case TMU_STATUS_NORMAL:
		break;
	case TMU_STATUS_TRIPPED:
		/*
		 * Status TRIPPED ans WARNING means corresponding threshold
		 * breach
		 */
		puts("EXYNOS_TMU: TRIPPING! Device power going down ...\n");
		set_ps_hold_ctrl();
		hang();
		break;
	case TMU_STATUS_WARNING:
		puts("EXYNOS_TMU: WARNING! Temperature very high\n");
		break;
	case TMU_STATUS_INIT:
		/*
		 * TMU_STATUS_INIT means something is wrong with temperature
		 * sensing and TMU status was changed back from NORMAL to INIT.
		 */
		puts("EXYNOS_TMU: WARNING! Temperature sensing not done\n");
		break;
	default:
		debug("EXYNOS_TMU: Unknown TMU state\n");
	}
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
#if defined CONFIG_EXYNOS_TMU
	if (tmu_init(gd->fdt_blob) != TMU_STATUS_NORMAL) {
		debug("%s: Failed to init TMU\n", __func__);
		return -1;
	}
	boot_temp_check();
#endif
#if CONFIG_VAL(SYS_MEM_TOP_HIDE)
	/* The last few MB of memory can be reserved for secure firmware */
	ulong size = CONFIG_SYS_MEM_TOP_HIDE;

	gd->ram_size -= size;
	gd->bd->bi_dram[CONFIG_NR_DRAM_BANKS - 1].size -= size;
#endif
	exynos_init();

	return 0;
}

int dram_init(void)
{
	unsigned int i;
	unsigned long addr;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CFG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		gd->ram_size += get_ram_size((long *)addr, SDRAM_BANK_SIZE);
	}
	return 0;
}

int dram_init_banksize(void)
{
	unsigned int i;
	unsigned long addr, size;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CFG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		size = get_ram_size((long *)addr, SDRAM_BANK_SIZE);

		gd->bd->bi_dram[i].start = addr;
		gd->bd->bi_dram[i].size = size;
	}

	return 0;
}

static int board_uart_init(void)
{
#ifndef CONFIG_PINCTRL_EXYNOS
	int err, uart_id, ret = 0;

	for (uart_id = PERIPH_ID_UART0; uart_id <= PERIPH_ID_UART3; uart_id++) {
		err = exynos_pinmux_config(uart_id, PINMUX_FLAG_NONE);
		if (err) {
			debug("UART%d not configured\n",
			      (uart_id - PERIPH_ID_UART0));
			ret |= err;
		}
	}
	return ret;
#else
	return 0;
#endif
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int err;
#ifdef CONFIG_BOARD_TYPES
	set_board_type();
#endif
	err = board_uart_init();
	if (err) {
		debug("UART init failed\n");
		return err;
	}

	return exynos_early_init_f();
}
#endif

#if CONFIG_IS_ENABLED(POWER_LEGACY) || CONFIG_IS_ENABLED(DM_PMIC)
int power_init_board(void)
{
	set_ps_hold_ctrl();

	return exynos_power_init();
}
#endif

#if defined(CONFIG_DISPLAY_BOARDINFO) || defined(CONFIG_DISPLAY_BOARDINFO_LATE)
int checkboard(void)
{
	if (IS_ENABLED(CONFIG_BOARD_TYPES)) {
		const char *board_info;

		if (IS_ENABLED(CONFIG_DISPLAY_BOARDINFO_LATE)) {
			/*
			 * Printing type requires having revision, although
			 * this will succeed only if done late.
			 * Otherwise revision will be set in misc_init_r().
			 */
			set_board_revision();
		}

		board_info = get_board_type();

		if (board_info)
			printf("Type:  %s\n", board_info);
	}

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
	int ret;
	int mmcbootdev = get_boot_mmc_dev();
	char mmcbootdev_str[16];

	ret = uclass_first_device_err(UCLASS_CROS_EC, &dev);
	if (ret && ret != -ENODEV && ret != -EPFNOSUPPORT) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n", ret);
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}

	printf("Boot device: MMC(%u)\n", mmcbootdev);
	sprintf(mmcbootdev_str, "%u", mmcbootdev);
	env_set("mmcbootdev", mmcbootdev_str);

	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	if (IS_ENABLED(CONFIG_BOARD_TYPES) &&
	    !IS_ENABLED(CONFIG_DISPLAY_BOARDINFO_LATE)) {
		/*
		 * If revision was not set by late display boardinfo,
		 * set it here. At this point regulators should be already
		 * available.
		 */
		set_board_revision();
	}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
#endif
#ifdef CONFIG_CMD_BMP
	if (panel_info.logo_on)
		draw_logo();
#endif
	return 0;
}
#endif

void reset_misc(void)
{
	struct gpio_desc gpio = {};
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
			"samsung,emmc-reset");
	if (node < 0)
		return;

	gpio_request_by_name_nodev(offset_to_ofnode(node), "reset-gpio", 0,
				   &gpio, GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&gpio)) {
		/*
		 * Reset eMMC
		 *
		 * FIXME: Need to optimize delay time. Minimum 1usec pulse is
		 *	  required by 'JEDEC Standard No.84-A441' (eMMC)
		 *	  document but real delay time is expected to greater
		 *	  than 1usec.
		 */
		dm_gpio_set_value(&gpio, 0);
		mdelay(10);
		dm_gpio_set_value(&gpio, 1);
	}
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
#ifdef CONFIG_USB_DWC3
	dwc3_uboot_exit(index);
#endif
	return 0;
}

int mmc_get_env_dev(void)
{
	return get_boot_mmc_dev();
}
