// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <fsl_ifc.h>
#include <i2c.h>
#include <fsl_csu.h>
#include <asm/arch/fdt.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC_SUPPORT
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
#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_FSL_LSCH2)
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
#ifdef CONFIG_SPL_FSL_LS_PPA
	ppa_init();
#endif
}

void board_init_f(ulong dummy)
{
	icache_enable();
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));
	board_early_init_f();
	timer_init();
#ifdef CONFIG_ARCH_LS2080A
	env_init();
#endif
	get_clocks();

	preloader_console_init();
	spl_set_bd();

#ifdef CONFIG_SPL_I2C_SUPPORT
	i2c_init_all();
#endif
#ifdef CONFIG_VID
	init_func_vid();
#endif
	dram_init();
#ifdef CONFIG_SPL_FSL_LS_PPA
#ifndef CONFIG_SYS_MEM_RESERVE_SECURE
#error Need secure RAM for PPA
#endif
	/*
	 * Secure memory location is determined in dram_init_banksize().
	 * gd->ram_size is deducted by the size of secure ram.
	 */
	dram_init_banksize();

	/*
	 * After dram_init_bank_size(), we know U-Boot only uses the first
	 * memory bank regardless how big the memory is.
	 */
	gd->ram_top = gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;

	/*
	 * If PPA is loaded, U-Boot will resume running at EL2.
	 * Cache and MMU will be enabled. Need a place for TLB.
	 * U-Boot will be relocated to the end of available memory
	 * in first bank. At this point, we cannot know how much
	 * memory U-Boot uses. Put TLB table lower by SPL_TLB_SETBACK
	 * to avoid overlapping. As soon as the RAM version U-Boot sets
	 * up new MMU, this space is no longer needed.
	 */
	gd->ram_top -= SPL_TLB_SETBACK;
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = (gd->ram_top - gd->arch.tlb_size) & ~(0x10000 - 1);
	gd->arch.tlb_allocated = gd->arch.tlb_addr;
#endif	/* CONFIG_SPL_FSL_LS_PPA */
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
#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
#endif /* CONFIG_SPL_BUILD */
