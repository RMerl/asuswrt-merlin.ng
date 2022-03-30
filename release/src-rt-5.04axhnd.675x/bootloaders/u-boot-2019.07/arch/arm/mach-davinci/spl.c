// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <nand.h>
#include <asm/arch/dm365_lowlevel.h>
#include <ns16550.h>
#include <malloc.h>
#include <spi_flash.h>
#include <mmc.h>

#ifndef CONFIG_SPL_LIBCOMMON_SUPPORT
void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)(CONFIG_SYS_NS16550_COM1), '\r');

	NS16550_putc((NS16550_t)(CONFIG_SYS_NS16550_COM1), c);
}
#endif /* CONFIG_SPL_LIBCOMMON_SUPPORT */

void board_init_f(ulong dummy)
{
	arch_cpu_init();

	spl_early_init();

	preloader_console_init();
}

u32 spl_boot_device(void)
{
	switch (davinci_syscfg_regs->bootcfg) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	case DAVINCI_NAND8_BOOT:
	case DAVINCI_NAND16_BOOT:
		return BOOT_DEVICE_NAND;
#endif

#ifdef CONFIG_SPL_MMC_SUPPORT
	case DAVINCI_SD_OR_MMC_BOOT:
	case DAVINCI_MMC_ONLY_BOOT:
		return BOOT_DEVICE_MMC1;
#endif

#ifdef CONFIG_SPL_SPI_FLASH_SUPPORT
	case DAVINCI_SPI0_FLASH_BOOT:
	case DAVINCI_SPI1_FLASH_BOOT:
		return BOOT_DEVICE_SPI;
#endif

	default:
		puts("Unknown boot device\n");
		hang();
	}
}
