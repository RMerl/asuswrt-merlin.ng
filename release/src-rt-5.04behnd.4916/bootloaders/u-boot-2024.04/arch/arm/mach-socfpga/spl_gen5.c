// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/misc.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/sections.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <dm/uclass.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	const u32 bsel = readl(socfpga_get_sysmgr_addr() +
			       SYSMGR_GEN5_BOOTINFO);

	switch (SYSMGR_GET_BOOTINFO_BSEL(bsel)) {
	case 0x1:	/* FPGA (HPS2FPGA Bridge) */
		return BOOT_DEVICE_RAM;
	case 0x2:	/* NAND Flash (1.8V) */
	case 0x3:	/* NAND Flash (3.0V) */
		return BOOT_DEVICE_NAND;
	case 0x4:	/* SD/MMC External Transceiver (1.8V) */
	case 0x5:	/* SD/MMC Internal Transceiver (3.0V) */
		return BOOT_DEVICE_MMC1;
	case 0x6:	/* QSPI Flash (1.8V) */
	case 0x7:	/* QSPI Flash (3.0V) */
		return BOOT_DEVICE_SPI;
	default:
		printf("Invalid boot device (bsel=%08x)!\n", bsel);
		hang();
	}
}

#ifdef CONFIG_SPL_MMC
u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}
#endif

void board_init_f(ulong dummy)
{
	const struct cm_config *cm_default_cfg = cm_get_default_config();
	unsigned long reg;
	int ret;
	struct udevice *dev;

	ret = spl_early_init();
	if (ret)
		hang();

	socfpga_get_managers_addr();

	/*
	 * Clear fake OCRAM ECC first as SBE
	 * and DBE might triggered during power on
	 */
	reg = readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_ECCGRP_OCRAM);
	if (reg & SYSMGR_ECC_OCRAM_SERR)
		writel(SYSMGR_ECC_OCRAM_SERR | SYSMGR_ECC_OCRAM_EN,
		       socfpga_get_sysmgr_addr() + SYSMGR_GEN5_ECCGRP_OCRAM);
	if (reg & SYSMGR_ECC_OCRAM_DERR)
		writel(SYSMGR_ECC_OCRAM_DERR  | SYSMGR_ECC_OCRAM_EN,
		       socfpga_get_sysmgr_addr() + SYSMGR_GEN5_ECCGRP_OCRAM);

	socfpga_sdram_remap_zero();
	socfpga_pl310_clear();

	debug("Freezing all I/O banks\n");
	/* freeze all IO banks */
	sys_mgr_frzctrl_freeze_req();

	/* Put everything into reset but L4WD0. */
	socfpga_per_reset_all();

	if (!socfpga_is_booting_from_fpga()) {
		/* Put FPGA bridges into reset too. */
		socfpga_bridges_reset(1);
	}

	socfpga_per_reset(SOCFPGA_RESET(OSC1TIMER0), 0);
	timer_init();

	debug("Reconfigure Clock Manager\n");
	/* reconfigure the PLLs */
	if (cm_basic_init(cm_default_cfg))
		hang();

	/* Enable bootrom to configure IOs. */
	sysmgr_config_warmrstcfgio(1);

	/* configure the IOCSR / IO buffer settings */
	if (scan_mgr_configure_iocsr())
		hang();

	sysmgr_config_warmrstcfgio(0);

	/* configure the pin muxing through system manager */
	sysmgr_config_warmrstcfgio(1);
	sysmgr_pinmux_init();
	sysmgr_config_warmrstcfgio(0);

	/* Set bridges handoff value */
	socfpga_bridges_set_handoff_regs(true, true, true);

	debug("Unfreezing/Thaw all I/O banks\n");
	/* unfreeze / thaw all IO banks */
	sys_mgr_frzctrl_thaw_req();

#ifdef CONFIG_DEBUG_UART
	socfpga_per_reset(SOCFPGA_RESET(UART0), 0);
	debug_uart_init();
#endif

	ret = uclass_get_device(UCLASS_RESET, 0, &dev);
	if (ret)
		debug("Reset init failed: %d\n", ret);

#ifdef CONFIG_SPL_NAND_DENALI
	clrbits_le32(SOCFPGA_RSTMGR_ADDRESS + RSTMGR_GEN5_PERMODRST, BIT(4));
#endif

	/* enable console uart printing */
	preloader_console_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		hang();
	}
}
