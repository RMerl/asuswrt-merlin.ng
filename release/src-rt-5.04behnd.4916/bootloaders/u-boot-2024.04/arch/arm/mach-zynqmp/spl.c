// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 - 2016 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ecc_spl_init.h>
#include <asm/arch/psu_init_gpl.h>
#include <asm/arch/sys_proto.h>

#if defined(CONFIG_DEBUG_UART_BOARD_INIT)
void board_debug_uart_init(void)
{
	psu_uboot_init();
}
#endif

void board_init_f(ulong dummy)
{
#if !defined(CONFIG_DEBUG_UART_BOARD_INIT)
	psu_uboot_init();
#endif

	board_early_init_r();
#ifdef CONFIG_SPL_ZYNQMP_DRAM_ECC_INIT
	zynqmp_ecc_init();
#endif
}

static void ps_mode_reset(ulong mode)
{
	writel(mode << ZYNQMP_CRL_APB_BOOT_PIN_CTRL_OUT_EN_SHIFT,
	       &crlapb_base->boot_pin_ctrl);
	udelay(5);
	writel(mode << ZYNQMP_CRL_APB_BOOT_PIN_CTRL_OUT_VAL_SHIFT |
	       mode << ZYNQMP_CRL_APB_BOOT_PIN_CTRL_OUT_EN_SHIFT,
	       &crlapb_base->boot_pin_ctrl);
}

/*
 * Set default PS_MODE1 which is used for USB ULPI phy reset
 * Also other resets can be connected to this certain pin
 */
#ifndef MODE_RESET
# define MODE_RESET	PS_MODE1
#endif

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	preloader_console_init();
	ps_mode_reset(MODE_RESET);
	board_init();
	psu_post_config_data();
}
#endif

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();

	if (spl_boot_list[0] == BOOT_DEVICE_MMC1)
		spl_boot_list[1] = BOOT_DEVICE_MMC2;
	if (spl_boot_list[0] == BOOT_DEVICE_MMC2)
		spl_boot_list[1] = BOOT_DEVICE_MMC1;

	spl_boot_list[2] = BOOT_DEVICE_RAM;
}

u32 spl_boot_device(void)
{
	u32 reg = 0;
	u8 bootmode;

#if defined(CONFIG_SPL_ZYNQMP_ALT_BOOTMODE_ENABLED)
	/* Change default boot mode at run-time */
	writel(CONFIG_SPL_ZYNQMP_ALT_BOOTMODE << BOOT_MODE_ALT_SHIFT,
	       &crlapb_base->boot_mode);
#endif

	reg = readl(&crlapb_base->boot_mode);
	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	switch (bootmode) {
	case JTAG_MODE:
		return BOOT_DEVICE_RAM;
#ifdef CONFIG_SPL_MMC
	case SD_MODE1:
	case SD1_LSHFT_MODE: /* not working on silicon v1 */
		return BOOT_DEVICE_MMC2;
	case SD_MODE:
	case EMMC_MODE:
		return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_DFU
	case USB_MODE:
		return BOOT_DEVICE_DFU;
#endif
#ifdef CONFIG_SPL_SATA
	case SW_SATA_MODE:
		return BOOT_DEVICE_SATA;
#endif
#ifdef CONFIG_SPL_SPI
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		return BOOT_DEVICE_SPI;
#endif
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	return 0;
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 0;
}
#endif
