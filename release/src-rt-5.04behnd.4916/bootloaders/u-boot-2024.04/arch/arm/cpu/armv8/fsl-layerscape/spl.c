// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clock_legacy.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <semihosting.h>
#include <spl.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <fsl_ifc.h>
#include <i2c.h>
#include <fsl_csu.h>
#include <asm/arch/fdt.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	if (semihosting_enabled())
		return BOOT_DEVICE_SMH;
#ifdef CONFIG_SPL_MMC
	return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_NAND_SUPPORT
	return BOOT_DEVICE_NAND;
#endif
#ifdef CONFIG_QSPI_BOOT
	return BOOT_DEVICE_NOR;
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
#if defined(CONFIG_NXP_ESBC) && defined(CONFIG_FSL_LSCH2)
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	*/
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif
}

void tzpc_init(void)
{
	/*
	 * Mark the whole OCRAM as non-secure, otherwise DMA devices cannot
	 * access it. This is for example necessary for MMC boot.
	 */
#ifdef TZPCR0SIZE_BASE
	out_le32(TZPCR0SIZE_BASE, 0);
#endif
}

__weak int init_func_vid(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	icache_enable();
	tzpc_init();

	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));
	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();
	board_early_init_f();
	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}
	timer_init();
#ifdef CONFIG_ARCH_LS2080A
	env_init();
#endif
	get_clocks();

	preloader_console_init();
	spl_set_bd();

#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
#ifdef CONFIG_SPL_I2C
	i2c_init_all();
#endif
#endif
#if defined(CONFIG_VID) && (defined(CONFIG_ARCH_LS1088A) || \
			    defined(CONFIG_ARCH_LX2160A) || \
			    defined(CONFIG_ARCH_LX2162A))
	init_func_vid();
#endif
	dram_init();
#if defined(CONFIG_QSPI_AHB_INIT) && defined(CONFIG_QSPI_BOOT)
	qspi_ahb_init();
#endif
}

#ifdef CONFIG_SPL_OS_BOOT
/*
 * Return
 * 0 if booting into OS is selected
 * 1 if booting into U-Boot is selected
 */
int spl_start_uboot(void)
{
	env_init();
	if (env_get_yesno("boot_os") != 0)
		return 0;

	return 1;
}
#endif	/* CONFIG_SPL_OS_BOOT */
#endif /* CONFIG_SPL_BUILD */
