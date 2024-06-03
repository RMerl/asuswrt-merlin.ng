// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2017 Xilinx, Inc. Michal Simek
 */
#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ps7_init_gpl.h>

void board_init_f(ulong dummy)
{
	ps7_init();

	arch_cpu_init();

#ifdef CONFIG_DEBUG_UART
	/* Uart debug for sure */
	debug_uart_init();
	puts("Debug uart enabled\n"); /* or printch() */
#endif
}

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	preloader_console_init();
#if defined(CONFIG_ARCH_EARLY_INIT_R) && defined(CONFIG_SPL_FPGA_SUPPORT)
	arch_early_init_r();
#endif
	board_init();
}
#endif

u32 spl_boot_device(void)
{
	u32 mode;

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
#ifdef CONFIG_SPL_SPI_SUPPORT
	case ZYNQ_BM_QSPI:
		puts("qspi boot\n");
		mode = BOOT_DEVICE_SPI;
		break;
#endif
	case ZYNQ_BM_NAND:
		mode = BOOT_DEVICE_NAND;
		break;
	case ZYNQ_BM_NOR:
		mode = BOOT_DEVICE_NOR;
		break;
#ifdef CONFIG_SPL_MMC_SUPPORT
	case ZYNQ_BM_SD:
		puts("mmc boot\n");
		mode = BOOT_DEVICE_MMC1;
		break;
#endif
	case ZYNQ_BM_JTAG:
		mode = BOOT_DEVICE_RAM;
		break;
	default:
		puts("Unsupported boot mode selected\n");
		hang();
	}

	return mode;
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* boot linux */
	return 0;
}
#endif

void spl_board_prepare_for_boot(void)
{
	ps7_post_config();
	debug("SPL bye\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
