// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 - 2016 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

void board_init_f(ulong dummy)
{
	board_early_init_f();
	board_early_init_r();

#ifdef CONFIG_DEBUG_UART
	/* Uart debug for sure */
	debug_uart_init();
	puts("Debug uart enabled\n"); /* or printch() */
#endif
	/* Delay is required for clocks to be propagated */
	udelay(1000000);

	debug("Clearing BSS 0x%p - 0x%p\n", __bss_start, __bss_end);
	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* No need to call timer init - it is empty for ZynqMP */
	board_init_r(NULL, 0);
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
}
#endif

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
#ifdef CONFIG_SPL_MMC_SUPPORT
	case SD_MODE1:
	case SD1_LSHFT_MODE: /* not working on silicon v1 */
/* if both controllers enabled, then these two are the second controller */
#ifdef CONFIG_SPL_ZYNQMP_TWO_SDHCI
		return BOOT_DEVICE_MMC2;
/* else, fall through, the one SDHCI controller that is enabled is number 1 */
#endif
	case SD_MODE:
	case EMMC_MODE:
		return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_DFU
	case USB_MODE:
		return BOOT_DEVICE_DFU;
#endif
#ifdef CONFIG_SPL_SATA_SUPPORT
	case SW_SATA_MODE:
		return BOOT_DEVICE_SATA;
#endif
#ifdef CONFIG_SPL_SPI_SUPPORT
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
	handoff_setup();

	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
